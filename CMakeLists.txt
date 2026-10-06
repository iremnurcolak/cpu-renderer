cmake_minimum_required(VERSION 3.20)

project(cpu_renderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(renderer
    src/line_renderer.cpp
    src/obj_renderer.cpp
    third_party/tinyrenderer/tgaimage.cpp
)

target_include_directories(renderer PUBLIC
    src
    third_party/tinyrenderer
)

add_executable(cpu_renderer src/main.cpp)
target_link_libraries(cpu_renderer PRIVATE renderer)
