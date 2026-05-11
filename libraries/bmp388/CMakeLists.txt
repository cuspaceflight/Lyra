add_library(bmp388 STATIC)

add_subdirectory(bmp388)

target_include_directories(bmp388 PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_compile_options(bmp388 PRIVATE -Wall -Wextra)

target_link_libraries(bmp388
  pico_stdlib
  hardware_i2c
)
