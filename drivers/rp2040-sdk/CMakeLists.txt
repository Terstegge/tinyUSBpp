target_sources(${PROJECT_NAME} PRIVATE
    usb_dcd_rp2040.cpp
    usb_endpoint_rp2040.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
