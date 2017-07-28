project(GrumpyIRC)
cmake_minimum_required (VERSION 2.8.0)
if(WIN32)
    if (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.0)
        add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)
    endif()
endif()

if(WIN32)
    if(MINGW)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()
else()
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# Settings
option(QT5_BUILD "Build with Qt5" false)
option(EXTREME_DEBUG "Enable some resource expensive and slowing debug info" false)
option(UNIT_TESTS "Enable unit tests" false)
option(SQLITE "Support sqlite backend storage" true)

# Configuration
if (UNIT_TESTS)
    SET(QT_USE_QTTEST TRUE)
endif()
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# Libraries first

add_subdirectory (libirc2htmlcode)
add_subdirectory (libirc/libirc/)
add_subdirectory (libirc/libircclient/)
add_subdirectory (libgp)

# GrumpyIRC client itself

add_subdirectory (libcore)
add_subdirectory (grumpyd)
add_subdirectory (GrumpyIRC)

# Unit test

if (UNIT_TESTS)
    add_subdirectory (tests)
endif()
