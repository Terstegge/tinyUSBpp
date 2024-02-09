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
// This is a small utility class for handling strings in
// USB descriptors. The maximum number of stored strings
// is fixed (see TUPP_MAX_STRINGS in usb_config.h).
//
#ifndef TUPP_USB_STRINGS_H
#define TUPP_USB_STRINGS_H

#include "usb_config.h"
#include <cstdint>
#include <array>

class usb_strings {
public:
    // This class is a simple singleton
    static usb_strings inst;

    // No copy, no assignment
    usb_strings (const usb_strings &) = delete;
    usb_strings & operator= (const usb_strings &) = delete;

    // Add a new string entry and return the index where the
    // pointer was stored. Index 0 is the language id (see
    // CTOR). The pointer has to point to a non-changing
    // C-string in memory, e.g. a (static) string literal.
    uint8_t add_string(const char * str);

    // Set up a USB buffer so that it conforms to a USB
    // string descriptor. The string to process is selected
    // by the index parameter. buffer has to point to an
    // allocated buffer, which has to have a minimum size
    // of strlen(string)+2. Two bytes are needed for
    // the descriptor length and type. One byte is needed
    // for every character (the generated string is UTF8).
    // Returns the total length of the constructed descriptor.
    uint8_t prepare_desc_utf8(uint8_t index, uint8_t * buffer);

    // Set up a USB buffer so that it conforms to a USB
    // string descriptor. The string to process is selected
    // by the index parameter. buffer has to point to an
    // allocated buffer, which has to have a minimum size
    // of strlen(string)*2+2. Two bytes are needed for
    // the descriptor length and type. Two bytes are needed
    // for every character (the generated string is UTF16).
    // Returns the total length of the constructed descriptor.
    uint8_t prepare_desc_utf16(uint8_t index, uint8_t * buffer);

    // Convert a given C-string into a UTF16 string including a
    // (double) NULL-termination character. The generated string
    // is stored in the given buffer. Return the size of the
    // generated UTF16 string.
    uint16_t convert_to_utf16(const char * str, uint8_t * buffer);

private:
    // Standard CTOR. It adds a default entry (index 0)
    // with the language descriptor (US english).
    usb_strings();

    // Array to store pointers to the (static) strings
    std::array<const char *, TUPP_MAX_STRINGS> _strings;
};

#endif // TUPP_USB_STRINGS_H
