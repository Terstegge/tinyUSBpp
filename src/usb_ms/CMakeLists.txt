target_sources(${PROJECT_NAME} PRIVATE
    usb_ms_compatible_ID.cpp
    usb_ms_config_subset.cpp
    usb_ms_func_subset.cpp
    usb_ms_header.cpp
    usb_ms_registry_property.cpp
    usb_ms_compat_descriptor.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

