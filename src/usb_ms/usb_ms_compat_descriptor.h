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
// This class adds a Microsoft compatible WebUSB descriptor
// to the USB device.
//
#ifndef USB_MS_WEBUSB_DESCRIPTOR_H
#define USB_MS_WEBUSB_DESCRIPTOR_H

#include "usb_device_controller.h"
#include "usb_bos.h"
#include "usb_device.h"
#include "usb_ms_dev_cap_webusb.h"
#include "usb_ms_dev_cap_platform.h"
#include "usb_ms_header.h"
#include "usb_ms_func_subset.h"
#include "usb_ms_config_subset.h"
#include "usb_ms_compatible_ID.h"
#include "usb_ms_registry_property.h"

class usb_ms_compat_descriptor {

public:
    explicit usb_ms_compat_descriptor(usb_device_controller & controller,
                                      usb_device & device);

private:
    uint16_t prepare_descriptor();

    usb_device_controller &     _controller;
    usb_device &                _device;

    // The WebUSB descriptor structure, based on a BOS (Binary Object Storage)
    usb_bos                     _bos;
//    usb_bos_dev_cap_webusb_ms   _web_platform       {_bos};
    usb_ms_dev_cap_platform _cap_platform       {_bos};
    usb_ms_header               _ms_header          {_cap_platform};
    usb_ms_config_subset        _ms_config_subset   {_ms_header};
    usb_ms_func_subset          _ms_func_subset     {_ms_config_subset};
    usb_ms_compatible_ID        _ms_compat_id       {_ms_func_subset};
    usb_ms_registry_property    _ms_reg_prop        {_ms_func_subset};

    uint8_t _buffer[256] {0};
};

#endif  //USB_MS_WEBUSB_DESCRIPTOR_H
