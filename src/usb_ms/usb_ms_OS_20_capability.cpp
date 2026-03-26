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
#include <cstring> // for memcpy
#include <cstdio>

#include "usb_ms_OS_20_capability.h"
#include "usb_log.h"
#include "usb_strings.h"
using enum usb_log::log_level;

#define GET_MS_OS20_DESC  ((TUPP::bRequest_t)0x1)

usb_ms_OS_20_capability::usb_ms_OS_20_capability(usb_device_controller & controller,
                                                 usb_device & device)
: descriptor(_descriptor), _controller(controller), _device(device) {

    // Set our capability descriptor
    _descriptor.bLength             = sizeof(TUPP::dev_cap_platform_ms_os_20_t);
    _descriptor.bDescriptorType     = TUPP::bDescriptorType_t::DESC_DEVICE_CAPABILITY;
    _descriptor.bDevCapabilityType  = TUPP::bDevCapabilityType_t::CAP_PLATFORM;
    _descriptor.bReserved           = 0;
    _descriptor.dwWindowsVersion    = WIN_VER_81;
    _descriptor.bMS_VendorCode      = GET_MS_OS20_DESC;
    _descriptor.bAltEnumCode        = 0;
    for(size_t i=0; i < sizeof(_descriptor.PlatformCapabilityUUID); ++i) {
        _descriptor.PlatformCapabilityUUID[i] = UUID[i];
    }
    set_total_length();

    // We are the parent of the MS OS 2.0 header
    header.set_dwWindowsVersion(WIN_VER_81);
    header.set_parent(this);

    // Set up USB request handler for this device
    _device.setup_handler = [&] (TUPP::setup_packet_t * pkt) {

//        uint16_t l = prepare_descriptor();
//        printf("\n");
//        for(size_t i=0; i < l; ++i) {
//            printf("%02X ", _buffer[i]);
//            if (((i+1) % 16) == 0) printf("\n");
//        }
//        printf("\n");

        TUPP_LOG(LOG_DEBUG, "device setup_handler()");
        if ( (pkt->bRequest == GET_MS_OS20_DESC) &&
             (pkt->wValue   == 0) && (pkt->wIndex == 7) ) {
            TUPP_LOG(LOG_INFO, "Getting MS OS 2.0 descriptor");
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

uint16_t usb_ms_OS_20_capability::prepare_descriptor() {
    TUPP_LOG(LOG_DEBUG, "prepare_descriptor()");

    // Copy header descriptor
    auto * tmp_ptr = _buffer;
    memcpy(tmp_ptr, header.get_descriptor(),
           header.get_descriptor_length());
    tmp_ptr += header.get_descriptor_length();

    // Copy all features below header
    for (auto & feature : header._features) {
        if (feature) {
            memcpy(tmp_ptr, feature->get_descriptor(),
                   feature->get_descriptor_length());
            tmp_ptr += feature->get_descriptor_length();
        }
    }

    // Copy all config subsets below header
    for (auto & config_subset : header._config_subsets) {
        if (config_subset) {
            memcpy(tmp_ptr, config_subset->get_descriptor(),
                   config_subset->get_descriptor_length());
            tmp_ptr += config_subset->get_descriptor_length();

            // Copy all features below a config subset
            for (auto & feature : config_subset->_features) {
                if (feature) {
                    memcpy(tmp_ptr, feature->get_descriptor(),
                           feature->get_descriptor_length());
                    tmp_ptr += feature->get_descriptor_length();
                }
            }

            // Copy all functional subsets below a config subset
            for (auto func_subset : config_subset->_func_subsets) {
                if (func_subset) {
                    memcpy(tmp_ptr, func_subset->get_descriptor(),
                           func_subset->get_descriptor_length());
                    tmp_ptr += func_subset->get_descriptor_length();

                    // Copy all feature below a functional subset
                    for (auto & feature : func_subset->_features) {
                        if (feature) {
                            memcpy(tmp_ptr, feature->get_descriptor(),
                                   feature->get_descriptor_length());
                            tmp_ptr += feature->get_descriptor_length();
                        }
                    }

                }
            }
        }
    }
    return (tmp_ptr - _buffer);
}
