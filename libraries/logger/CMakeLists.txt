add_library(logger STATIC)

add_subdirectory(logger)

target_include_directories(logger PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_compile_options(logger PRIVATE -Wall -Wextra)

target_link_libraries(logger
  pico_stdlib
)

if (DEFINED LOG_ENABLE)
  message("Enable logger")
  target_compile_definitions(logger PUBLIC LOG_ENABLE)
endif()

if (DEFINED LOG_LEVEL)
  message("Set log level ${LOG_LEVEL}")
  target_compile_definitions(logger PUBLIC LOG_LEVEL=${LOG_LEVEL})
endif()

if (DEFINED LOG_ENABLE_COLOUR)
  target_compile_definitions(logger PUBLIC LOG_ENABLE_COLOUR)
endif()
