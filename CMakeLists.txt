cmake_minimum_required(VERSION 3.13)

# initialize pico-sdk from GIT
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_FETCH_FROM_GIT on)

# pico_sdk_import.cmake is a single file copied from this SDK
# note: this must happen before project()
include(pico_sdk_import.cmake)
# include(example_auto_set_url.cmake)

project(pico_water)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

if (TARGET tinyusb_device)
    add_executable(pico_water
            pico_water.c
            )

    # pull in common dependencies
    target_link_libraries(pico_water
                          pico_stdlib
                          pico_multicore
                          pico_util
                          hardware_adc)

    # enable usb output, disable uart output
    pico_enable_stdio_usb(pico_water 1)
    pico_enable_stdio_uart(pico_water 0)

    # create map/bin/hex/uf2 file etc.
    pico_add_extra_outputs(pico_water)

    # add url via pico_set_program_url
    # example_auto_set_url(pico_water)
elseif(PICO_ON_DEVICE)
    message(WARNING "not building pico_water because TinyUSB submodule is not initialized in the SDK")
endif()
