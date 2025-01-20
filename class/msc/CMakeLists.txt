target_sources(${TUPP_TARGET} INTERFACE
        usb_msc_bot_device.cpp
)

target_include_directories(${TUPP_TARGET}
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)
