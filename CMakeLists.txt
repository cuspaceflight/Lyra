cmake_minimum_required(VERSION 4.1)

option(BUILD_EXAMPLES "Build the examples" ON)

set(PICO_BOARD pico2)
set(PICO_PLATFORM rp2350-arm-s)

include(cmake/pico_sdk_import.cmake)

project(Lyra C CXX ASM)

pico_sdk_init()

add_subdirectory(libraries)
add_subdirectory(src)

if (BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

add_compile_options(-Wall -Werror)
