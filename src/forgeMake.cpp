#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <filesystem>
#include <sys/stat.h>

#include <thread>
#include <mutex>

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define ORANGE  "\033[38;5;214m"
#define GREEN   "\033[32m"
#define CYAN    "\033[36m"

constexpr int debug = 0;

namespace fs = std::filesystem;

std::mutex coutMutex;
std::mutex objMutex;
std::vector<std::string> objectFiles;



struct StructBuildConfig {
    std::string compilerC;
    std::string compilerCXX;
    std::string platform;
    std::string flagsC;
    std::string flagsCXX;
    std::string name;
    std::vector<std::string> src;
    std::string lib;
};

StructBuildConfig buildconfig;


void readFirstWordFromAllLines(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cout << RED "[ERR]: " RESET << "Failed to open the file: " << filename << std::endl;
        std::exit(1);
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t equalPos = line.find('=');
        if (equalPos == std::string::npos) continue;

        std::string key = line.substr(0, equalPos);
        std::string value = line.substr(equalPos + 1);

        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);

        if (key == "compilerC") buildconfig.compilerC = value;
        else if (key == "compilerCXX") buildconfig.compilerCXX = value;
        else if (key == "platform") buildconfig.platform = value;
        else if (key == "flagsC") buildconfig.flagsC = value;
        else if (key == "flagsCXX") buildconfig.flagsCXX = value;
        else if (key == "name") buildconfig.name = value;
        else if (key == "src") buildconfig.src.push_back(value);
        else if (key == "lib") buildconfig.lib = value;

        if (debug) {
            std::cout << CYAN "[INFO]: " RESET << "Processed line: " << line << RESET <<"\n";
        }
    }

    file.close();
}


bool isModified(const std::string& source, const std::string& object) {
    struct stat srcStat{}, objStat{};

    if (stat(source.c_str(), &srcStat) != 0) return true;  // source missing
    if (stat(object.c_str(), &objStat) != 0) return true;  // object missing

    return srcStat.st_mtime > objStat.st_mtime;
}


std::vector<std::string> expandWildcards(const std::string& pattern) {
    std::vector<std::string> files;
    std::string cmd = "ls " + pattern + " 2>/dev/null";

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return files;

    char buffer[512];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        std::string file = buffer;
        file.erase(std::remove(file.begin(), file.end(), '\n'), file.end());
        files.push_back(file);
    }

    pclose(pipe);

    return files;
}


std::vector<std::string> getIncludedHeaders(const std::string& sourceFile) {
    std::vector<std::string> headers;
    std::ifstream file(sourceFile);
    if (!file.is_open()) return headers;

    fs::path sourcePath = fs::absolute(fs::path(sourceFile).parent_path());

    std::string line;
    while (std::getline(file, line)) {
        std::size_t pos = line.find("#include");

        if (pos != std::string::npos) {
            std::size_t start = line.find("\"", pos);
            std::size_t end = line.find("\"", start + 1);
            
            if (start != std::string::npos && end != std::string::npos) {
                std::string header = line.substr(start + 1, end - start - 1);
                fs::path headerPath = sourcePath / header;

                if (fs::exists(headerPath)) {
                    headers.push_back(headerPath.string());

                    if (debug) {
                        std::cout << CYAN "[INFO]: " RESET << "Found header: " << headerPath << "\n";
                    }
                }
            }
        }
    }

    return headers;
}


bool isModifiedWithHeaders(const std::string& source, const std::string& object) {
    if (isModified(source, object)) return true;

    for (const auto& header : getIncludedHeaders(source)) {
        if (isModified(header, object)) return true;
    }

    return false;
}

void compileFile(const std::string& srcFile, const std::string& compiler, const std::string& flags) {
    std::string base = fs::path(srcFile).stem().string();
    std::string objFile = "ForgeMakeCache/" + base + ".o";

    if (isModifiedWithHeaders(srcFile, objFile)) {
        std::ostringstream cmd;
        cmd << compiler << " -c " << flags << " " << srcFile << " -o " << objFile;

        {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cout << GREEN "[INFO]: " RESET << "Compiling: " << cmd.str() << "\n";
        }

        if (system(cmd.str().c_str()) != 0) {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cerr << RED "[ERR]: " RESET << "Compilation failed: " << srcFile << "\n";
            return;
        }
    } else {
        if (debug) {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cout << CYAN "[INFO]: " RESET << "Up-to-date: " << srcFile << "\n";
        }
    }

    {
        std::lock_guard<std::mutex> lock(objMutex);
        objectFiles.push_back(objFile);
    }
}

void compileAndLink() {
    fs::create_directory("ForgeMakeCache");
    std::vector<std::thread> threads;

    for (const auto& pattern : buildconfig.src) {
        for (const auto& srcFile : expandWildcards(pattern)) {
            std::string extension = fs::path(srcFile).extension();
            std::string compiler, flags;

            if (extension == ".c") {
                compiler = buildconfig.compilerC;
                flags = buildconfig.flagsC;
            } else if (extension == ".cpp" || extension == ".cc" || extension == ".cxx") {
                compiler = buildconfig.compilerCXX;
                flags = buildconfig.flagsCXX;
            } else {
                std::lock_guard<std::mutex> lock(coutMutex);
                std::cout << ORANGE "[WARN]: " RESET << "Unsupported file type: " << srcFile << RESET << "\n";
                continue;
            }

            threads.emplace_back(compileFile, srcFile, compiler, flags);
        }
    }

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    std::ostringstream linkCmd;
    linkCmd << buildconfig.compilerCXX << " -o " << buildconfig.name;

    for (const auto& obj : objectFiles) {
        linkCmd << " " << obj;
    }

    linkCmd << " " << buildconfig.lib;

    std::cout << CYAN "[INFO]: " RESET << "Linking: " << linkCmd.str() << "\n";

    if (system(linkCmd.str().c_str()) != 0) {
        std::cerr << RED "[ERR]: " RESET << "Linking failed.\n";
    }
}


int main() {
    std::string filename = "list.frg";

    readFirstWordFromAllLines(filename);
    compileAndLink();

    return 0;
}
