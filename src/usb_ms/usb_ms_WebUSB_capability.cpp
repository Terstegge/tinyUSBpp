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
#include "usb_ms_WebUSB_capability.h"
#include "usb_bos.h"
#include "usb_strings.h"
#include "usb_log.h"
using enum usb_log::log_level;
using enum TUPP::URL_FORMAT;

usb_ms_WebUSB_capability::usb_ms_WebUSB_capability(usb_bos & bos, TUPP::URL_FORMAT fmt, const char * url)
: usb_ms_parent((uint8_t *)&_descriptor, sizeof(_descriptor)),
  descriptor(_descriptor)
{
    TUPP_LOG(LOG_DEBUG, "usb_ms_WebUSB_capability() @%x", this);
    // Set our capability descriptor
    _descriptor.bLength             = sizeof(TUPP::dev_cap_platform_ms_webusb_t);
    _descriptor.bDescriptorType     = TUPP::bDescriptorType_t::DESC_DEVICE_CAPABILITY;
    _descriptor.bDevCapabilityType  = TUPP::bDevCapabilityType_t::CAP_PLATFORM;
    _descriptor.bReserved           = 0;
    _descriptor.bcdVersion          = 0x0100;
    _descriptor.bVendorCode         = TUPP::GET_MS_WEBUSB;
    _descriptor.iLandingPage        = usb_strings::inst.add_string(url);

    // Set the UUID
    for(size_t i=0; i < sizeof(_descriptor.PlatformCapabilityUUID); ++i) {
        _descriptor.PlatformCapabilityUUID[i] = TUPP::UUID_webusb[i];
    }

    // Set the URL format in the BOS descriptor
    bos.set_url_format(fmt);

    // Add this capability to the BOS
    bos.add_capability(this);
}
