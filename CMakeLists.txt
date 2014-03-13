cmake_minimum_required(VERSION 2.8)
project(RFUS)

# Set up CMAKE settings
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_INSTALL_PREFIX "")

# Configure for C++11
add_definitions(-std=c++11)

# File all .cc files in the source dir and helpers dir
file(GLOB_RECURSE SOURCES "src/*.cc" "src/*.hpp" "include/*.hpp")

# Setup include path for the lib
include_directories(${CMAKE_SOURCE_DIR})

# Build the lib using the cc files found above.
add_library(rfus ${SOURCES})

# Output the includes into the build dir
file(GLOB OUTPUT_INCLUDES "rfus/*.hpp")
file(GLOB OUTPUT_HELPERS "rfus/helpers/*.hpp")

# Setup install stuff for make
install(FILES ${OUTPUT_INCLUDES} DESTINATION ${CMAKE_BINARY_DIR}/include)
install(FILES ${OUTPUT_HELPERS} DESTINATION ${CMAKE_BINARY_DIR}/include/helpers)
