project(GrumpyIRC)
cmake_minimum_required (VERSION 2.8.0)

# Settings
option(QT5_BUILD "Build with Qt5" false)
option(SQLITE "Support sqlite backend storage" true)

# Configuration
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
