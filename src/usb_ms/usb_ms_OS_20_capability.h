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
// This class represents an MS OS 2.0 capability descriptor.
// It contains a public MS OS 2.0 header, which can be used
// to set up an MS OS 2.0 compatible descriptor tree. It also
// adds the USB request handler (see the .cpp file), which
// will handle the related USB requests.
//
#ifndef USB_MS_OS_20_CAPABILITY_H
#define USB_MS_OS_20_CAPABILITY_H

class usb_bos;
#include "usb_ms_parent.h"
#include "usb_ms_header.h"
#include "usb_ms_structs.h"

class usb_ms_OS_20_capability : public usb_ms_parent {
public:

    explicit usb_ms_OS_20_capability(usb_bos & bos);

    // No copy, no assignment
    usb_ms_OS_20_capability(const usb_ms_OS_20_capability &) = delete;
    usb_ms_OS_20_capability & operator= (const usb_ms_OS_20_capability &) = delete;

    // Methods from usb_ms_parent
    inline void update() override {
        set_wMSOSDescriptorSetTotalLength();
    }
    void set_total_length() override { }

    // Read-only version of our descriptor
    const TUPP::dev_cap_platform_ms_os_20_t & descriptor;

    // Public MS OS 2.0 header, so we can add to
    // the descriptor tree
    usb_ms_header header;

private:
    // Propagate the total length of the MS OS 2.0
    // descriptor tree into our own descriptor
    void set_wMSOSDescriptorSetTotalLength() {
        _descriptor.wMSOSDescriptorSetTotalLength =
                header.descriptor.wTotalLength;
    }

    TUPP::dev_cap_platform_ms_os_20_t _descriptor;
};

#endif // USB_MS_OS_20_CAPABILITY_H
