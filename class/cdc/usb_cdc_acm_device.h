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
// This class implements a CDC ACM device, which will be
// recognized as a serial port (COM[x]: on Windows,
// /dev/ttyACM[x] on Linux. The user interface are simple
// read and write methods, plus some callback handlers
// (see below).
//
#ifndef TUPP_USB_CDC_ACM_DEVICE_H
#define TUPP_USB_CDC_ACM_DEVICE_H

#include "usb_cdc_common.h"
#include "usb_configuration.h"
#include "usb_device_controller.h"

#include "usb_interface.h"
#include "usb_interface_association.h"

#include "usb_fd_header.h"
#include "usb_fd_call_mgmt.h"
#include "usb_fd_acm.h"
#include "usb_fd_union.h"
#include "usb_fifo.h"
#include <functional>
#include <utility>

class usb_cdc_acm_device {
public:
    usb_cdc_acm_device(usb_device_controller & controller,
                       usb_configuration & configuration);

    // Reading data from this device. Parameters are the
    // buffer and the maximum size of bytes to be read.
    // The user has to make sure that the provided buffer
    // is large enough.
    // Return value is the actual amount of bytes read.
    uint16_t read(uint8_t *buf, uint16_t max_len);

    // Return the number of available characters to read
    uint16_t available();

    // Write data to this device. The buffer and its size
    // are passed as parameters. The return value signals
    // if the writing was successful (true). Either all
    // or no data is written!
    bool write(const uint8_t *buf, uint16_t len);

    // Send a serial state notification to this device.
    bool notify_serial_state(const USB::CDC::bmUartState_t & state);

    // Callback handlers
    std::function<void(const CDC::line_coding_t & lc)>  line_coding_handler;
    std::function<void(bool dtr, bool rts)>             control_line_handler;
    std::function<void(uint16_t millis)>                break_handler;

    // Read only version of line coding information
    const CDC::line_coding_t & line_coding;

private:
    // CDC ACM descriptor tree
    usb_configuration &         _configuration;
    usb_interface_association   _assoc       {_configuration};
    usb_interface               _if_ctrl     {_assoc};
    usb_interface               _if_data     {_assoc};
    usb_fd_header               _header_fd   {_if_ctrl};
    usb_fd_call_mgmt            _call_mgmt_fd{_if_ctrl};
    usb_fd_acm                  _acm_fd      {_if_ctrl};
    usb_fd_union                _union_fd    {_if_ctrl};

    // USB endpoints
    usb_endpoint *              _ep_ctrl_in  {nullptr};
    usb_endpoint *              _ep_data_in  {nullptr};
    usb_endpoint *              _ep_data_out {nullptr};

    // Line coding information
    CDC::line_coding_t          _line_coding;

    // Fifos for received data and data to be transmitted.
    fifo<uint8_t, TUPP_CDC_ACM_FIFO_SIZE> _received_data;
    fifo<uint8_t, TUPP_CDC_ACM_FIFO_SIZE> _data_to_transmit;

    // Internal data buffers
    uint8_t _buffer_out[TUPP_DEFAULT_PAKET_SIZE] {0};
    uint8_t _buffer_in [TUPP_DEFAULT_PAKET_SIZE] {0};
};

#endif  // TUPP_USB_CDC_ACM_DEVICE_H
