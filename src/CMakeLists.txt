target_sources(${PROJECT_NAME} PRIVATE
    usb_bos.cpp
    usb_configuration.cpp
    usb_device.cpp
    usb_device_controller.cpp
    usb_endpoint.cpp
    usb_interface.cpp
    usb_interface_association.cpp
    usb_log.cpp
    usb_strings.cpp
)

add_subdirectory(usb_ms)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
