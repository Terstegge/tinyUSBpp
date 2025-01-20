cmake_minimum_required(VERSION 3.16)

#set(PICO_SDK_PATH ~/github/pico-sdk)
set(PICO_BOARD pico)
#set(PICO_BOARD pico2)

# tinyUSB++ needs some features of this C++20
set(CMAKE_CXX_STANDARD 20)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(cdc_acm_dev_pdp8 C CXX ASM)

# Initialize the SDK
pico_sdk_init()

add_executable(cdc_acm_dev_pdp8
    cdc_acm_dev_pdp8.cpp
)
add_subdirectory(src)

add_subdirectory(../.. tinyUSB++)

# pull in needed libraries
target_link_libraries(cdc_acm_dev_pdp8
        tinyUSB++
        pico_stdlib
)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(cdc_acm_dev_pdp8)

