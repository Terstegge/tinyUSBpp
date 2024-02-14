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
// Simple CDC ACM example. The serial device will
// simply echo every character. This can e.g. be
// used for data throughput measurements.
//
#include <cctype>
#include "pico/stdlib.h"

#include "usb_dcd_rp2040.h"
#include "usb_device_controller.h"
#include "usb_ms_compat_descriptor.h"
#include "usb_cdc_acm_device.h"

int main() {
    // USB Device driver
    usb_dcd_rp2040 & driver = usb_dcd_rp2040::inst();
    // USB device: Root object of USB descriptor tree
    usb_device device;
    // Put generic USB Device Controller on top
    usb_device_controller controller(driver, device);

    // USB device descriptor
    device.set_bcdUSB         (0x0210);
    device.set_bMaxPacketSize0(64);
    device.set_idVendor       (0x0001);
    device.set_idProduct      (0x0002);
    device.set_Manufacturer   ("Dummy Manufacturer");
    device.set_Product        ("TinyUSB++ ACM Demo");

    // USB BOS descriptor
    usb_ms_compat_descriptor ms_compat(controller, device);

    // USB configuration descriptor
    usb_configuration config(device);
    config.set_bConfigurationValue(1);
    config.set_bmAttributes( { .remote_wakeup = 0,
                               .self_powered  = 0,
                               .bus_powered   = 1 } );
    config.set_bMaxPower_mA(100);

    // USB CDC ACM device
    usb_cdc_acm_device acm_device(controller, config);

    bool line_code_updated = false;

    acm_device.line_coding_handler = ([&](const CDC::line_coding_t &) {
        line_code_updated = true;
    });

    acm_device.control_line_handler = ([&](bool dtr, bool rts) {
    });

    acm_device.break_handler = ([](uint16_t millis) {
    });

    // Activate USB device
    driver.pullup_enable(true);
    while (!controller.active_configuration) ;

    uint8_t buff[256];
    while(1) {
        uint16_t len = acm_device.read(buff, 256);
        if (len) {
            uint32_t written = 0;
            while(written != len) {
                written += acm_device.write(buff+written, len-written);
            }
        }
    }
}
