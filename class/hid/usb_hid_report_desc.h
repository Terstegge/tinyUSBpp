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
// This file provides macros for building HID Report Descriptors
// in a readable way, instead of using raw 'magic numbers'.
// Based on the HID Usage Tables specification (HID 1.11).
//
#ifndef TUPP_USB_HID_REPORT_DESC_H
#define TUPP_USB_HID_REPORT_DESC_H

#include <cstdint>

//--------------------------------------------------------------------
// Basic Report Item macro
// Generates one descriptor byte + optional data bytes
//--------------------------------------------------------------------
#define HID_REPORT_DATA_0(data)
#define HID_REPORT_DATA_1(data)   , (uint8_t)(data)
#define HID_REPORT_DATA_2(data)   , (uint8_t)((data) & 0xFF) \
                                  , (uint8_t)(((data) >> 8) & 0xFF)

#define HID_REPORT_ITEM(data, tag, type, size) \
    (uint8_t)(((tag) << 4) | ((type) << 2) | (size)) \
    HID_REPORT_DATA_##size(data)

//--------------------------------------------------------------------
// Report Item Types
//--------------------------------------------------------------------
#define RI_TYPE_MAIN    0
#define RI_TYPE_GLOBAL  1
#define RI_TYPE_LOCAL   2

//--------------------------------------------------------------------
// Main Items
//--------------------------------------------------------------------
#define RI_MAIN_INPUT           8
#define RI_MAIN_OUTPUT          9
#define RI_MAIN_COLLECTION      10
#define RI_MAIN_FEATURE         11
#define RI_MAIN_COLLECTION_END  12

//--------------------------------------------------------------------
// Global Items
//--------------------------------------------------------------------
#define RI_GLOBAL_USAGE_PAGE    0
#define RI_GLOBAL_LOGICAL_MIN   1
#define RI_GLOBAL_LOGICAL_MAX   2
#define RI_GLOBAL_REPORT_SIZE   7
#define RI_GLOBAL_REPORT_COUNT  9

//--------------------------------------------------------------------
// Local Items
//--------------------------------------------------------------------
#define RI_LOCAL_USAGE          0
#define RI_LOCAL_USAGE_MIN      1
#define RI_LOCAL_USAGE_MAX      2

//--------------------------------------------------------------------
// Readable macros for descriptor items
//--------------------------------------------------------------------
#define HID_USAGE_PAGE(x)         HID_REPORT_ITEM(x, RI_GLOBAL_USAGE_PAGE,  RI_TYPE_GLOBAL, 1)
#define HID_USAGE(x)              HID_REPORT_ITEM(x, RI_LOCAL_USAGE,        RI_TYPE_LOCAL,  1)
#define HID_USAGE_MIN(x)          HID_REPORT_ITEM(x, RI_LOCAL_USAGE_MIN,    RI_TYPE_LOCAL,  1)
#define HID_USAGE_MAX(x)          HID_REPORT_ITEM(x, RI_LOCAL_USAGE_MAX,    RI_TYPE_LOCAL,  1)
#define HID_LOGICAL_MIN(x)        HID_REPORT_ITEM(x, RI_GLOBAL_LOGICAL_MIN, RI_TYPE_GLOBAL, 1)
#define HID_LOGICAL_MAX(x)        HID_REPORT_ITEM(x, RI_GLOBAL_LOGICAL_MAX, RI_TYPE_GLOBAL, 1)
#define HID_REPORT_SIZE(x)        HID_REPORT_ITEM(x, RI_GLOBAL_REPORT_SIZE, RI_TYPE_GLOBAL, 1)
#define HID_REPORT_COUNT(x)       HID_REPORT_ITEM(x, RI_GLOBAL_REPORT_COUNT,RI_TYPE_GLOBAL, 1)
#define HID_COLLECTION(x)         HID_REPORT_ITEM(x, RI_MAIN_COLLECTION,    RI_TYPE_MAIN,   1)
#define HID_COLLECTION_END        HID_REPORT_ITEM(0, RI_MAIN_COLLECTION_END,RI_TYPE_MAIN,   0)
#define HID_INPUT(x)              HID_REPORT_ITEM(x, RI_MAIN_INPUT,         RI_TYPE_MAIN,   1)
#define HID_OUTPUT(x)             HID_REPORT_ITEM(x, RI_MAIN_OUTPUT,        RI_TYPE_MAIN,   1)

//--------------------------------------------------------------------
// Usage Pages
//--------------------------------------------------------------------
#define HID_USAGE_PAGE_DESKTOP    0x01
#define HID_USAGE_PAGE_KEYBOARD   0x07
#define HID_USAGE_PAGE_BUTTON     0x09

