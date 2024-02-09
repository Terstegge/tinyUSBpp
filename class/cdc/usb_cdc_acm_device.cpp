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
#include "usb_cdc_acm_device.h"
#include "usb_common.h"
#include "usb_log.h"
#include <cassert>

using namespace USB;
using enum USB::bInterfaceClass_t;
using enum USB::bInterfaceSubClass_t;
using enum USB::bInterfaceProtocol_t;
using enum USB::ep_attributes_t;
using enum USB::direction_t;

usb_cdc_acm_device::usb_cdc_acm_device(
        usb_device_controller & controller,
        usb_configuration & configuration)
: line_coding(_line_coding), _configuration(configuration)
{
    TUPP_LOG(LOG_DEBUG, "usb_cdc_acm_device() @%x", this);

    // USB interface association descriptor config
    //////////////////////////////////////////////
    _assoc.set_bFunctionClass   (IF_CLASS_CDC);
    _assoc.set_bFunctionSubClass(IF_SUBCLASS_ABSTRACT_CONTROL_MODEL);
    _assoc.set_bFunctionProtocol(IF_PROTOCOL_NONE);

    // USB interface descriptors config
    ///////////////////////////////////
    _if_ctrl.set_bInterfaceClass   (IF_CLASS_CDC);
    _if_ctrl.set_bInterfaceSubClass(IF_SUBCLASS_ABSTRACT_CONTROL_MODEL);
    _if_ctrl.set_bInterfaceProtocol(IF_PROTOCOL_NONE);

    _if_data.set_bInterfaceClass   (IF_CLASS_CDC_DATA);
    _if_data.set_bInterfaceProtocol(IF_PROTOCOL_NONE);

    // USB Functional Descriptors config
    ////////////////////////////////////
    _header_fd.set_bcdCDC(0x0120);

    _call_mgmt_fd.set_bmCapabilities({
        .handle_call_management         = 0,
        .call_management_via_data_class = 0
    });
    _call_mgmt_fd.set_bDataInterface(_if_data.descriptor.bInterfaceNumber);

    _acm_fd.set_bmCapabilities({
        .comm_feature_support           = 0,
        .line_coding_support            = 1,
        .send_break_support             = 1,
        .network_connection_support     = 0
    });

    _union_fd.set_bControlInterface     (_if_ctrl.descriptor.bInterfaceNumber);
    _union_fd.set_bSubordinateInterface0(_if_data.descriptor.bInterfaceNumber);

    // USB endpoints
    ////////////////
    _ep_data_in  = controller.create_endpoint(_if_data, DIR_IN,  TRANS_BULK);
    _ep_data_out = controller.create_endpoint(_if_data, DIR_OUT, TRANS_BULK);
    _ep_ctrl_in  = controller.create_endpoint(_if_ctrl, DIR_IN,  TRANS_INTERRUPT);

    // Prepare new request to receive data
    //////////////////////////////////////
    _ep_data_out->start_transfer(_buffer_out, _ep_data_out->descriptor.wMaxPacketSize);

    // Endpoint handlers
    ////////////////////
    _ep_data_out->data_handler = [&](uint8_t *buf, uint16_t len) {
        // Check if we need to stop incoming data.
        if (_received_data.available_put() < (2 * _ep_data_out->descriptor.wMaxPacketSize)) {
            _ep_data_out->send_NAK(true);
        }
        // Copy all available bytes to the fifo
        for (int i=0; i < len; ++i) {
            bool b = _received_data.put(buf[i]);
            assert(b);
        }
        // Trigger a new receive
        _ep_data_out->start_transfer(_buffer_out, _ep_data_out->descriptor.wMaxPacketSize);
    };

    _ep_data_in->data_handler = [&](uint8_t *, uint16_t len) {
        // Value of len is discarded, use it as a local variable!
        // Try to send as much data as possible up to MaxPacketSize
        for(len = 0; len < _ep_data_in->descriptor.wMaxPacketSize; ++len) {
            if (!_data_to_transmit.get(_buffer_in[len])) {
                break;
            }
        }
        // Trigger a new transmit if we have sent something before
        // (which might indicate there is more data to sent...)
        if (len) {
            _ep_data_in->start_transfer(_buffer_in, len);
        }
    };

    // Handler for CDC ACM specific requests
    ////////////////////////////////////////
    _if_ctrl.setup_handler = [&](USB::setup_packet_t *pkt) {
        switch(pkt->bRequest) {
            case bRequest_t::REQ_CDC_SET_LINE_CODING: {
                TUPP_LOG(LOG_INFO, "Handling REQ_CDC_SET_LINE_CODING");
                assert(pkt->wLength == sizeof(_line_coding) );
                // Set the data handler
                controller.handler = [&] (const uint8_t * data, uint16_t len) {
                    assert(data == (uint8_t *)&line_coding);
                    assert(len  == sizeof(CDC::line_coding_t));
                    if (line_coding_handler) {
                        line_coding_handler(line_coding);
                    }
                };
                // Receive line coding info
                controller._ep0_out->start_transfer((uint8_t *)&_line_coding, sizeof(_line_coding));
                break;
            }
            case bRequest_t::REQ_CDC_GET_LINE_CODING: {
                TUPP_LOG(LOG_INFO, "Handling REQ_CDC_GET_LINE_CODING");
                assert(pkt->wLength == sizeof(_line_coding) );
                controller._ep0_in->start_transfer((uint8_t *)&_line_coding, sizeof(_line_coding));
                break;
            }
            case bRequest_t::REQ_CDC_SET_CONTROL_LINE_STATE: {
                TUPP_LOG(LOG_INFO, "Handling REQ_CDC_SET_CONTROL_LINE_STATE");
                // Call user handler (should be short)
                if (control_line_handler) {
                    control_line_handler(pkt->wValue & 0x0001,
                                         pkt->wValue & 0x0002);
                }
                // Status stage
                controller._ep0_in->send_zlp_data1();
                break;
            }
            case bRequest_t::REQ_CDC_SEND_BREAK: {
                TUPP_LOG(LOG_INFO, "Handling REQ_CDC_SEND_BREAK");
                // Call user handler (should be short)
                if (break_handler) {
                    break_handler(pkt->wValue);
                }
                // Status stage
                controller._ep0_in->send_zlp_data1();
                break;
            }
            default: {
                TUPP_LOG(LOG_ERROR, "Unsupported CDC command 0x%x", pkt->bRequest);
                controller._ep0_in->send_stall(true);
                controller._ep0_out->send_stall(true);
            }
        }
    };
}

