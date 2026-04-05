cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)
#include(pico_extras_import.cmake)

project(my_project)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(EEE
    main.c
)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries( EEE 
pico_stdlib
#pico_btstack_classic 
#pico_btstack_ble
#pico_btstack_cyw43
#pico_cyw43_driver
pico_cyw43_arch
#pico_cyw43_arch_none
hardware_adc
hardware_pwm
pico_cyw43_arch_lwip_threadsafe_background
)


#include_directories(/home/stefa/rrr3)
target_include_directories(EEE PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        )


pico_enable_stdio_usb(EEE 1)
pico_enable_stdio_uart(EEE 0)


pico_add_extra_outputs(EEE) # create map/bin/hex/uf2 file in addition to ELF.