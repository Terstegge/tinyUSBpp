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
// This class represents a header functional descriptor
//
#ifndef TUPP_USB_FD_HEADER_H
#define TUPP_USB_FD_HEADER_H

#include "usb_cdc_common.h"
#include "usb_fd_base.h"
#include "usb_log.h"

class usb_fd_header : public usb_fd_base {
public:
    explicit usb_fd_header(usb_interface & i);

    // No copy, no assignment
    usb_fd_header (const usb_fd_header &) = delete;
    usb_fd_header & operator= (const usb_fd_header &) = delete;

    inline void set_bcdCDC(uint16_t val) {
        TUPP_LOG(LOG_DEBUG, "set_bcdCDC(%d)", val);
        _descriptor.bcdCDC = val;
    }

private:
    USB::CDC::func_desc_header_t _descriptor {};
};

#endif  // TUPP_USB_FD_HEADER_H
