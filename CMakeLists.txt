# CMake 最低版本号要求
cmake_minimum_required (VERSION 3.7)

project (Desktop_translator)

set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)
set(OpenGL_GL_PREFERENCE LEGACY)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ./out)
endif()


find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

# find_file(SDL2_INCLUDE_DIR NAME SDL.h HINTS SDL2)
# find_library(SDL2_LIBRARY NAME SDL2)


include_directories(${PROJECT_SOURCE_DIR})
include_directories(common)


add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(common)

# Typically you don't care so much for a third party library's tests to be
# run from your own project's code.
set(JSON_BuildTests OFF CACHE INTERNAL "")

# If you only include this third party in PRIVATE source files, you do not
# need to install it when your main project gets installed.
# set(JSON_Install OFF CACHE INTERNAL "")

# Don't use include(nlohmann_json/CMakeLists.txt) since that carries with it
# unintended consequences that will break the build.  It's generally
# discouraged (although not necessarily well documented as such) to use
# include(...) for pulling in other CMake projects anyways.
add_subdirectory(./3rdparty/json)