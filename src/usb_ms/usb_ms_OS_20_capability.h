//    _   _             _    _  _____ ____
//   | | (_)           | |  | |/ ____|  _ \   _     _
//   | |_ _ _ __  _   _| |  | | (___ | |_) |_| |_ _| |_
//   | __| | '_ \| | | | |  | |\___ \|  _ < _   _|_   _|
//   | |_| | | | | |_| | |__| |____) | |_) | |_|   |_|
//    \__|_|_| |_|\__, |\____/|_____/|____/
//                __/ |
//               |___/
//
// This file is part of tinyUSB++, C++ based and easy to
// use library for USB host/device functionality.
// (c) A. Terstegge  (Andreas.Terstegge@gmail.com)
//
// This class represents a MS OS 2.0 capability descriptor.
// It contains a public MS OS 2.0 header, which can be used
// to set up an MS OS 2.0 compatible descriptor tree. It also
// adds the USB request handler (see the .cpp file), which
// will handle the related USB requests.
//
#ifndef USB_MS_OS_20_CAPABILITY_H
#define USB_MS_OS_20_CAPABILITY_H

#include "usb_device_controller.h"
#include "usb_device.h"
#include "usb_bos_dev_cap.h"
#include "usb_ms_parent.h"
#include "usb_ms_header.h"
#include "usb_ms_structs.h"

class usb_ms_OS_20_capability : public usb_bos_dev_cap,
                                public usb_ms_parent {
public:

    usb_ms_OS_20_capability(usb_device_controller & controller,
                            usb_device & device);

    // No copy, no assignment
    usb_ms_OS_20_capability(const usb_ms_OS_20_capability &) = delete;
    usb_ms_OS_20_capability & operator= (const usb_ms_OS_20_capability &) = delete;

    // Methods from usb_bos_dev_cap class
    uint16_t get_bLength() const override {
        return _descriptor.bLength;
    }

    uint8_t * get_desc_ptr() const override {
        return (uint8_t *)&descriptor;
    }

    // Methods from usb_ms_parent
    inline void update() override {
        set_total_length();
    }

    // Read-only version of our descriptor
    const TUPP::dev_cap_platform_ms_os_20_t & descriptor;

    // Public MS OS 2.0 header, so we can add to
    // the descriptor tree
    usb_ms_header header;

    // Windows version 8.1 and newer
    constexpr static uint32_t WIN_VER_81 = 0x06030000;

private:

    // Propagate the total length of the MS OS 2.0
    // descriptor tree into our own descriptor
    void set_total_length() {
        _descriptor.wMSOSDescriptorSetTotalLength =
                header.descriptor.wTotalLength;
    }

    uint16_t prepare_descriptor();
    uint8_t _buffer[256] {0};

    usb_device_controller & _controller;
    usb_device & _device;

    TUPP::dev_cap_platform_ms_os_20_t _descriptor;

    // Platform capability UUID {d8dd60df-4589-4cc7-9cd2-659d9e648a9f}
    // Indicates that this device supports the MS OS 2.0 descriptor
    constexpr static uint8_t UUID[16] =
            { 0xDF, 0x60, 0xDD, 0xD8, 0x89, 0x45, 0xC7, 0x4C,
              0x9C, 0xD2, 0x65, 0x9D, 0x9E, 0x64, 0x8A, 0x9F };
};

#endif // USB_MS_OS_20_CAPABILITY_H
