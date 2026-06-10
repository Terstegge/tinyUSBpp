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
#ifndef TUPP_USB_HID_DEFINES_H
#define TUPP_USB_HID_DEFINES_H

//////////////////////////////
// HID class specific requests
//////////////////////////////
#define HID_REQUESTS \
    REQ_HID_GET_REPORT      = 0x01, \
    REQ_HID_GET_IDLE        = 0x02, \
    REQ_HID_GET_PROTOCOL    = 0x03, \
    REQ_HID_SET_REPORT      = 0x09, \
    REQ_HID_SET_IDLE        = 0x0A, \
    REQ_HID_SET_PROTOCOL    = 0x0B

///////////////////////////////////////
// HID class specific interface classes
///////////////////////////////////////
#define HID_INTERFACE_CLASSES \
    IF_CLASS_HID    = 0x03

//////////////////////////////////////////
// HID class specific interface subclasses
//////////////////////////////////////////
#define HID_INTERFACE_SUBCLASSES \
    IF_SUBCLASS_HID_NONE    = 0x00, \
    IF_SUBCLASS_HID_BOOT    = 0x01

//////////////////////////////////////////
// HID class specific interface protocols
//////////////////////////////////////////
#define HID_INTERFACE_PROTOCOLS \
    IF_PROTOCOL_HID_NONE        = 0x00, \
    IF_PROTOCOL_HID_KEYBOARD    = 0x01, \
    IF_PROTOCOL_HID_MOUSE       = 0x02

// HID Descriptor Types
#define HID_DESC_TYPE_HID       0x21
#define HID_DESC_TYPE_REPORT    0x22

// HID Protocol values
#define HID_PROTOCOL_BOOT       0x00
#define HID_PROTOCOL_REPORT     0x01

// HID Country Code
#define HID_COUNTRY_NONE        0x00

// Keyboard modifier keys
#define HID_MOD_LEFT_CTRL       0x01
#define HID_MOD_LEFT_SHIFT      0x02
#define HID_MOD_LEFT_ALT        0x04
#define HID_MOD_LEFT_GUI        0x08
#define HID_MOD_RIGHT_CTRL      0x10
#define HID_MOD_RIGHT_SHIFT     0x20
#define HID_MOD_RIGHT_ALT       0x40
#define HID_MOD_RIGHT_GUI       0x80

// Mouse buttons
#define HID_MOUSE_LEFT          0x01
#define HID_MOUSE_RIGHT         0x02
#define HID_MOUSE_MIDDLE        0x04

#endif // TUPP_USB_HID_DEFINES_H
