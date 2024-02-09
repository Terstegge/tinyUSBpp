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
// (c) 2023 A. Terstegge  (Andreas.Terstegge@gmail.com)
//
#include "usb_ms_webusb_descriptor.h"
#include "usb_log.h"
#include <cstring>

#define GET_WEBUSB_DESC ((USB::bRequest_t)0x1)
#define GET_WEBUSB_URL  ((USB::bRequest_t)0x2)
#define URL_HTTP        "\x00"
#define URL_HTTPS       "\x01"
#define URL_FULL        "\xff"

usb_ms_webusb_descriptor::usb_ms_webusb_descriptor(usb_device_controller & ctrl,
                                                   usb_device & dev)
: _controller(ctrl), _device(dev), _bos(_device)
{
    // {3408b638-09a9-47a0-8bfd-a0768815b665}
//    uint8_t uuid1[16] = { 0x38, 0xB6, 0x08, 0x34, 0xA9, 0x09, 0xA0, 0x47,
//                          0x8B, 0xFD, 0xA0, 0x76, 0x88, 0x15, 0xB6, 0x65 };

    // {d8dd60df-4589-4cc7-9cd2-659d9e648a9f}
    uint8_t uuid2[16] = { 0xDF, 0x60, 0xDD, 0xD8, 0x89, 0x45, 0xC7, 0x4C,
                          0x9C, 0xD2, 0x65, 0x9D, 0x9E, 0x64, 0x8A, 0x9F };

    uint32_t win_version = 0x06030000;

//    _web_platform.set_PlatformCapabilityUUID ( uuid1 );
//    _web_platform.set_bcdVersion             ( 0x0100 );
//    _web_platform.set_bVendorCode            ( GET_WEBUSB_URL );
//    _web_platform.set_iLandingPage           ( URL_FULL "http://www.fh-aachen.de");

    _cap_platform.set_PlatformCapabilityUUID ( uuid2 );
    _cap_platform.set_dwWindowsVersion       ( win_version );
    _cap_platform.set_bMS_VendorCode         ( GET_WEBUSB_DESC );
    _cap_platform.set_bAltEnumCode           ( 0 );

    _ms_header.set_dwWindowsVersion          ( win_version );

    _ms_compat_id.set_compatible_id          ( "WINUSB" );
    _ms_compat_id.set_sub_compatible_id      ( "" );

    _ms_reg_prop.add_property_name           ( "DeviceInterfaceGUIDs\0" );
    _ms_reg_prop.add_property_value          ( "{CDB3B5AD-293B-4663-AA36-1AAE46463776}" );

    _device.setup_handler = [&] (USB::setup_packet_t * pkt) {
        TUPP_LOG(LOG_DEBUG, "device setup_handler()");
        if ( (pkt->bRequest == GET_WEBUSB_URL) &&
             (pkt->wValue   == 1) && (pkt->wIndex == 2) ) {
            TUPP_LOG(LOG_INFO, "Getting MS WebUSB URL");
            uint8_t len = usb_strings::inst.prepare_desc_utf8(pkt->wValue, _buffer);
            if (len > pkt->wLength) len = pkt->wLength;
            _controller._ep0_in->start_transfer(_buffer, len);
        } else if ( (pkt->bRequest == GET_WEBUSB_DESC) &&
                    (pkt->wValue   == 0) && (pkt->wIndex == 7) ) {
            TUPP_LOG(LOG_INFO, "Getting MS WebUSB descriptor");
            uint16_t len = prepare_descriptor();
            _controller._ep0_in->start_transfer(_buffer, len);
        } else {
            TUPP_LOG(LOG_ERROR, "Unknown device request 0x%x", pkt->bRequest);
            _controller._ep0_in->send_stall(true);
            _controller._ep0_out->send_stall(true);
        }
    };

//    printf("Header len %04x\n", ms_header.descriptor.wLength);
//    printf("Header ttl %04x\n", ms_header.descriptor.wTotalLength);
//    printf("Config len %04x\n", ms_config_subset.descriptor.wLength);
//    printf("Config ttl %04x\n", ms_config_subset.descriptor.wTotalLength);
//    printf("Func   len %04x\n", ms_func_subset.descriptor.wLength);
//    printf("Func   sub %04x\n", ms_func_subset.descriptor.wSubsetLength);
//    printf("Compat len %04x\n", ms_compat_id.descriptor.wLength);
//    printf("Prop   len %04x\n", ms_reg_prop.get_length());

}

uint16_t usb_ms_webusb_descriptor::prepare_descriptor() {
    auto * tmp_ptr  = _buffer;
    auto * ptr1     = &_ms_header.descriptor;
    auto   len1     =  _ms_header.descriptor.wLength;
    memcpy(tmp_ptr, ptr1, len1);
    tmp_ptr += len1;
    for (auto & _config_subset : _ms_header._config_subsets) {
        auto * ptr2 = &_config_subset->descriptor;
        auto   len2 =  _config_subset->descriptor.wLength;
        memcpy(tmp_ptr, ptr2, len2);
        tmp_ptr += len2;
        for (auto & _func_subset : _config_subset->_func_subsets) {
            auto  ptr3 = &_func_subset->descriptor;
            auto  len3  = _func_subset->descriptor.wLength;
            memcpy(tmp_ptr, ptr3, len3);
            tmp_ptr += len3;
            if (_func_subset->_compat_id) {
                auto ptr4 = &_func_subset->_compat_id->descriptor;
                auto len4 =  _func_subset->_compat_id->descriptor.wLength;
                memcpy(tmp_ptr, ptr4, len4);
                tmp_ptr += len4;
            }
            for (auto & _reg_prop : _func_subset->_reg_props) {
                auto ptr5 = _reg_prop->descriptor();
                auto len5 = _reg_prop->get_length();
                memcpy(tmp_ptr, ptr5, len5);
                tmp_ptr += len5;
            }
        }
    }
    return (tmp_ptr - _buffer);
}