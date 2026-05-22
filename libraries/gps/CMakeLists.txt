add_library(gps STATIC)

add_subdirectory(gps)

target_include_directories(gps PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_compile_options(gps PRIVATE -Wall -Wextra)

target_link_libraries(gps
  pico_stdlib
  hardware_i2c

  logger
)
