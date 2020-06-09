cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(MCHawk2 VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB SOURCES
    "include/*.hpp"
    "include/Net/*.hpp"
    "include/Utils/*.hpp"
    "src/*.cpp"
    "src/Net/*.cpp"
    "src/Utils/*.cpp"
)

# add the executable
add_executable(MCHawk2 ${SOURCES})

add_subdirectory(deps/zlib-1.2.11)
include_directories(MCHawk2 ${ZLIB_DEPS_DIR})
target_link_libraries(MCHawk2 PUBLIC zlib)