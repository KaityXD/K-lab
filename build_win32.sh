#!/bin/bash

set -e

# Get the directory where the script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build_win32"
BUILD_TYPE="Release"
JOBS=$(nproc 2>/dev/null || echo 4)
TOOLCHAIN_FILE="$SCRIPT_DIR/win32_toolchain.cmake"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --clean)
            echo "Cleaning build directory..."
            rm -rf "$BUILD_DIR"
            shift
            ;;
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --jobs)
            JOBS="$2"
            shift 2
            ;;
        --toolchain)
            TOOLCHAIN_FILE="$2"
            # If not an absolute path, make it relative to the original PWD
            [[ "$TOOLCHAIN_FILE" != /* ]] && TOOLCHAIN_FILE="$(pwd)/$TOOLCHAIN_FILE"
            shift 2
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--clean] [--debug] [--jobs N] [--toolchain FILE]"
            exit 1
            ;;
    esac
done

echo "Creating build directory..."
mkdir -p "$BUILD_DIR"

cd "$BUILD_DIR"

# Check for available Windows compilers
echo "Checking for Windows compilers..."

if command -v x86_64-w64-mingw32-gcc >/dev/null 2>&1; then
    echo "Found MinGW cross-compiler"
    echo "Configuring CMake for Windows cross-compilation..."
    cmake "$SCRIPT_DIR" \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
        -G "Unix Makefiles"
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    echo "Detected Windows environment (MSYS2/Cygwin)"
    echo "Configuring CMake for native Windows build..."
    cmake "$SCRIPT_DIR" \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -G "Unix Makefiles"
else
    echo "No Windows compiler found!"
    echo ""
    echo "To build for Windows, you need one of these:"
    echo ""
    echo "1. MinGW-w64 cross-compiler (Linux/macOS):"
    echo "   - Install: sudo apt-get install mingw-w64"
    echo "   - This creates: x86_64-w64-mingw32-gcc"
    echo ""
    echo "2. Native MinGW (Windows with MSYS2):"
    echo "   - Install MSYS2 from https://www.msys2.org/"
    echo "   - Install MinGW: pacman -S mingw-w64-x86_64-gcc"
    echo ""
    echo "3. Visual Studio (Windows):"
    echo "   - Use build_windows.bat instead"
    echo ""
    echo "Alternatively, you can:"
    echo "- Build natively on Linux: ./build.sh"
    echo "- Build natively on macOS: ./build_macos.sh"
    echo "- Build on Windows with VS: build_windows.bat"
    exit 1
fi

# Build
echo "Building project with $JOBS jobs..."
if [[ -f "Makefile" ]]; then
    make -j"$JOBS"
elif [[ -f "build.ninja" ]]; then
    ninja -j"$JOBS"
else
    cmake --build . --parallel "$JOBS"
fi

echo "Windows build complete!"
echo "Executable: ./$BUILD_DIR/klab.exe"
echo ""
echo "NOTE: This build is STATICALLY LINKED. You should NOT need most DLLs."
echo "If it still asks for them, only copy the ones it specifically names."
echo ""
echo "If you still see error 0xc000007b, it means you have a 32-bit DLL in your path."
echo "Check your architecture with this command:"
echo "  file ./$BUILD_DIR/klab.exe"
echo "  It should say 'PE32+ executable (console) x86-64' for 64-bit."
echo ""
echo "Make sure to include folders: assets/, shaders/, scenarios/"
