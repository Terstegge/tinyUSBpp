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
// This class is the central USB device controller. Its main
// job is to set up EP0 and listen to USB request. The standard
// requests are handled within this class, the device/interface/
// endpoint-specific requests will be forwarded to the correct
// destination.

#include "usb_device_controller.h"

#include "usb_structs.h"
#include "usb_device.h"
#include "usb_configuration.h"
#include "usb_interface.h"
#include "usb_interface_association.h"
#include "usb_bos.h"
#include "usb_fd_base.h"
#include "usb_strings.h"
#include "usb_log.h"

#include <cstring>
#include <cassert>

using namespace TUPP;
using enum TUPP::bRequest_t;
using enum TUPP::bDescriptorType_t;
using enum TUPP::recipient_t;
using enum TUPP::ep_attributes_t;
using enum TUPP::direction_t;

usb_device_controller::usb_device_controller(usb_dcd_interface & driver, usb_device & device)
    : active_configuration(_active_configuration), _driver(driver), _device(device)
{
    TUPP_LOG(LOG_DEBUG, "usb_device_controller() @%x", this);
    // Create standard endpoints with address 0
    _ep0_in  = _driver.create_endpoint(0x80, TRANS_CONTROL);
    _ep0_out = _driver.create_endpoint(0x00, TRANS_CONTROL);
    // The data handlers for EP0
    _ep0_in->data_handler = [&](uint8_t *, uint16_t len) {
        // Prepare to receive status stage from host
        if (len) _ep0_out->send_zlp_data1();
    };
    _ep0_out->data_handler = [&](uint8_t * data, uint16_t len) {
        // Reply to received data with zero-length packet
        if (len) _ep0_in->send_zlp_data1();
        // Call data handler. Remember we are in an IRQ context here,
        // and the handler should be as short as possible (maybe only
        // setting a flag...)
        if (handler) {
            handler(data, len);
            handler = nullptr;
        }
    };

    // Handler for USB bus reset
    _driver.bus_reset_handler = [&]() {
        TUPP_LOG(LOG_INFO, "USB Bus Reset");
        // Reset the USB address
        _driver.reset_address();
        // Deactivate configuration, if existing
        if (_active_configuration) {
            auto conf = _device.find_configuration(_active_configuration);
            if (conf) {
                conf->activate_endpoints(false);
            } else{
                TUPP_LOG(LOG_WARNING, "Could not deactivate configuration %d",
                         _active_configuration);
            }
        }
        _active_configuration = 0;
    };

    // Handler for setup requests
    _driver.setup_handler = [&](TUPP::setup_packet_t *pkt) {
        TUPP_LOG(LOG_DEBUG, "setup_handler()");
        // Reset EP0
        _ep0_in->reset();
        _ep0_out->reset();
        // Process standard requests
        if (pkt->type == type_t::TYPE_STANDARD) {
            switch (pkt->bRequest) {
                // Device requests
                case REQ_SET_ADDRESS:
                    handle_set_address(pkt);
                    break;
                case REQ_GET_DESCRIPTOR:
                    handle_get_descriptor(pkt);
                    break;
                case REQ_SET_DESCRIPTOR:
                    handle_set_descriptor(pkt);
                    break;
                case REQ_GET_CONFIGURATION:
                    handle_get_configuration(pkt);
                    break;
                case REQ_SET_CONFIGURATION:
                    handle_set_configuration(pkt);
                    break;
                // Interface requests
                case REQ_GET_INTERFACE:
                    handle_get_interface(pkt);
                    break;
                case REQ_SET_INTERFACE:
                    handle_set_interface(pkt);
                    break;
                    // Endpoint requests
                case REQ_SYNCH_FRAME:
                    handle_synch_frame(pkt);
                    break;
                // Requests for different recipients
                case REQ_GET_STATUS:
                    handle_get_status(pkt);
                    break;
                case REQ_CLEAR_FEATURE:
                    handle_clear_feature(pkt);
                    break;
                case REQ_SET_FEATURE:
                    handle_set_feature(pkt);
                    break;
                default:
                    TUPP_LOG(LOG_WARNING, "Unknown standard setup request type %d", pkt->type);
            }
        } else {
            // We received a non-standard request.
            // Find the proper destination and forward it.
            switch(pkt->recipient) {
                case REC_DEVICE: {
                    if (_device.setup_handler) {
                        _device.setup_handler(pkt);
                    }
                    break;
                }
                case REC_INTERFACE: {
                    auto config = _device.find_configuration(_active_configuration);
                    if (config) {
                        auto interface = config->interfaces[pkt->wIndex];
                        if (interface) {
                            if (interface->setup_handler) {
                                interface->setup_handler(pkt);
                            }
                        }
                    }
                    break;
                }
                case REC_ENDPOINT: {
                    auto ep = _driver.addr_to_ep(pkt->wIndex);
                    if (ep) {
                        if (ep->setup_handler) {
                            ep->setup_handler(pkt);
                        }
                    }
                    break;
                }
                default: {
                    TUPP_LOG(LOG_WARNING, "Could not find recipient %d", pkt->recipient);
                }
            }
        }
    };

    // Start DCD by enabling interrupts
    _driver.irq_enable(true);
}

