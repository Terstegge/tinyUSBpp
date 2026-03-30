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
//
#include "usb_ms_OS_20_capability.h"
#include "usb_bos.h"
#include "usb_log.h"
using enum usb_log::log_level;

usb_ms_OS_20_capability::usb_ms_OS_20_capability(usb_bos & bos)
: usb_ms_parent((uint8_t *)&_descriptor, sizeof(_descriptor) ), descriptor(_descriptor)
{
    TUPP_LOG(LOG_DEBUG, "usb_ms_OS_20_capability() @%x", this);
    // Set our capability descriptor
    _descriptor.bLength             = sizeof(TUPP::dev_cap_platform_ms_os_20_t);
    _descriptor.bDescriptorType     = TUPP::bDescriptorType_t::DESC_DEVICE_CAPABILITY;
    _descriptor.bDevCapabilityType  = TUPP::bDevCapabilityType_t::CAP_PLATFORM;
    _descriptor.bReserved           = 0;
    _descriptor.dwWindowsVersion    = TUPP::WIN_VER_81;
    _descriptor.bMS_VendorCode      = TUPP::GET_MS_OS20_DESC;
    _descriptor.bAltEnumCode        = 0;

    // Set the UUID
    for(size_t i=0; i < sizeof(_descriptor.PlatformCapabilityUUID); ++i) {
        _descriptor.PlatformCapabilityUUID[i] = TUPP::UUID_ms_os_20[i];
    }
    set_wMSOSDescriptorSetTotalLength();

    // We are the parent of the MS OS 2.0 header
    header.set_dwWindowsVersion(TUPP::WIN_VER_81);
    header.set_parent(this);

    // Set the header in the BOS descriptor
    bos.set_ms_header(&header);

    // Add this capability to the BOS
    bos.add_capability(this);
}
