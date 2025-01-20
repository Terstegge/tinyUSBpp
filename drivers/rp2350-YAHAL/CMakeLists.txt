target_sources(${TUPP_TARGET} INTERFACE
        usb_dcd.cpp
        usb_endpoint_rp2350.cpp
)

target_include_directories(${TUPP_TARGET}
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)
