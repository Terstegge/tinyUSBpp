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
// (c) 2024 A. Terstegge  (Andreas.Terstegge@gmail.com)
//
#ifndef TUPP_USB_BOS_DEV_CAP_WEBUSB_MS_H
#define TUPP_USB_BOS_DEV_CAP_WEBUSB_MS_H

#include "usb_ms_structs.h"
#include "usb_bos.h"
#include "usb_bos_dev_cap.h"
#include "usb_strings.h"

class usb_ms_dev_cap_webusb : public usb_bos_dev_cap {
public:
    explicit usb_ms_dev_cap_webusb(usb_bos & bos) : descriptor{_descriptor}
    {
        _descriptor.bLength             = sizeof(USB::dev_cap_platform_ms_webusb_t);
        _descriptor.bDescriptorType     = USB::bDescriptorType_t::DESC_DEVICE_CAPABILITY;
        _descriptor.bDevCapabilityType  = USB::bDevCapabilityType_t::CAP_PLATFORM;
        _descriptor.bReserved           = 0;
        bos.add_capability(this);
    }

    // No copy, no assignment
    usb_ms_dev_cap_webusb(const usb_ms_dev_cap_webusb &) = delete;
    usb_ms_dev_cap_webusb & operator= (const usb_ms_dev_cap_webusb &) = delete;

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

    inline void set_bcdVersion(uint16_t ver) {
        _descriptor.bcdVersion = ver;
    }

    inline void set_bVendorCode(USB::bRequest_t code) {
        _descriptor.bVendorCode = code;
    }

    inline void set_iLandingPage(const char * url) {
        _descriptor.iLandingPage = usb_strings::inst.add_string(url);
    }

    // Read-only version of our descriptor
    const USB::dev_cap_platform_ms_webusb_t & descriptor;

    friend class usb_ms_header;

private:
    // The binary object store (BOS) descriptor
    USB::dev_cap_platform_ms_webusb_t _descriptor;
};

#endif  // TUPP_USB_BOS_DEV_CAP_WEBUSB_MS_H
