#!/bin/bash

set -e

# Get the directory where the script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build_macos"
BUILD_TYPE="Release"
JOBS=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)

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
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--clean] [--debug] [--jobs N]"
            exit 1
            ;;
    esac
done

echo "Creating build directory..."
mkdir -p "$BUILD_DIR"

cd "$BUILD_DIR"

# Configure only if CMakeCache.txt doesn't exist or we're doing a clean build
if [[ ! -f "CMakeCache.txt" ]] || [[ "$BUILD_TYPE" != "$(grep -o 'CMAKE_BUILD_TYPE:STRING=[^ ]*' CMakeCache.txt | cut -d= -f2)" ]]; then
    echo "Configuring CMake for macOS..."
    cmake "$SCRIPT_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15
else
    echo "CMake already configured for macOS $BUILD_TYPE build."
fi

# Build
echo "Building project with $JOBS jobs..."
make -j"$JOBS"

echo "macOS build complete!"
echo "Executable: ./$BUILD_DIR/klab"
echo ""
echo "To run: ./$BUILD_DIR/klab"
echo ""
echo "To create an app bundle, run: make install"