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
// (c) 2023 A. Terstegge  (Andreas.Terstegge@gmail.com)
//
#ifndef TUPP_USB_BOS_DEV_CAP_PLATFORM_MS_H
#define TUPP_USB_BOS_DEV_CAP_PLATFORM_MS_H

#include "usb_bos.h"
#include "usb_bos_dev_cap.h"
#include "usb_ms_structs.h"

class usb_ms_dev_cap_platform : public usb_bos_dev_cap {
public:
    explicit usb_ms_dev_cap_platform(usb_bos & bos) : descriptor(_descriptor)
    {
        _descriptor.bLength             = sizeof(USB::dev_cap_platform_ms_os_20_t);
        _descriptor.bDescriptorType     = USB::bDescriptorType_t::DESC_DEVICE_CAPABILITY;
        _descriptor.bDevCapabilityType  = USB::bDevCapabilityType_t::CAP_PLATFORM;
        _descriptor.bReserved           = 0;
        _descriptor.wMSOSDescriptorSetTotalLength = 0;
        bos.add_capability(this);
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

    inline void set_PlatformCapabilityUUID(uint8_t uuid[16]) {
        for(int i=0; i < 16; ++i) {
            _descriptor.PlatformCapabilityUUID[i] = uuid[i];
        }
    }

    inline void set_dwWindowsVersion(uint32_t ver) {
        _descriptor.dwWindowsVersion = ver;
    }

    inline void set_bMS_VendorCode(USB::bRequest_t code) {
        _descriptor.bMS_VendorCode = code;
    }

    inline void set_bAltEnumCode(uint8_t code) {
        _descriptor.bAltEnumCode = code;
    }

    // Read-only version of our descriptor
    const USB::dev_cap_platform_ms_os_20_t & descriptor;

    friend class usb_ms_header;

private:
    // The binary object store (BOS) descriptor
    USB::dev_cap_platform_ms_os_20_t _descriptor;
};

#endif  // TUPP_USB_BOS_DEV_CAP_PLATFORM_MS_H
