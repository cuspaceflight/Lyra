add_library(icm40609D STATIC)

add_subdirectory(icm40609D)

target_include_directories(icm40609D PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_compile_options(icm40609D PRIVATE -Wall -Wextra)

target_link_libraries(icm40609D
  pico_stdlib
  hardware_spi
)
