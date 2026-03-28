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
#include "usb_ms_compat_descriptor.h"
#include "usb_strings.h"
#include "usb_log.h"
using enum usb_log::log_level;

#define GET_WEBUSB_URL  ((TUPP::bRequest_t)0x1)
#define GET_WEBUSB_DESC ((TUPP::bRequest_t)0x2)

usb_ms_compat_descriptor::usb_ms_compat_descriptor(usb_device_controller & ctrl,
                                                   usb_device & dev,
                                                   uint8_t configuration_value,
                                                   uint8_t first_interface,
                                                   const char * URL)
: _controller(ctrl), _device(dev), _bos(_device)
{
    TUPP_LOG(LOG_DEBUG, "usb_ms_compat_descriptor() @%x", this);
    (void)URL;
#if 0
// {3408b638-09a9-47a0-8bfd-a0768815b665}
    uint8_t uuid1[16] = { 0x38, 0xB6, 0x08, 0x34, 0xA9, 0x09, 0xA0, 0x47,
                          0x8B, 0xFD, 0xA0, 0x76, 0x88, 0x15, 0xB6, 0x65 };
     _web_platform.set_PlatformCapabilityUUID ( uuid1 );
    _web_platform.set_bcdVersion             ( 0x0100 );
    _web_platform.set_bVendorCode            ( GET_WEBUSB_URL );
    if (URL) {
        _web_platform.set_iLandingPage       ( URL );
    }
#endif

    // Platform capability UUID {d8dd60df-4589-4cc7-9cd2-659d9e648a9f}
    // Indicates that this device supports the MS OS 2.0 descriptor
    const uint8_t uuid2[16] = { 0xDF, 0x60, 0xDD, 0xD8, 0x89, 0x45, 0xC7, 0x4C,
                                0x9C, 0xD2, 0x65, 0x9D, 0x9E, 0x64, 0x8A, 0x9F };

    // Windows version 8.1 and newer
    uint32_t win_version = 0x06030000;

    // Set up MS OS 2.0 platform capability descriptor
    // and add it to our Binary Object Storage (BOS)
    //////////////////////////////////////////////////
    _cap_platform.set_PlatformCapabilityUUID ( uuid2 );
    _cap_platform.set_dwWindowsVersion       ( win_version );
    _cap_platform.set_bMS_VendorCode         ( GET_WEBUSB_DESC );
    _cap_platform.set_bAltEnumCode           ( 0 );
    _bos.add_capability(_cap_platform);

    // Set up MS OS 2.0 descriptor set header,
    // specifying the relevant Windows version
    //////////////////////////////////////////
    _ms_header.set_dwWindowsVersion( win_version );
    _cap_platform.add_ms_header    ( _ms_header  );

    // Set up MS OS 2.0 configuration subset descriptor
    // specify the configuration value for this descriptor
    //////////////////////////////////////////////////////
    _ms_config_subset.set_bConfigurationValue( configuration_value );
    _ms_header.add( _ms_config_subset );

    // Set up MS OS 2.0 functional subset header, and add a
    // compatibility ID and a registry property
    ///////////////////////////////////////////////////////
    _ms_func_subset.set_bFirstInterface ( first_interface );
    _ms_config_subset.add( _ms_func_subset );

    // Add feature compatibility ID
    ///////////////////////////////
    _ms_compat_id.set_compatible_id     ( "WINUSB" );
    _ms_compat_id.set_sub_compatible_id ( "" );
    _ms_func_subset.add( _ms_compat_id );

    // Add feature registry property
    ////////////////////////////////
    _ms_func_subset.add     ( _ms_reg_prop );
    _ms_reg_prop.add_string ( "DeviceInterfaceGUIDs" );
    _ms_reg_prop.add_string ( "{CDB3B5AD-293B-4663-AA36-1AAE46463776}" );
    _ms_reg_prop.add_end_marker();


    _device.setup_handler = [&] (TUPP::setup_packet_t * pkt) {

//        uint16_t len = prepare_descriptor();
//        printf("\n");
//        for(size_t i=0; i < len; ++i) {
//            printf("%02X ", _buffer[i]);
//            if (((i+1) % 16) == 0) printf("\n");
//        }
//        printf("\n");

        TUPP_LOG(LOG_DEBUG, "device setup_handler()");
        if ( (pkt->bRequest == GET_WEBUSB_URL) && (pkt->wIndex == 2) ) {
                TUPP_LOG(LOG_INFO, "Getting MS WebUSB URL");
                // Make sure we have a landing page (string index > 0)!
                if (pkt->wValue) {
                uint8_t len = usb_strings::inst.prepare_string_desc_utf8(pkt->wValue, _buffer);
                if (len > pkt->wLength) len = pkt->wLength;
                _controller._ep0_in->start_transfer(_buffer, len);
            } else {
                _controller._ep0_in->send_stall(true);
                _controller._ep0_out->send_stall(true);
            }
        } else if ( (pkt->bRequest == GET_WEBUSB_DESC) &&
                    (pkt->wValue   == 0) && (pkt->wIndex == 7) ) {
            TUPP_LOG(LOG_INFO, "Getting MS WebUSB descriptor");
            uint16_t len = prepare_descriptor();
            _controller._ep0_in->start_transfer(_buffer, len);
        } else {
            TUPP_LOG(LOG_ERROR, "Unknown device request %d %d %d",
                     pkt->bRequest, pkt->wValue, pkt->wIndex);
            _controller._ep0_in->send_stall(true);
            _controller._ep0_out->send_stall(true);
        }
    };
}

uint16_t usb_ms_compat_descriptor::prepare_descriptor() {
    TUPP_LOG(LOG_DEBUG, "prepare_descriptor()");
    _ms_header.desc_begin();
    uint8_t * _buffer_ptr = _buffer;
    size_t s = _ms_header.desc_total_size();
    for (size_t i=0; i<s; ++i) {
        *_buffer_ptr++ = _ms_header.desc_getNext();
    }
    return s;
}
