cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(MCHawk2 VERSION 0.0.1)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#add_subdirectory(vendor/zlib)

file(GLOB_RECURSE SOURCES src/*.cpp)

# add the executable
add_executable(MCHawk2 ${SOURCES})

#target_include_directories(MCHawk2 PUBLIC vendor/zlib ${ZLIB_DEPS_DIR})
#target_link_directories(MCHawk2 PUBLIC ${ZLIB_DEPS_DIR})

IF (UNIX)
	target_link_libraries(MCHawk2 PUBLIC pthread z)
ELSE()
	target_link_libraries(MCHawk2 PUBLIC zlib)
ENDIF()

