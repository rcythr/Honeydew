# This file is part of Honeydew 
# Honeydew is licensed under the MIT LICENSE. See the LICENSE file for more info.

cmake_minimum_required(VERSION 2.8)
project(HONEYDEW)

# Set up CMAKE settings
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_INSTALL_PREFIX "")

list(APPEND CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++ -Wall -pedantic") 

# File all .cc files in the source dir and helpers dir
file(GLOB_RECURSE SOURCES "src/*.cc" "src/*.hpp" "include/*.hpp")

# Setup include path for the lib
include_directories(${CMAKE_SOURCE_DIR})

# Build the lib using the cc files found above.
add_library(honeydew ${SOURCES})
target_link_libraries(honeydew pthread)

# Output the includes into the build dir
file(GLOB OUTPUT_INCLUDES "honeydew/*.hpp")
file(GLOB OUTPUT_HELPERS "honeydew/helpers/*.hpp")

# Setup install stuff for make
install(FILES ${OUTPUT_INCLUDES} DESTINATION ${CMAKE_BINARY_DIR}/include)
install(FILES ${OUTPUT_HELPERS} DESTINATION ${CMAKE_BINARY_DIR}/include/helpers)

# Now we add in the examples directory
add_subdirectory(examples)
