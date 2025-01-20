target_sources(tinyUSB++ INTERFACE
    usb_msc_bot_device.cpp
)

target_include_directories(tinyUSB++ INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
