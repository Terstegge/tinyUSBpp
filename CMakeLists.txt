cmake_minimum_required(VERSION 3.16)

add_library(tinyUSB++ INTERFACE)

# tinyUSB++ uses some C++20 features, and we want
# the code to run as fast as possible :)
target_compile_options(tinyUSB++ INTERFACE -Ofast)
set_property(TARGET tinyUSB++ PROPERTY CXX_STANDARD 20)

# Check for platform-specific needs.
# RP2350 generates a hard fault after an unaligned access to USB RAM
if (PICO_RP2350 OR RP2350)
    target_compile_definitions(tinyUSB++ INTERFACE TUPP_USE_BYTEWISE_MEMCPY=1)
endif ()

add_subdirectory(class)
add_subdirectory(drivers)
add_subdirectory(src)

