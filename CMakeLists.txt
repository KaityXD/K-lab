cmake_minimum_required(VERSION 3.10)
project(klab)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(OpenGL REQUIRED)
find_package(PkgConfig QUIET)
find_package(SQLite3 QUIET)

if(PKG_CONFIG_FOUND)
    pkg_check_modules(GLFW3 QUIET glfw3)
    pkg_check_modules(GLEW QUIET glew)
    pkg_check_modules(LUAJIT QUIET luajit)
    if(NOT SQLite3_FOUND)
        pkg_check_modules(SQLITE3 QUIET sqlite3)
    endif()
endif()

# Fallback for SQLite3
if(NOT SQLite3_FOUND AND NOT SQLITE3_FOUND)
    find_path(SQLITE3_INCLUDE_DIRS sqlite3.h)
    find_library(SQLITE3_LIBRARIES NAMES sqlite3)
    if(SQLITE3_INCLUDE_DIRS AND SQLITE3_LIBRARIES)
        set(SQLITE3_FOUND TRUE)
    endif()
endif()

# Fallback for GLFW3
if(NOT GLFW3_FOUND)
    find_package(glfw3 QUIET)
    if(NOT glfw3_FOUND)
        find_package(GLFW3 QUIET)
    endif()
    if(GLFW3_FOUND OR glfw3_FOUND)
        set(GLFW3_LIBRARIES glfw)
    endif()
endif()

# Fallback for GLEW
if(NOT GLEW_FOUND)
    find_package(GLEW QUIET)
endif()

# Fallback for LuaJIT (often needs manual help on Windows)
if(NOT LUAJIT_FOUND)
    find_path(LUAJIT_INCLUDE_DIR lua.hpp PATH_SUFFIXES luajit-2.1 luajit-2.0)
    find_library(LUAJIT_LIBRARIES NAMES luajit-5.1 luajit libluajit-5.1.so.2)
    if(LUAJIT_INCLUDE_DIR AND LUAJIT_LIBRARIES)
        set(LUAJIT_FOUND TRUE)
        set(LUAJIT_INCLUDE_DIRS ${LUAJIT_INCLUDE_DIR})
    endif()
endif()

# Check essentials
if(NOT GLFW3_FOUND AND NOT glfw3_FOUND)
    message(FATAL_ERROR "GLFW3 not found!")
endif()
if(NOT GLEW_FOUND)
    message(FATAL_ERROR "GLEW not found!")
endif()
if(NOT SQLite3_FOUND AND NOT SQLITE3_FOUND)
    message(FATAL_ERROR "SQLite3 not found!")
endif()

# Fix LuaJIT include path for Ubuntu/Debian
if(NOT LUAJIT_FOUND)
    find_path(LUAJIT_INCLUDE_DIR lua.hpp PATH_SUFFIXES luajit-2.1 luajit-2.0)
    find_library(LUAJIT_LIBRARIES NAMES luajit-5.1 luajit libluajit-5.1.so.2)
    if(LUAJIT_INCLUDE_DIR AND LUAJIT_LIBRARIES)
        set(LUAJIT_FOUND TRUE)
        set(LUAJIT_INCLUDE_DIRS ${LUAJIT_INCLUDE_DIR})
    endif()
endif()

# Force LuaJIT include directory for Ubuntu
if(NOT LUAJIT_FOUND)
    if(EXISTS "/usr/include/luajit-2.1")
        set(LUAJIT_INCLUDE_DIRS "/usr/include/luajit-2.1")
        set(LUAJIT_LIBRARIES "luajit-5.1")
        set(LUAJIT_FOUND TRUE)
    elseif(EXISTS "/usr/include/lua5.1")
        set(LUAJIT_INCLUDE_DIRS "/usr/include/lua5.1")
        set(LUAJIT_LIBRARIES "lua5.1")
        set(LUAJIT_FOUND TRUE)
    endif()
endif()

# Include directories
find_package(glm QUIET)
if (NOT GLM_FOUND)
    # Check common locations if find_package fails
    find_path(GLM_INCLUDE_DIR glm/glm.hpp
        PATHS /usr/include /usr/local/include /mingw64/include
    )
    if(GLM_INCLUDE_DIR)
        include_directories(${GLM_INCLUDE_DIR})
    else()
        message(WARNING "GLM not found! Some features might not work.")
    endif()
else()
    include_directories(${GLM_INCLUDE_DIRS})
endif()

if (SQLite3_FOUND)
    include_directories(${SQLite3_INCLUDE_DIRS})
elseif (SQLITE3_FOUND)
    include_directories(${SQLITE3_INCLUDE_DIRS})
endif()

