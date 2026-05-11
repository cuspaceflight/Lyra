add_library(sx126X STATIC)

add_subdirectory(sx126X)

target_include_directories(sx126X PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_compile_options(sx126X PRIVATE -Wall -Wextra)

target_link_libraries(sx126X
  pico_stdlib
  hardware_spi
)
