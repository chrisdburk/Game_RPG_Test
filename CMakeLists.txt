cmake_minimum_required(VERSION 3.12)
project(ShackledDragon LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
add_executable(rpg ${SOURCES})
target_include_directories(rpg PRIVATE "${CMAKE_SOURCE_DIR}/src")

if(MSVC)
    target_compile_options(rpg PRIVATE /W4)
else()
    target_compile_options(rpg PRIVATE -Wall -Wextra -Wpedantic)
endif()