//--------------------------------------------------------------------
// Desktop Usages
//--------------------------------------------------------------------
#define HID_USAGE_DESKTOP_POINTER   0x01
#define HID_USAGE_DESKTOP_MOUSE     0x02
#define HID_USAGE_DESKTOP_KEYBOARD  0x06
#define HID_USAGE_DESKTOP_X         0x30
#define HID_USAGE_DESKTOP_Y         0x31
#define HID_USAGE_DESKTOP_WHEEL     0x38

//--------------------------------------------------------------------
// Collection types
//--------------------------------------------------------------------
#define HID_COLLECTION_APPLICATION  0x01
#define HID_COLLECTION_PHYSICAL     0x00

//--------------------------------------------------------------------
// Input/Output flags
//--------------------------------------------------------------------
#define HID_DATA        (0 << 0)
#define HID_CONSTANT    (1 << 0)
#define HID_ARRAY       (0 << 1)
#define HID_VARIABLE    (1 << 1)
#define HID_ABSOLUTE    (0 << 2)
#define HID_RELATIVE    (1 << 2)

//--------------------------------------------------------------------
// Pre-defined standard descriptors
//--------------------------------------------------------------------

// Standard Keyboard Report Descriptor
#define HID_DESC_KEYBOARD \
    HID_USAGE_PAGE (HID_USAGE_PAGE_DESKTOP),    \
    HID_USAGE      (HID_USAGE_DESKTOP_KEYBOARD),\
    HID_COLLECTION (HID_COLLECTION_APPLICATION),\
    HID_USAGE_PAGE (HID_USAGE_PAGE_KEYBOARD),   \
    HID_USAGE_MIN  (224),                        \
    HID_USAGE_MAX  (231),                        \
    HID_LOGICAL_MIN(0),                          \
    HID_LOGICAL_MAX(1),                          \
    HID_REPORT_SIZE(1),                          \
    HID_REPORT_COUNT(8),                         \
    HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
    HID_REPORT_COUNT(1),                         \
    HID_REPORT_SIZE(8),                          \
    HID_INPUT(HID_CONSTANT),                     \
    HID_REPORT_COUNT(6),                         \
    HID_REPORT_SIZE(8),                          \
    HID_LOGICAL_MIN(0),                          \
    HID_LOGICAL_MAX(101),                        \
    HID_USAGE_PAGE (HID_USAGE_PAGE_KEYBOARD),   \
    HID_USAGE_MIN  (0),                          \
    HID_USAGE_MAX  (101),                        \
    HID_INPUT(HID_DATA | HID_ARRAY | HID_ABSOLUTE), \
    HID_COLLECTION_END

// Standard Mouse Report Descriptor
#define HID_DESC_MOUSE \
    HID_USAGE_PAGE (HID_USAGE_PAGE_DESKTOP),   \
    HID_USAGE      (HID_USAGE_DESKTOP_MOUSE),  \
    HID_COLLECTION (HID_COLLECTION_APPLICATION),\
    HID_USAGE      (HID_USAGE_DESKTOP_POINTER),\
    HID_COLLECTION (HID_COLLECTION_PHYSICAL),  \
    HID_USAGE_PAGE (HID_USAGE_PAGE_BUTTON),    \
    HID_USAGE_MIN  (1),                         \
    HID_USAGE_MAX  (3),                         \
    HID_LOGICAL_MIN(0),                         \
    HID_LOGICAL_MAX(1),                         \
    HID_REPORT_COUNT(3),                        \
    HID_REPORT_SIZE(1),                         \
    HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
    HID_REPORT_COUNT(1),                        \
    HID_REPORT_SIZE(5),                         \
    HID_INPUT(HID_CONSTANT),                    \
    HID_USAGE_PAGE (HID_USAGE_PAGE_DESKTOP),   \
    HID_USAGE      (HID_USAGE_DESKTOP_X),      \
    HID_USAGE      (HID_USAGE_DESKTOP_Y),      \
    HID_LOGICAL_MIN(0x81),                      \
    HID_LOGICAL_MAX(0x7F),                      \
    HID_REPORT_SIZE(8),                         \
    HID_REPORT_COUNT(2),                        \
    HID_INPUT(HID_DATA | HID_VARIABLE | HID_RELATIVE), \
    HID_USAGE      (HID_USAGE_DESKTOP_WHEEL),  \
    HID_LOGICAL_MIN(0x81),                      \
    HID_LOGICAL_MAX(0x7F),                      \
    HID_REPORT_SIZE(8),                         \
    HID_REPORT_COUNT(1),                        \
    HID_INPUT(HID_DATA | HID_VARIABLE | HID_RELATIVE), \
    HID_COLLECTION_END,                         \
    HID_COLLECTION_END

#endif // TUPP_USB_HID_REPORT_DESC_H
