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
// This class represents a union functional descriptor
//
#ifndef TUPP_USB_FD_UNION_H
#define TUPP_USB_FD_UNION_H

#include "usb_cdc_structs.h"
#include "usb_fd_base.h"
#include "usb_log.h"

class usb_fd_union : public usb_fd_base {
public:
    explicit usb_fd_union(usb_interface & i);

    // No copy, no assignment
    usb_fd_union (const usb_fd_union &) = delete;
    usb_fd_union & operator= (const usb_fd_union &) = delete;

    inline void set_bControlInterface(uint8_t val) {
        TUPP_LOG(LOG_DEBUG, "set_bControlInterface(%d)", val);
        _descriptor.bControlInterface = val;
    }
    inline void set_bSubordinateInterface0(uint8_t val) {
        TUPP_LOG(LOG_DEBUG, "set_bSubordinateInterface0(%d)", val);
        _descriptor.bSubordinateInterface[0] = val;
    }

private:
    USB::CDC::func_desc_union_t<1> _descriptor {};
};

#endif  // TUPP_USB_FD_UNION_H
