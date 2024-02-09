target_sources(${PROJECT_NAME} PRIVATE
    usb_msc_bot_device.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
