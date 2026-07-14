#ifndef TUPP_USB_FD_HID_H
#define TUPP_USB_FD_HID_H

#include "usb_fd_base.h"
#include "usb_hid_structs.h"
#include "usb_hid_defines.h"
#include "usb_interface.h"

class usb_fd_hid : public usb_fd_base {
public:
    explicit usb_fd_hid(usb_interface & i, uint16_t report_desc_len)
    : usb_fd_base(i, (uint8_t *)&_descriptor, sizeof(HID::hid_descriptor_t))
    {
        _descriptor.bLength           = sizeof(HID::hid_descriptor_t);
        _descriptor.bDescriptorType   = HID_DESC_TYPE_HID;
        _descriptor.bcdHID            = 0x0111;
        _descriptor.bCountryCode      = HID_COUNTRY_NONE;
        _descriptor.bNumDescriptors   = 1;
        _descriptor.bDescriptorType2  = HID_DESC_TYPE_REPORT;
        _descriptor.wDescriptorLength = report_desc_len;

        // Add this functional descriptor to the parent interface
        _parent.add_func_descriptor(this);
    }

    usb_fd_hid (const usb_fd_hid &) = delete;
    usb_fd_hid & operator= (const usb_fd_hid &) = delete;

private:
    HID::hid_descriptor_t _descriptor {};
};

#endif // TUPP_USB_FD_HID_H
