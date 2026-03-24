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
#ifndef TUPP_USB_MS_DEV_CAP_PLATFORM_H
#define TUPP_USB_MS_DEV_CAP_PLATFORM_H

#include <cstdio>

#include "usb_bos.h"
#include "usb_bos_dev_cap.h"
#include "usb_ms_header.h"
#include "usb_ms_structs.h"

class usb_ms_dev_cap_platform : public usb_bos_dev_cap, public usb_ms_parent {
public:
    usb_ms_dev_cap_platform() : descriptor(_descriptor) {
        _descriptor.bLength             = sizeof(TUPP::dev_cap_platform_ms_os_20_t);
        _descriptor.bDescriptorType     = TUPP::bDescriptorType_t::DESC_DEVICE_CAPABILITY;
        _descriptor.bDevCapabilityType  = TUPP::bDevCapabilityType_t::CAP_PLATFORM;
        _descriptor.bReserved           = 0;
        set_total_length();
    }

    // No copy, no assignment
    usb_ms_dev_cap_platform(const usb_ms_dev_cap_platform &) = delete;
    usb_ms_dev_cap_platform & operator= (const usb_ms_dev_cap_platform &) = delete;

    uint16_t get_bLength() override {
        return _descriptor.bLength;
    }

    uint8_t * get_desc_ptr() override {
        return (uint8_t *)&descriptor;
    }

    inline void set_PlatformCapabilityUUID(const uint8_t uuid[16]) {
        for(int i=0; i < 16; ++i) {
            _descriptor.PlatformCapabilityUUID[i] = uuid[i];
        }
    }

    inline void set_dwWindowsVersion(const uint32_t ver) {
        _descriptor.dwWindowsVersion = ver;
    }

    inline void set_bMS_VendorCode(const TUPP::bRequest_t code) {
        _descriptor.bMS_VendorCode = code;
    }

    inline void set_bAltEnumCode(const uint8_t code) {
        _descriptor.bAltEnumCode = code;
    }

    inline void add_ms_header(usb_ms_header & h) {
        _ms_header = &h;
        h.set_parent(this);
        set_total_length();
    }

    // Read-only version of our descriptor
    const TUPP::dev_cap_platform_ms_os_20_t & descriptor;

    inline void update() override {
        set_total_length();
    }

private:

    // Propagate the total length of the MS OS 2.0
    // descriptor tree into our own descriptor
    void set_total_length() {
        if (_ms_header) {
            _descriptor.wMSOSDescriptorSetTotalLength =
                    _ms_header->descriptor.wTotalLength;
        }
    }

    // Child object
    usb_ms_header * _ms_header {nullptr};

    TUPP::dev_cap_platform_ms_os_20_t _descriptor;
};

#endif  // TUPP_USB_MS_DEV_CAP_PLATFORM_H
