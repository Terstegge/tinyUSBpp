target_sources(tinyUSB++ INTERFACE
    usb_cdc_acm_device.cpp
    usb_fd_acm.cpp
    usb_fd_call_mgmt.cpp
    usb_fd_header.cpp
    usb_fd_union.cpp
)

target_include_directories(tinyUSB++ INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
