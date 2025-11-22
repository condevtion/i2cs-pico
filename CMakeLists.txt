cmake_minimum_required(VERSION 3.18)

include("$ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake")

project(pico2_blink C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(i2cs-pico
	main.c
	bus.h
	bus.c
)

add_compile_definitions(
	$<$<CONFIG:Debug>:DEBUG>
)

pico_enable_stdio_usb(i2cs-pico 1)
pico_enable_stdio_uart(i2cs-pico 0)

pico_add_extra_outputs(i2cs-pico)

target_link_libraries(i2cs-pico pico_stdlib hardware_i2c)
