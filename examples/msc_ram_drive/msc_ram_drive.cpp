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
// This example provides a 200kB RAM-Drive as a USB MSC device.
//
#include <cstring>

#include "pico/stdlib.h"

#include "usb_bos.h"
#include "usb_dcd.h"
#include "usb_device.h"
#include "usb_device_controller.h"
#include "usb_cdc_acm_device.h"
#include "usb_msc_bot_device.h"

#include "usb_ms_OS_20_capability.h"
#include "usb_ms_func_subset.h"
#include "usb_ms_compatible_ID.h"

#include "image.h"

// Parameters of RAM disk
const uint16_t BLOCK_SIZE  = 512;
const uint32_t BLOCK_COUNT = 400;

int main() {
    // USB Device driver
    usb_dcd & driver = usb_dcd::inst();
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

    // USB configuration descriptor
    usb_configuration config(device);
    config.set_bConfigurationValue(1);
    config.set_bmAttributes( { .remote_wakeup = 0,
                               .self_powered  = 0,
                               .bus_powered   = 1 } );
    config.set_bMaxPower_mA(100);

    // USB CDC ACM device
    usb_cdc_acm_device acm_device(controller, config);

    // MSC device
    usb_msc_bot_device msc_device(controller, config);
    msc_device.set_vendor_id  ("Vendor");
    msc_device.set_product_id ("MSC Test Device");
    msc_device.set_product_rev("1.0");

    // Add MS OS 2.0 descriptor
    ///////////////////////////
    usb_bos bos(controller, device); // Add a Binary Object Store
    usb_ms_OS_20_capability ms_os20(bos);
    bos.add_capability(ms_os20);

    // This functional subset is only valid for a
    // specific interface number, here the CDC Data interface
    usb_ms_func_subset ms_func_subset;
    ms_func_subset.set_bFirstInterface(acm_device.get_data_if_number());
    ms_os20.header.add(ms_func_subset);

    // CDC will use the WINUSB driver in Windows
    usb_ms_compatible_ID compat_id;
    compat_id.set_compatible_id( "WINUSB" );
    ms_func_subset.add(compat_id);

    // Initialize the LED
    gpio_init   (PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // RAM area for our drive
    uint8_t ram_drive[BLOCK_COUNT][BLOCK_SIZE] {0};
    memcpy(ram_drive, image, sizeof image);

    // Set callback handlers
    msc_device.capacity_handler = [&](uint16_t & block_size,
                                      uint32_t & block_count) {
        block_size  = BLOCK_SIZE;
        block_count = BLOCK_COUNT;
    };
    msc_device.read_handler = [&](uint8_t * buff, uint32_t block) {
        memcpy(buff, ram_drive[block], BLOCK_SIZE);
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        return 0;
    };
    msc_device.write_handler = [&](uint8_t * buff, uint32_t block) {
        memcpy(ram_drive[block], buff, BLOCK_SIZE);
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        return 0;
    };

    // Activate USB device
    driver.pullup_enable(true);
    while (!controller.active_configuration) sleep_ms(20);

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
            gpio_put(PICO_DEFAULT_LED_PIN, false);
        }
        // Handle MSC requests
        msc_device.handle_request();
    }
}