void usb_device_controller::handle_set_address(setup_packet_t * pkt) {
    TUPP_LOG(LOG_DEBUG, "handle_set_address()");
    assert(pkt->direction == DIR_OUT);
    assert(pkt->recipient == REC_DEVICE);
    assert(pkt->wIndex  == 0);
    assert(pkt->wLength == 0);
    _driver.set_address(pkt->wValue & 0xff);
    // Status stage
    _ep0_in->send_zlp_data1();
}

void usb_device_controller::handle_get_descriptor(setup_packet_t * pkt) {
    TUPP_LOG(LOG_DEBUG, "handle_get_descriptor()");
    assert(pkt->direction == DIR_IN);
    assert(pkt->recipient == REC_DEVICE);
    // Extract type and index
    uint8_t desc_index = pkt->wValue & 0xff;
    auto desc_type = (TUPP::bDescriptorType_t)(pkt->wValue >> 8);
    // Initialize data pointer
    uint8_t * tmp_ptr = _buf;
    switch (desc_type) {
        case DESC_DEVICE: {
            TUPP_LOG(LOG_INFO, "Get device descriptor (len=%d)",
                     pkt->wLength);
            // Sometimes the host wants to get only a fraction of
            // the descriptor (on a MACbook len=8).
            // So no strict check with assert(_device.descriptor.bLength <= pkt->wLength);
            uint16_t len = _device.descriptor.bLength;
            if (pkt->wLength < len) len = pkt->wLength;
            _ep0_in->start_transfer((uint8_t *) &_device.descriptor, len);
            break;
        }
        case DESC_CONFIGURATION: {
            TUPP_LOG(LOG_INFO, "Get configuration descriptor (index %d, len=%d)",
                     desc_index, pkt->wLength);
            auto conf = _device.configurations[desc_index];
            if (conf) {
                assert(pkt->wLength >= sizeof(configuration_descriptor_t));
                // Copy configuration descriptor first
                memcpy(tmp_ptr, &conf->descriptor, sizeof(configuration_descriptor_t));
                tmp_ptr += sizeof(configuration_descriptor_t);
                // Check if we need the big stuff
                if (pkt->wLength >= conf->descriptor.wTotalLength) {
                    // Copy interface and endpoint descriptors
                    for (auto interface : conf->interfaces) {
                        if (interface) {
                            tmp_ptr += interface->prepare_descriptor(
                                tmp_ptr, TUPP_MAX_DESC_SIZE - (tmp_ptr-_buf));
                        }
                    }
                }
                uint16_t len = tmp_ptr - _buf;
            assert(len <= pkt->wLength);
            _ep0_in->start_transfer(_buf, len);
            } else {
                // No configuration, stall the EP0
                _ep0_in->send_stall(true);
                _ep0_out->send_stall(true);
            }
            break;
        }
        case DESC_STRING: {
            TUPP_LOG(LOG_INFO, "Get string descriptor [%d] (len=%d)",
                     pkt->wValue & 0xff, pkt->wLength);
            uint8_t index = pkt->wValue & 0xff;
            uint8_t len = usb_strings::inst.prepare_string_desc_utf16(index, _buf);
            if (len > pkt->wLength) len = pkt->wLength;
            assert(len <= pkt->wLength);
            _ep0_in->start_transfer(_buf, len);
            break;
        }
        case DESC_OTG: {
            TUPP_LOG(LOG_INFO, "Get OTG descriptor (len=%d)", pkt->wLength);
            _ep0_in->send_stall(true);
            _ep0_out->send_stall(true);
            break;
        }
        case DESC_DEBUG: {
            TUPP_LOG(LOG_INFO, "Get Debug descriptor (len=%d)", pkt->wLength);
            _ep0_in->send_stall(true);
            _ep0_out->send_stall(true);
            break;
        }
        case DESC_BOS: {
            TUPP_LOG(LOG_INFO, "Get BOS descriptor (len=%d)", pkt->wLength);
            if (_device.bos) {
                // We have a BOS descriptor
                tmp_ptr += _device.bos->prepare_descriptor(
                           tmp_ptr, TUPP_MAX_DESC_SIZE - (tmp_ptr-_buf));
                uint16_t len = tmp_ptr - _buf;
                if (pkt->wLength < len) len = pkt->wLength;
                _ep0_in->start_transfer(_buf, len);
            } else {
                // No BOS, so stall the EP0
                _ep0_in->send_stall(true);
                _ep0_out->send_stall(true);
            }
            break;
        }
        case DESC_DEVICE_QUALIFIER: {
            TUPP_LOG(LOG_INFO, "Get device qualifier descriptor (len=%d)",
                     pkt->wLength);
            _ep0_in->send_stall(true);
            _ep0_out->send_stall(true);
            break;
        }
        default:
            TUPP_LOG(LOG_WARNING, "Unsupported descriptor type %d", desc_type);
            // Don't know what to report, so stall EP0
            _ep0_in->send_stall(true);
            _ep0_out->send_stall(true);
    }
}

