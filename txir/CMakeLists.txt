cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(txir_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()
add_executable(txir
txir.c
)
pico_enable_stdio_usb(txir 1)
pico_enable_stdio_uart(txir 0)
pico_add_extra_outputs(txir)
target_link_libraries(txir pico_stdio pico_stdlib pico_cyw43_arch_none)
# pico_cyw43_arch_lwip_threadsafe_background

target_include_directories(txir PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        )
