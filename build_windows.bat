@echo off
setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
set BUILD_DIR=%SCRIPT_DIR%build_windows
set BUILD_TYPE=Release
set JOBS=%NUMBER_OF_PROCESSORS%

:parse_args
if "%~1"=="" goto :build
if /i "%~1"=="--clean" (
    echo Cleaning build directory...
    if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
    shift
    goto :parse_args
)
if /i "%~1"=="--debug" (
    set BUILD_TYPE=Debug
    shift
    goto :parse_args
)
if /i "%~1"=="--jobs" (
    set JOBS=%~2
    shift
    shift
    goto :parse_args
)
echo Unknown option: %~1
echo Usage: %0 [--clean] [--debug] [--jobs N]
exit /b 1

:build
echo Creating build directory...
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

cd "%BUILD_DIR%"

if not exist "CMakeCache.txt" (
    echo Configuring CMake...
    cmake "%SCRIPT_DIR%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
) else (
    echo CMake already configured for %BUILD_TYPE% build.
)

echo Building project with %JOBS% jobs...
cmake --build . --config %BUILD_TYPE% --parallel %JOBS%

echo Build complete!
echo Executable: .\%BUILD_DIR%\%BUILD_TYPE%\klab.exe
echo.
echo To run: .\%BUILD_DIR%\%BUILD_TYPE%\klab.exe