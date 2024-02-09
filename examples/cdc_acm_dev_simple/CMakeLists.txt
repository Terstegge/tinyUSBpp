cmake_minimum_required(VERSION 3.12)

# set(PICO_SDK_PATH ~/github/pico-sdk)
set(PICO_BOARD pico)

# tinyUSB++ needs some features of this C++ standard
set(CMAKE_CXX_STANDARD 20)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(cdc_acm_dev_simple C CXX ASM)

# Initialize the SDK
pico_sdk_init()

add_subdirectory(tinyUSB++)

add_executable(cdc_acm_dev_simple
    cdc_acm_dev_simple.cpp
)

# pull in needed libraries
target_link_libraries(cdc_acm_dev_simple pico_stdlib tinyUSB++_cdc_acm_dev_simple)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(cdc_acm_dev_simple)
