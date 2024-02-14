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
// This example provides a 200kB RAM-Drive as a USB MSC device.
//
#include "pico/stdlib.h"

#include "usb_dcd_rp2040.h"
#include "usb_device_controller.h"
#include "usb_ms_compat_descriptor.h"
#include "usb_cdc_acm_device.h"
#include "usb_msc_bot_device.h"
#include "image.h"

// Parameters of RAM disk
const uint16_t BLOCK_SIZE  = 512;
const uint32_t BLOCK_COUNT = 400;

int main() {
    // Initialize the LED
    gpio_init   (PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

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
    device.set_Product        ("tinyUSB++ MSC Demo");

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

    // RAM area for our drive
    uint8_t ram_drive[BLOCK_COUNT][BLOCK_SIZE] {0};
    memcpy(ram_drive, image, sizeof image);

    // MSC device
    usb_msc_bot_device msc_device(controller, config);
    msc_device.set_vendor_id  ("Vendor");
    msc_device.set_product_id ("MSC Test Device");
    msc_device.set_product_rev("1.0");

    // Set callback handlers
    msc_device.capacity_handler = [&](uint16_t & block_size,
                                      uint32_t & block_count) {
        block_size  = BLOCK_SIZE;
        block_count = BLOCK_COUNT;
    };
    msc_device.read_handler = [&](uint8_t * buff, uint32_t block) {
        memcpy(buff, ram_drive[block], BLOCK_SIZE);
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
    };
    msc_device.write_handler = [&](uint8_t * buff, uint32_t block) {
        memcpy(ram_drive[block], buff, BLOCK_SIZE);
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
    };

    // Activate USB device
    driver.pullup_enable(true);
    while (!controller.active_configuration) ;

    uint8_t  buff[16];
    uint32_t count = 0;
    while(1) {
        // Check if key has been pressed
        if (acm_device.available()) {
            // Read in the characters
            acm_device.read(buff, 16);
            // Output the RAM Drive contents to the ACM device
            uint32_t  len = BLOCK_COUNT * BLOCK_SIZE;
            uint32_t  written = 0;
            uint8_t * src = (uint8_t *)ram_drive;
            while(written != len) {
                written += acm_device.write(src+written, len-written);
            }
        }
        // Handle LED reset to off
        count++;
        if (count > 200000) {
            count = 0;
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
        }
        // Handle MSC requests
        msc_device.handle_request();
    }
}
