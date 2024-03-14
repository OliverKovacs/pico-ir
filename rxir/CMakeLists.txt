cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(rxir_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()
add_executable(rxir
rxir.c
)
pico_enable_stdio_usb(rxir 1)
pico_enable_stdio_uart(rxir 0)
pico_add_extra_outputs(rxir)
target_link_libraries(rxir pico_stdlib pico_cyw43_arch_none)
