//
// Created by andreas on 07.07.26.
//
#include "usb_host_controller.h"
#include "usb_host_controller_states.h"
#include "usb_log.h"

void host_disconnected::enter() {
    TUPP_LOG(LOG_DEBUG, "Enter host_disconnect");
}

void host_disconnected::leave() {
    TUPP_LOG(LOG_DEBUG, "Leave host_disconnect");
    _context->_driver.port_reset(100);
}

void host_disconnected::process_event(host_events e) {
    if (e == DEVICE_CONNECTED) {
        _context->set_state(&_context->_host_get8);
    }
}



void host_get8::enter() {
    TUPP_LOG(LOG_DEBUG, "host_get8: requesting 8-byte device descriptor");
    TUPP::setup_packet_t req {};
    req.direction = TUPP::direction_t::DIR_IN;
    req.type      = TUPP::type_t::TYPE_STANDARD;
    req.recipient = TUPP::recipient_t::REC_DEVICE;
    req.bRequest  = TUPP::bRequest_t::REQ_GET_DESCRIPTOR;
    req.wValue    = (uint16_t)(TUPP::bDescriptorType_t::DESC_DEVICE) << 8;
    req.wIndex    = 0;
    req.wLength   = 8;

    _context->_driver.control_xfer(0, req, _buffer,
        [&](hcd_xfer_result_t result) {
            TUPP_LOG(LOG_INFO, "In callback from control_xfer() !!!");
            host_events e = result.success ? TRANSFER_COMPLETE : DEVICE_DISCONNECTED;
            if (_context->_event_queue.available_put()) {
                _context->_event_queue.put(e);
            }
        });
}

void host_get8::process_event(host_events e) {
    if (e == TRANSFER_COMPLETE) {
        TUPP_LOG(LOG_INFO, "host_get8: GET_DESCRIPTOR(8) complete, "
                 "bLength=%d bMaxPacketSize0=%d",
                 _buffer[0], _buffer[7]);
        // TODO: transition to next enumeration state (SET_ADDRESS, etc.)
    } else if (e == DEVICE_DISCONNECTED) {
        TUPP_LOG(LOG_WARNING, "host_get8: device disconnected during transfer");
        _context->set_state(&_context->_host_disconnected);
    }
}