void usb_device_controller::handle_set_descriptor(setup_packet_t * pkt) {
    TUPP_LOG(LOG_INFO, "Set descriptor");
    (void)pkt;
    assert(pkt->direction == DIR_OUT);
    assert(pkt->recipient == REC_DEVICE);
    // Notimplemented so far
    _ep0_in->send_stall(true);
    _ep0_out->send_stall(true);
}

void usb_device_controller::handle_get_configuration(setup_packet_t *pkt) {
    TUPP_LOG(LOG_INFO, "Get configuration (%d)", _active_configuration);
    (void)pkt;
    assert(pkt->direction == DIR_IN);
    assert(pkt->recipient == REC_DEVICE);
    _ep0_in->start_transfer((uint8_t *)(&_active_configuration), 1);
}

void usb_device_controller::handle_set_configuration(setup_packet_t *pkt) {
    TUPP_LOG(LOG_DEBUG, "Set configuration (%d)", pkt->wValue & 0xff);
    assert(pkt->direction == DIR_OUT);
    assert(pkt->recipient == REC_DEVICE);
    uint8_t index = pkt->wValue & 0xff;
    // Check if we change the configuration
    usb_configuration * conf = nullptr;
    if (_active_configuration != index) {
        // De-activate current configuration
        if (_active_configuration) {
            conf = _device.find_configuration(_active_configuration);
            if (conf) {
                conf->activate_endpoints(false);
                TUPP_LOG(LOG_INFO, "Disabled configuration %d", _active_configuration);
            }
            _active_configuration = 0;
        }
        if (index) {
            conf = _device.find_configuration(index);
            if (conf) {
                conf->activate_endpoints(true);
                TUPP_LOG(LOG_INFO, "Enabled configuration %d", index);
            }
            _active_configuration = index;
        }
    }
    // Status stage
    _ep0_in->send_zlp_data1();
}

void usb_device_controller::handle_get_interface(setup_packet_t *pkt) {
    TUPP_LOG(LOG_INFO, "Get interface (%d)", pkt->wIndex & 0xff);
    assert(pkt->direction == DIR_IN);
    assert(pkt->recipient == REC_INTERFACE);
    uint8_t index = pkt->wIndex & 0xff;
    if (_active_configuration) {
        auto interface = _device.configurations[_active_configuration]->interfaces[index];
        if (interface) {
            _ep0_in->start_transfer(&interface->_descriptor.bAlternateSetting, 1);
            return;
        }
    }
    // We did not find the interface, so stall EP0
    _ep0_in->send_stall(true);
    _ep0_out->send_stall(true);
}

void usb_device_controller::handle_set_interface(setup_packet_t *pkt) {
    TUPP_LOG(LOG_INFO, "Set interface");
    assert(pkt->direction == DIR_OUT);
    assert(pkt->recipient == REC_INTERFACE);
    uint8_t index = pkt->wIndex & 0xff;
    if (_active_configuration) {
        auto interface = _device.configurations[_active_configuration]->interfaces[index];
        if (interface) {
            interface->set_bAlternateSetting(pkt->wValue & 0xff);
        }
    }
    // Status stage
    _ep0_in->send_zlp_data1();
}

