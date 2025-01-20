target_sources(tinyUSB++ INTERFACE
    usb_dcd.cpp
    usb_endpoint_rp2040.cpp
)

target_include_directories(tinyUSB++ INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
