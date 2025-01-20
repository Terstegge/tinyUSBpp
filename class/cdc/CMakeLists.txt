target_sources(${TUPP_TARGET} INTERFACE
        usb_cdc_acm_device.cpp
        usb_fd_acm.cpp
        usb_fd_call_mgmt.cpp
        usb_fd_header.cpp
        usb_fd_union.cpp
)

target_include_directories(${TUPP_TARGET}
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)
