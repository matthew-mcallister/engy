cmake_minimum_required(VERSION 3.21)

project(
    Engy
    VERSION 1.0
    DESCRIPTION "Game engine, quick and dirty"
    LANGUAGES CXX
)

file(GLOB_RECURSE sources "src/*.cpp")
file(GLOB_RECURSE headers "src/*.cpp")

add_executable(engy ${sources} ${headers})

find_package(GLEW REQUIRED)
target_link_libraries(engy GLEW::GLEW)

find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
target_link_libraries(engy SDL2::SDL2)