include_directories(
    ${OPENGL_INCLUDE_DIRS}
    ${GLFW3_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${LUAJIT_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/thirdparty
    ${PROJECT_SOURCE_DIR}/thirdparty/imgui
    ${PROJECT_SOURCE_DIR}/thirdparty/imgui/backends
)

# Source files
set(SOURCES
    src/main.cpp
    src/engine/Engine.cpp
    src/engine/AssetManager.cpp
    src/engine/Window.cpp
    src/engine/Input.cpp
    src/engine/Camera.cpp
    src/engine/SoundManager.cpp
    src/engine/Renderer.cpp
    src/engine/Mesh.cpp
    src/engine/TextureManager.cpp
    src/game/Target.cpp
    src/game/Scenario.cpp
    src/game/ScoreManager.cpp
    src/ui/Menu.cpp
    src/ui/Settings.cpp
    src/ui/ScenarioBrowser.cpp
    src/ui/Crosshair.cpp
    src/scripting/LuaManager.cpp
    
    # ImGui sources
    thirdparty/imgui/imgui.cpp
    thirdparty/imgui/imgui_demo.cpp
    thirdparty/imgui/imgui_draw.cpp
    thirdparty/imgui/imgui_tables.cpp
    thirdparty/imgui/imgui_widgets.cpp
    thirdparty/imgui/backends/imgui_impl_glfw.cpp
    thirdparty/imgui/backends/imgui_impl_opengl3.cpp
)

# Executable
if(NOT SOURCES)
    message(FATAL_ERROR "No source files found for target klab")
endif()
add_executable(klab ${SOURCES})

# Libraries
set(LIBRARIES
    ${OPENGL_LIBRARIES}
    ${GLFW3_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${LUAJIT_LIBRARIES}
    ${SQLite3_LIBRARIES}
    ${SQLITE3_LIBRARIES}
)

if(WIN32)
    list(APPEND LIBRARIES opengl32 gdi32 user32 shell32)
    if(MINGW)
        # Force static linking for MinGW to reduce DLL dependencies
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")
        add_definitions(-DGLEW_STATIC)
        # Try to find static versions of libraries
        find_library(GLFW3_STATIC_LIB NAMES glfw3.a libglfw3.a glfw3static libglfw3static)
        find_library(GLEW_STATIC_LIB NAMES glew32s.a libglew32s.a GLEW_STATIC)
        find_library(LUAJIT_STATIC_LIB NAMES libluajit-5.1.a luajit-5.1.a)
        
        # Use static libs if found
        if(GLFW3_STATIC_LIB)
            set(GLFW3_LIBRARIES ${GLFW3_STATIC_LIB})
            message(STATUS "Using static GLFW3: ${GLFW3_STATIC_LIB}")
        endif()
        if(GLEW_STATIC_LIB)
            set(GLEW_LIBRARIES ${GLEW_STATIC_LIB})
            message(STATUS "Using static GLEW: ${GLEW_STATIC_LIB}")
        endif()
        if(LUAJIT_STATIC_LIB)
            set(LUAJIT_LIBRARIES ${LUAJIT_STATIC_LIB})
            message(STATUS "Using static LuaJIT: ${LUAJIT_STATIC_LIB}")
        endif()
    endif()
else()
    list(APPEND LIBRARIES pthread m dl)
endif()

target_link_libraries(klab ${LIBRARIES})

# Installation
if(WIN32)
    install(TARGETS klab DESTINATION .)
    
    # Attempt to copy necessary DLLs if they can be found
    if(MINGW)
        set(DLL_SEARCH_PATHS
            /usr/x86_64-w64-mingw32/bin
            /usr/lib/gcc/x86_64-w64-mingw32/*/
            /mingw64/bin
            ${CMAKE_SOURCE_DIR}/bin
        )
        
        set(REQUIRED_DLLS
            glfw3.dll
            glew32.dll
            luajit.dll
            libgcc_s_seh-1.dll
            libstdc++-6.dll
            libwinpthread-1.dll
        )
        
        foreach(dll ${REQUIRED_DLLS})
            find_file(DLL_PATH_${dll} ${dll} PATHS ${DLL_SEARCH_PATHS})
            if(DLL_PATH_${dll})
                install(FILES ${DLL_PATH_${dll}} DESTINATION .)
            else()
                message(STATUS "Could not find ${dll}, you may need to copy it manually.")
            endif()
        endforeach()
    endif()
else()
    install(TARGETS klab DESTINATION bin)
endif()

install(DIRECTORY assets DESTINATION .)
install(DIRECTORY shaders DESTINATION .)
install(DIRECTORY scenarios DESTINATION .)
install(FILES config.cfg crosshairs.cfg DESTINATION . OPTIONAL)
