target_sources(${TUPP_TARGET} INTERFACE
        usb_hcd.cpp
)
target_include_directories(${TUPP_TARGET}
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)
