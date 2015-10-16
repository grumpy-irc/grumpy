project(GrumpyIRC)
cmake_minimum_required (VERSION 2.9.0)

# Configuration
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# Libraries first

add_subdirectory (libirc2htmlcode)
add_subdirectory (libirc/libirc/)
add_subdirectory (libirc/libircclient/)

# GrumpyIRC client itself

add_subdirectory (libcore)
add_subdirectory (grumpyd)
add_subdirectory (GrumpyIRC)
