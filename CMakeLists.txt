cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(wacom_wireless C CXX ASM)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

pico_sdk_init()

add_executable(wacom_wireless
    src/main.c
    src/usb_host.c
    src/wacom_hid.c
)

target_sources(wacom_wireless PRIVATE
    src/transport_ble.c
    src/transport_wifi.c
)

target_include_directories(wacom_wireless PRIVATE src tusb_config)

target_link_libraries(wacom_wireless PRIVATE tinyusb_host)

target_link_libraries(wacom_wireless PRIVATE
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    pico_btstack_ble
    pico_btstack_cyw43
    hardware_gpio
)

pico_set_cyw43_config(wacom_wireless)
pico_enable_stdio_usb(wacom_wireless 1)
pico_enable_stdio_uart(wacom_wireless 1)
pico_add_extra_outputs(wacom_wireless)
