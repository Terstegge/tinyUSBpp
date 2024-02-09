target_sources(${PROJECT_NAME} PRIVATE
    usb_cdc_acm_device.cpp
    usb_fd_acm.cpp
    usb_fd_call_mgmt.cpp
    usb_fd_header.cpp
    usb_fd_union.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
