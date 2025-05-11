# ForgeMake Build System

ForgeMake is a simple and lightweight build system designed to compile your C and C++ projects. It is currently designed to work exclusively on Linux platforms. It allows you to define your project’s source files, compiler settings, and flags in a straightforward, human-readable `.frg` configuration file. Once set up, simply run `./ForgeMake` in your project directory to automatically compile your code.

---

## Features

- **Linux-only Support**: ForgeMake is optimized for Linux, providing an efficient build process for Linux-based environments.
- **Customizable Compiler**: Supports a compiler of your choice, with `clang` and `clang++` as defaults.
- **Easy Configuration**: Specify source files, compiler flags, and libraries in a simple `.frg` file.
- **File Change Detection**: ForgeMake tracks the changes in source files, ensuring only modified files are recompiled.
- **Direct Compiler Calls**: Unlike systems like `make` or `ninja`, ForgeMake directly invokes the compiler, giving you complete control over the build process.

---

## Installation

1. **Compile ForgeMake**:  
   To compile ForgeMake, you'll need to run the following command:

   ```bash
   clang++ -O2 -std=c++17 -o ForgeMake forgeMake.cpp
   ```

2. **Make ForgeMake Executable**:  
   After compiling, make sure to set the executable permission for the `ForgeMake` binary:

   ```bash
   chmod +x ForgeMake
   ```

---

## Configuration File (`list.frg`)

In your project’s root directory, create a `list.frg` file. This file should define all the necessary settings for ForgeMake to compile your project. Here is an example of a `list.frg` file:

```bash
compilerC = clang
compilerCXX = clang++
platform = linux
src = ../src/DrakonEngine/*.cpp
src = ../src/DrakonEngine/render/*.cpp
src = ../src/DrakonEngine/render/opengl/*.cpp
src = ../src/DrakonEngine/render/opengl/glad/*.cpp
src = ../src/*.cpp
lib = ../lib/libglfw3.a ../lib/libglm.a
flagsC = -std=c17
flagsCXX = -std=c++17
name = drkEngine
```

### Available Fields:
- `compilerC`: The C compiler (e.g., `clang`).
- `compilerCXX`: The C++ compiler (e.g., `clang++`).
- `platform`: The target platform (set to `linux` for now).
- `src`: A list of source files or directories to include in the build.
- `lib`: Any additional static libraries required for linking (e.g., `libglfw3.a`).
- `flagsC`: C compiler flags (e.g., `-std=c17`).
- `flagsCXX`: C++ compiler flags (e.g., `-std=c++17`).
- `name`: The name of the output binary.

---

## Usage

1. **Create the `list.frg` File**:  
   Write a configuration file (`list.frg`) in the same directory as the `ForgeMake` executable.

2. **Run ForgeMake**:  
   To run the build system and compile your project, use the following command in the terminal:

   ```bash
   ./ForgeMake
   ```

   ForgeMake will automatically detect any changes in your source files and compile only the modified files.

---

## Future Features

- **Cross-Platform Support**: Currently, ForgeMake only supports Linux-to-Linux compilation. Future updates will add support for Windows and macOS platforms.
- **Parallelism**: The ability to compile files in parallel will be added to improve performance, especially on larger projects.

---

## Notes

- ForgeMake is a minimalistic build system with no dependencies on traditional build systems like `make` or `ninja`. It’s designed to be simple, fast, and flexible.
- Currently, only Linux is supported as the platform for compiling your project.
- Ensure that all paths to source files and libraries in the `list.frg` file are correct.

--- 

## License

ForgeMake is released under the GNU General Public License v3.0 License. See [LICENSE](./LICENSE) for details.