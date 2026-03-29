#!/bin/bash

set -e

# Detect platform
detect_platform() {
    case "$(uname -s)" in
        Linux*)     echo "linux";;
        Darwin*)    echo "macos";;
        CYGWIN*)    echo "windows";;
        MINGW*)     echo "windows";;
        *)          echo "unknown";;
    esac
}

# Get the directory where the script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

PLATFORM=$(detect_platform)
echo "Detected platform: $PLATFORM"

# Build based on platform
case $PLATFORM in
    linux)
        echo "Building for Linux..."
        "$SCRIPT_DIR/build.sh" "$@"
        ;;
    macos)
        echo "Building for macOS..."
        "$SCRIPT_DIR/build_macos.sh" "$@"
        ;;
    windows)
        echo "Building for Windows using MinGW..."
        "$SCRIPT_DIR/build_win32.sh" "$@"
        ;;
    *)
        echo "Unsupported platform: $PLATFORM"
        echo "Please use one of the platform-specific build scripts:"
        echo "  - build.sh (Linux)"
        echo "  - build_macos.sh (macOS)"
        echo "  - build_win32.sh (Windows with MinGW)"
        echo "  - build_windows.bat (Windows with Visual Studio)"
        exit 1
        ;;
esac