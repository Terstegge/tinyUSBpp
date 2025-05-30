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
// This class represents a MSC (Mass Storage Class) BOT (Bulk Only Tramsfer)
// device. The user interface are 6 handler functions (see below).
//
#ifndef TUPP_USB_MSC_BOT_DEVICE_H
#define TUPP_USB_MSC_BOT_DEVICE_H

#include "usb_msc_structs.h"
#include "scsi_structs.h"
#include "usb_configuration.h"
#include "usb_interface.h"
#include "usb_endpoint.h"
#include "usb_device_controller.h"
#include "usb_config.h"
#include <array>

class usb_msc_bot_device {
public:
    usb_msc_bot_device(usb_device_controller & controller,
                       usb_configuration     & configuration);

    // Handle next MSC request.
    void handle_request();

    // Callback handler to get the block size and block count of the device.
    // block size should normally be TUPP_MSC_BLOCK_SIZE, other cases are
    // currently not supported.
    std::function<void(uint16_t & block_size, uint32_t & block_count)> capacity_handler;

    // Callback handler to read a single block from the device
    std::function<uint8_t(uint8_t * buff, uint32_t block)> read_handler;

    // Callback handler to write a single block to the device
    std::function<uint8_t (uint8_t * buff, uint32_t block)> write_handler;

    // Callback handler to get the 'writable' state
    std::function<bool()> is_writeable_handler;

    // Callback handler to get the start/stop and eject state
    std::function<void(bool start, bool load_eject)> start_stop_handler;

    // Callback handler for the removable state
    std::function<void(bool prevent_removal)> remove_handler;

    // Setters for IDs in inquiry response
    void set_vendor_id(const char * id);
    void set_product_id(const char * id);
    void set_product_rev(const char * rev);

    // Setter for device status
    void set_device_ready(bool ready);

private:

    enum class state_t : uint8_t {
        RECEIVE_CBW = 0,
        DATA_READ   = 1,
        DATA_WRITE  = 2,
        SEND_CSW    = 3
    };

    // CDC ACM descriptor tree
    usb_configuration &         _configuration;
    usb_interface               _interface {_configuration};

    // USB endpoints
    usb_endpoint *              _ep_in  {nullptr};
    usb_endpoint *              _ep_out {nullptr};

    uint8_t                     _max_lun {0};

    state_t                     _state;
    MSC::csw_t                  _csw {};

    bool                        _device_ready {true};

    // Internal data buffers
    volatile uint16_t           _buffer_out_len {0};
    uint8_t                     _buffer_out[TUPP_MSC_BLOCK_SIZE] {0};
    uint8_t                     _buffer_in [TUPP_MSC_BLOCK_SIZE] {0};

    // Various SCSI response types
    SCSI::inquiry_response_t                    _inquiry_response;
    SCSI::request_sense_fixed_response_t        _sense_fixed_response;
    SCSI::read_capacity_10_response_t           _read_capacity_10_response;
    SCSI::read_format_capacity_10_response_t    _read_format_capacity_10_response;
    SCSI::mode_sense_6_response_t               _mode_sense_6_response;

    void process_scsi_command();

    // Data transfer parameters
    uint16_t                    _blocks_to_transfer {0};
    uint16_t                    _blocks_transferred {0};
    uint32_t                    _block_addr {0};
};

#endif  // TUPP_USB_MSC_BOT_DEVICE_H

