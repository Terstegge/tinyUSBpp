cmake_minimum_required(VERSION 3.12)

# set(PICO_SDK_PATH ~/github/pico-sdk)
set(PICO_BOARD pico)

# tinyUSB++ needs some features of this C++ standard
set(CMAKE_CXX_STANDARD 20)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(msc_ram_drive C CXX ASM)

# Initialize the SDK
pico_sdk_init()

add_subdirectory(../.. tinyUSB++)

add_executable(msc_ram_drive
    msc_ram_drive.cpp
    image.cpp
)

# pull in needed libraries
target_link_libraries(msc_ram_drive pico_stdlib tinyUSB++_msc_ram_drive)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(msc_ram_drive)
