cmake_minimum_required(VERSION 3.20)

project(cpu_renderer LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(renderer
    src/colors.c
    src/line_renderer.c
    src/triangle_renderer.c
    src/obj_renderer.c
    third_party/tinyrenderer/tgaimage.c
)

target_include_directories(renderer PUBLIC
    src
    third_party/tinyrenderer
)

if(NOT MSVC)
    target_link_libraries(renderer PUBLIC m)
    target_compile_options(renderer PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(cpu_renderer src/main.c)
target_link_libraries(cpu_renderer PRIVATE renderer)

include(CTest)
if(BUILD_TESTING)
    add_executable(renderer_tests tests/renderer_tests.c)
    target_link_libraries(renderer_tests PRIVATE renderer)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test-output")
    add_test(NAME renderer_tests COMMAND renderer_tests)
    set_tests_properties(renderer_tests PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test-output")
endif()
