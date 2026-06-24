cmake_minimum_required(VERSION 4.2)
project(random_c C)

set(CMAKE_C_STANDARD 11)

find_package(SDL3)
add_executable(random_c
        sdl_allme.c
        src/vec2.c)
target_link_libraries(random_c SDL3 m)