void usb_device_controller::handle_synch_frame(setup_packet_t *pkt) {
    TUPP_LOG(LOG_INFO, "Synch frame");
    assert(pkt->direction == DIR_IN);
    assert(pkt->recipient == REC_ENDPOINT);
    uint8_t addr = pkt->wIndex & 0xff;
    // Find endpoint and forward request
    auto ep = _driver.addr_to_ep(addr);
    if (ep) {
        if (ep->setup_handler) {
            ep->setup_handler(pkt);
        }
    }
}

void usb_device_controller::handle_get_status(setup_packet_t * pkt) {
    TUPP_LOG(LOG_INFO, "Get status");
    assert(pkt->direction == DIR_IN);
    assert(pkt->wValue  == 0);
    assert(pkt->wLength == 2);
    uint16_t data = 0;
    switch(pkt->recipient) {
        case REC_DEVICE: {
            auto config = _device.find_configuration(_active_configuration);
            if (config) {
                if (config->descriptor.bmAttributes.self_powered) {
                    data |= 1;
                }
                if (config->descriptor.bmAttributes.remote_wakeup) {
                    data |= 2;
                }
            } else {
                TUPP_LOG(LOG_WARNING, "Could not find active configuration %d for GET STATUS",
                         _active_configuration);
            }
            break;
        }
        case REC_INTERFACE: {
            // Status of interface is always 0
            break;
        }
        case REC_ENDPOINT: {
            auto ep = _driver.addr_to_ep(pkt->wIndex);
            if (ep) {
                data = ep->is_stalled() ? 1 : 0;
            } else {
                TUPP_LOG(LOG_WARNING, "Could not find EP 0x%x for GET STATUS", pkt->wIndex);
            }
            break;
        }
        default:
            TUPP_LOG(LOG_WARNING, "Unknown recipient for GET STATUS: %d",
                     pkt->recipient);
    }
    // Send status
    _ep0_in->start_transfer((uint8_t *)&data, 2);
}

void usb_device_controller::handle_clear_feature(setup_packet_t *pkt) {
    assert(pkt->direction == DIR_OUT);
    switch(pkt->recipient) {
        case REC_DEVICE: {
            if (pkt->wValue == 1) {
                TUPP_LOG(LOG_INFO, "Set feature: Remote wakeup off");
                auto config = _device.find_configuration(_active_configuration);
                if (config) config->set_remote_wakeup(false);
            } else {
                TUPP_LOG(LOG_WARNING, "Unknown CLEAR FEATURE id: %d", pkt->wValue);
            }
            break;
        }
        case REC_ENDPOINT: {
            if (pkt->wValue == 0) {
                TUPP_LOG(LOG_INFO, "Set feature: EP 0x%x stall off", pkt->wIndex);
                auto ep = _driver.addr_to_ep(pkt->wIndex);
                if (ep) ep->send_stall(false);
            } else {
                TUPP_LOG(LOG_WARNING, "Unknown CLEAR FEATURE id: %d", pkt->wValue);
            }
            break;
        }
        default:
            TUPP_LOG(LOG_WARNING, "Unknown recipient of CLEAR FEATURE: %d",
                     pkt->recipient);
    }
    // Status stage
    _ep0_in->send_zlp_data1();
}

void usb_device_controller::handle_set_feature(setup_packet_t *pkt) {
    assert(pkt->direction == DIR_OUT);
    switch(pkt->recipient) {
        case REC_DEVICE: {
            if (pkt->wValue == 1) {
                TUPP_LOG(LOG_INFO, "Set feature: Remote wakeup on");
                auto config = _device.find_configuration(_active_configuration);
                if (config) config->set_remote_wakeup(true);
            } else {
                TUPP_LOG(LOG_WARNING, "Unknown SET FEATURE id: %d", pkt->wValue);
            }
            break;
        }
        case REC_ENDPOINT: {
            if (pkt->wValue == 0) {
                TUPP_LOG(LOG_INFO, "Set feature: EP 0x%x stall on", pkt->wIndex);
                auto ep = _driver.addr_to_ep(pkt->wIndex);
                if (ep) ep->send_stall(true);
            } else {
                TUPP_LOG(LOG_WARNING, "Unknown SET FEATURE id: %d", pkt->wValue);
            }
            break;
        }
        default:
            TUPP_LOG(LOG_WARNING, "Unknown recipient for SET FEATURE: %d",
                     pkt->recipient);
    }
    // Status stage
    _ep0_in->send_zlp_data1();
}
