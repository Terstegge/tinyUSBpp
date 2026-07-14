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
#ifndef TUPP_USB_HID_STRUCTS_H
#define TUPP_USB_HID_STRUCTS_H

#include <cstdint>

namespace HID {

    // HID Keyboard report (8 bytes)
    struct keyboard_report_t {
        uint8_t modifier;     // Modifier keys (Shift, Ctrl, Alt...)
        uint8_t reserved;     // Always 0
        uint8_t keycode[6];   // Up to 6 keys pressed simultaneously
    } __attribute__((packed));

    // HID Mouse report (4 bytes)
    struct mouse_report_t {
        uint8_t buttons;  // Bit0=Left, Bit1=Right, Bit2=Middle
        int8_t  x;        // X movement (-127 to +127)
        int8_t  y;        // Y movement (-127 to +127)
        int8_t  wheel;    // Scroll wheel (-127 to +127)
    } __attribute__((packed));

    // HID Descriptor
    struct hid_descriptor_t {
        uint8_t  bLength;
        uint8_t  bDescriptorType;
        uint16_t bcdHID;
        uint8_t  bCountryCode;
        uint8_t  bNumDescriptors;
        uint8_t  bDescriptorType2;
        uint16_t wDescriptorLength;
    } __attribute__((packed));

} // namespace HID

#endif // TUPP_USB_HID_STRUCTS_H