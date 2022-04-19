cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(MCHawk2 VERSION 0.0.1)

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

IF (UNIX)
	target_link_libraries(MCHawk2 PUBLIC pthread)
ENDIF()

include_directories(MCHawk2 deps/sol2/include)
find_package(Lua)
if (${Lua_FOUND})
	include_directories(/usr/local/include ${LUA_INCLUDE_DIR})
	target_link_libraries(MCHawk2 PUBLIC z)
	target_link_libraries(MCHawk2 PUBLIC lua5.4)
ELSE (${Lua_FOUND})
	add_subdirectory(deps/zlib-1.2.11)
	add_subdirectory(deps/lua-5.3.5)
	include_directories(MCHawk2 ${ZLIB_DEPS_DIR})
	include_directories(MCHawk2 deps/lua-5.3.5/src)
	target_link_libraries(MCHawk2 PUBLIC zlib)
	target_link_libraries(MCHawk2 PUBLIC LuaLib)
ENDIF (${Lua_FOUND})

