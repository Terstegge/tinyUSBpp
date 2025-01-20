target_sources(${TUPP_TARGET} PRIVATE
        usb_dcd.cpp
        usb_endpoint_rp2350.cpp
)

target_include_directories(${TUPP_TARGET}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
