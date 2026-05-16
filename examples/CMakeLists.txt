file(GLOB EXAMPLE_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)

foreach(DIR ${EXAMPLE_DIRS})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${DIR})
        # Look for a main.cpp in each folder
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${DIR}/main.c)
            add_executable(example_${DIR} ${DIR}/main.c)
            target_link_libraries(example_${DIR} PRIVATE
              pico_stdlib
              pico_stdio_usb
              bmp388
              icm40609D
              sx126X
            )

            pico_enable_stdio_usb(example_${DIR} 1)
            pico_enable_stdio_uart(example_${DIR} 0)

            pico_add_extra_outputs(example_${DIR})

            # Organize them into a folder in IDEs like Visual Studio/Xcode
            set_target_properties(example_${DIR} PROPERTIES FOLDER "Examples")
        endif()
    endif()
endforeach()
