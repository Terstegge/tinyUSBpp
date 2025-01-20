if (YAHAL_DIR)
    if (${YAHAL_MCU} STREQUAL "rp2040")
        add_subdirectory(rp2040-YAHAL)
        target_include_directories(${tinyUSB++_TARGET}
                INTERFACE ${YAHAL_DIR}/include/MCU/rp2040
                INTERFACE ${YAHAL_DIR}/include/CPU/CMSIS
        )
    elseif (${YAHAL_MCU} STREQUAL "rp2350")
        add_subdirectory(rp2350-YAHAL)
        target_include_directories(${tinyUSB++_TARGET}
                INTERFACE ${YAHAL_DIR}/include/MCU/rp2350
                INTERFACE ${YAHAL_DIR}/include/CPU/CMSIS
        )
    else ()
        message(FATAL_ERROR "Unsupported YAHAL MCU: ${YAHAL_MCU}")
    endif ()
elseif (PICO_SDK_PATH)
    if (PICO_RP2040)
        add_subdirectory(rp2040-sdk)
        target_link_libraries(${TUPP_TARGET} INTERFACE pico_stdlib)
    elseif (PICO_RP2350)
        add_subdirectory(rp2350-sdk)
        target_link_libraries(${TUPP_TARGET} INTERFACE pico_stdlib)
    else ()
        message(FATAL_ERROR "Unsupported PICO MCU")
    endif ()
endif ()
