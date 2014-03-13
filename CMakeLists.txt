cmake_minimum_required(VERSION 2.8)
project(RFUS)

#Set the output directory for the library
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Configure for C++11
add_definitions(-std=c++11)

# File all .cc files in the source dir and helpers dir
file(GLOB_RECURSE SOURCES "src/*.cc" "src/*.hpp" "include/*.hpp" "src/detail/*.hpp")

# Setup include path for the lib
include_directories(include)

# Build the lib using the cc files found above.
add_library(rfus ${SOURCES})

