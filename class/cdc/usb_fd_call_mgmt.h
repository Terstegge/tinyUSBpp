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
// This class represents a call management functional descriptor
//
#ifndef TUPP_USB_FD_CALL_MGMT_H
#define TUPP_USB_FD_CALL_MGMT_H

#include "usb_cdc_structs.h"
#include "usb_fd_base.h"
#include "usb_log.h"

class usb_fd_call_mgmt : public usb_fd_base {
public:
    explicit usb_fd_call_mgmt(usb_interface & i);

    // No copy, no assignment
    usb_fd_call_mgmt (const usb_fd_call_mgmt &) = delete;
    usb_fd_call_mgmt & operator= (const usb_fd_call_mgmt &) = delete;

    inline void set_bmCapabilities(USB::CDC::bmCmCapabilities_t val) {
        TUPP_LOG(LOG_DEBUG, "set_bmCapabilities(0x%x)", val);
        _descriptor.bmCapabilities = val;
    }
    inline void set_bDataInterface(uint8_t val) {
        TUPP_LOG(LOG_DEBUG, "set_bDataInterface(%d)", val);
        _descriptor.bDataInterface = val;
    }

private:
    USB::CDC::func_desc_cm_t _descriptor {};
};

#endif  // TUPP_USB_FD_CALL_MGMT_H