uint16_t usb_cdc_acm_device::read(uint8_t *buf, uint16_t max_len) {
    uint8_t  val = 0;
    uint16_t len;
    for (len=0; len < max_len; ++len) {
        if (_received_data.get(val)) {
            buf[len] = val;
        } else break;
    }
    // Check if we can receive more data
    if (_received_data.available_get() < (2 * _ep_data_out->descriptor.wMaxPacketSize)) {
        _ep_data_out->send_NAK(false);
    }
    return len;
}

uint16_t usb_cdc_acm_device::available() {
    return _received_data.available_get();
}

bool usb_cdc_acm_device::write(const uint8_t *buf, uint16_t len) {
    if (_data_to_transmit.available_put() < len) {
        return false;
    }
    for(int i=0; i < len; ++i) {
        _data_to_transmit.put(buf[i]);
    }
    // Check if we need a new initial transfer
    // if the endpoint is currently not active
    if (!_ep_data_in->is_active()) {
        // Parameters are discarded. The in data handler
        // looks at the FIFO to decide whether to send data!
        _ep_data_in->data_handler(nullptr, 0);
    }
    return true;
}

bool usb_cdc_acm_device::notify_serial_state(const USB::CDC::bmUartState_t & state) {
    TUPP_LOG(LOG_DEBUG, "notify_serial_state()");
    if (_ep_ctrl_in->is_active()) {
        return false;
    }
    // Create serial state object and set attributes
    CDC::notif_serial_state_t serial_state;
    serial_state.wIndex      = _if_ctrl.descriptor.bInterfaceNumber;
    serial_state.bmUartState = state;
    // Send notification to host
    _ep_ctrl_in->start_transfer((uint8_t *)&serial_state, sizeof(serial_state));
    return true;
}
