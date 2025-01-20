cmake_minimum_required(VERSION 3.16)

# In multi-binary projects every target has to have a different
# name. So combine the target name with the current project name!
set(TUPP_TARGET tinyUSB++_${PROJECT_NAME})

add_library(${TUPP_TARGET} INTERFACE)

# tinyUSB++ uses some C++20 features, and we want
# the code to run as fast as possible :)
target_compile_options(${TUPP_TARGET}
        INTERFACE -Ofast
)
set_property(TARGET ${TUPP_TARGET}
        PROPERTY CXX_STANDARD 20
)

# Check for platform-specific needs.
# RP2350 generates a hard fault after an unaligned access to USB RAM
if (PICO_RP2350 OR RP2350)
    target_compile_definitions(${TUPP_TARGET}
        INTERFACE TUPP_USE_BYTEWISE_MEMCPY=1
    )
endif ()

add_subdirectory(class)
add_subdirectory(drivers)
add_subdirectory(src)

