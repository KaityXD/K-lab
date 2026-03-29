# Windows (MinGW-w64) Toolchain File
set(CMAKE_SYSTEM_NAME Windows)

# Allow overriding the compiler prefix (default to x86_64-w64-mingw32-)
if(NOT MINGW_PREFIX)
    set(MINGW_PREFIX x86_64-w64-mingw32-)
endif()

set(CMAKE_C_COMPILER ${MINGW_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${MINGW_PREFIX}g++)
set(CMAKE_RC_COMPILER ${MINGW_PREFIX}windres)

# Adjust searching behavior: find programs in the host environment, 
# but search for libraries and headers in the target environment.
set(MINGW_ROOT_PATHS 
    /usr/x86_64-w64-mingw32 
    /usr/lib/gcc/x86_64-w64-mingw32
    /mingw64
)

foreach(path ${MINGW_ROOT_PATHS})
    if(EXISTS ${path})
        list(APPEND CMAKE_FIND_ROOT_PATH ${path})
    endif()
endforeach()

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
