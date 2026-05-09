if (DEFINED ENV{PICO_SDK_PATH} AND NOT DEFINED PICO_SDK_PATH)
  set (PICO_SDK_PATH $ENV{PICO_SDK_PATH})
  message("Using PICO_SDK_PATH from environment: '${PICO_SDK_PATH}'")
endif()

if (NOT PICO_SDK_PATH)
  message(FATAL_ERROR "PICO_SDK_PATH not specified")
endif()

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)
