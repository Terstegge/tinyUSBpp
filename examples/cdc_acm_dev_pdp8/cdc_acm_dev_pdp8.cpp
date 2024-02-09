// This is a simulator for a PDP-8, a minicomputer
// build by Digital Equipment Corporation in the
// 1960s. It was a 12-bit computer with 4k-words
// core memory (every bit was stored in a small
// magnetic core) and a compact instruction set.
// The core memory is preloaded with FOCAL-69,
// a simple interpreter language. A manual of this
// language can be found in the doc folder.
//
// Use a terminal on USB ACM device to program
// a PDP-8 in FOCAL-69 :)

#include "Memory.h"
#include "CPU.h"
#include "Devices.h"
#include "Device_TTI.h"
#include "Device_TTO.h"

#include "pico/stdlib.h"
#include "usb_dcd_rp2040.h"
#include "usb_device_controller.h"
#include "usb_ms_webusb_descriptor.h"
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
    device.set_idVendor       (0x04A0);
    device.set_idProduct      (0x008E);
    device.set_Manufacturer   ("Digital Equipment Corp.");
    device.set_Product        ("PDP8 Demo");

    // USB BOS descriptor
    usb_ms_webusb_descriptor webusb(controller, device);

    // USB configuration descriptor
    usb_configuration config(device);
    config.set_bConfigurationValue(1);
    config.set_bmAttributes( { .remote_wakeup = 0,
                               .self_powered  = 0,
                               .bus_powered   = 1 } );
    config.set_bMaxPower_mA(100);

    // USB CDC ACM device
    usb_cdc_acm_device acm_device(controller, config);

    // Activate USB device
    driver.pullup_enable(true);
    while (!controller.active_configuration) ;

    // Wait for a keypress and welcome
    while (!acm_device.available()) sleep_ms(100);
    const uint8_t message[] = "\r\n\r\n***** DEC PDP-8 Simulator *****\r\n\r\n";
    acm_device.write(message, sizeof(message));

    // Add some PDP devices (Teletype input/output)
    Devices devices;
    devices.addDevice(new Device_TTI(acm_device)); // Add a Keyboard Device
    devices.addDevice(new Device_TTO(acm_device)); // Add a Teletype Device

    Memory m;
    CPU cpu(m, devices);
    cpu.run(); // will not return
}
