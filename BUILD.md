# Build System

This project supports cross-platform building with CMake. Choose the appropriate build script for your platform:

## Quick Start

```bash
# Auto-detect platform and build
./build_all.sh

# Or use platform-specific scripts:
# Linux: ./build.sh
# macOS: ./build_macos.sh
# Windows (MinGW): ./build_win32.sh
# Windows (Visual Studio): build_windows.bat
```

## Build Options

All build scripts support these options:
- `--clean`: Clean build (removes build directory)
- `--debug`: Build in Debug mode (default is Release)
- `--jobs N`: Set number of parallel jobs

### Examples

```bash
# Clean debug build with 8 jobs
./build.sh --clean --debug --jobs 8

# Cross-compile for Windows from Linux (requires MinGW)
./build_win32.sh --clean --jobs 4

# Windows with Visual Studio
build_windows.bat --clean --debug
```

## Platform Requirements

### Linux
- CMake 3.10+
- GCC/Clang
- Development packages: `libglfw3-dev`, `libglew-dev`, `libluajit-5.1-dev`

### macOS
- CMake 3.10+
- Xcode Command Line Tools
- Homebrew packages: `glfw`, `glew`, `luajit`

### Windows (MinGW)
- MinGW-w64 cross-compiler (Linux/macOS) or MSYS2 (Windows)
- Linux: `sudo apt-get install mingw-w64`
- macOS: `brew install mingw-w64`

### Windows (Visual Studio)
- Visual Studio 2022 or later
- CMake 3.20+
- vcpkg for dependencies (optional)

## Installing MinGW Cross-Compiler

### Ubuntu/Debian
```bash
sudo apt-get install mingw-w64
```

### macOS
```bash
brew install mingw-w64
```

### Arch Linux
```bash
sudo pacman -S mingw-w64-gcc
```

### Windows (MSYS2)
```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake
```