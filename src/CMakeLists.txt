add_executable(Lyra
  "main.c"
  "bmp.c"
  "mpu.c"
)

target_link_libraries(Lyra
  pico_stdlib
  pico_stdio_usb
  hardware_i2c
  hardware_spi
)

pico_enable_stdio_usb(Lyra 1)
pico_enable_stdio_uart(Lyra 0)

pico_add_extra_outputs(Lyra)
