if (YAHAL_DIR)
    add_subdirectory(rp2040-YAHAL)

    target_include_directories(${PROJECT_NAME}
        PUBLIC ${YAHAL_DIR}/include/MCU/rp2040
        PUBLIC ${YAHAL_DIR}/include/CPU/CMSIS
    )
elseif (PICO_SDK_PATH)
    add_subdirectory(rp2040-sdk)

    target_include_directories(${PROJECT_NAME}
        PUBLIC ${PICO_SDK_PATH}/src/rp2040/hardware_regs/include
        PUBLIC ${PICO_SDK_PATH}/src/rp2040/hardware_structs/include
        PUBLIC ${PICO_SDK_PATH}/src/rp2_common/pico_platform/include
        PUBLIC ${PICO_SDK_PATH}/src/rp2_common/hardware_base/include
        PUBLIC ${PICO_SDK_PATH}/src/rp2_common/hardware_irq/include
        PUBLIC ${PICO_SDK_PATH}/src/rp2_common/hardware_resets/include
        PUBLIC ${CMAKE_BINARY_DIR}/generated/pico_base
        PUBLIC ${PICO_SDK_PATH}/src/common/pico_base/include
    )
endif ()
