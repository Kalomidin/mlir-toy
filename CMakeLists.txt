# For a better template to copy, see examples/standalone
cmake_minimum_required(VERSION 3.20.0)

project(toy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "-g") # Add debugging symbols
set(CMAKE_BUILD_TYPE Debug)    # Set build type to Debug

# Specify LLVM components to link
set(LLVM_LINK_COMPONENTS
  Support
)

include_directories(include/)

add_executable(toy src/main.cpp parser/AST.cpp)