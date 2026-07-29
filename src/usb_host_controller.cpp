//
// Created by andreas on 07.07.26.
//

#include "usb_host_controller.h"
#include "semaphore.h"

usb_host_controller::usb_host_controller(usb_hcd_interface &driver) : task{"USB HCD"}, _host_disconnected(this),
                                                                      _host_get8(this),
                                                                      _driver{driver} {
    TUPP_LOG(LOG_DEBUG, "usb_host_controller() @%x", this);
    // Set starting state
    set_state(&_host_disconnected);

    // Enable interrupts in driver
    driver.irq_enable(true);

    driver.connect_handler = [&](uint8_t speed) {
        TUPP_LOG(LOG_DEBUG, "Received speed %d", speed);
        host_events e = (speed !=0)  ? DEVICE_CONNECTED : DEVICE_DISCONNECTED;
        if (_event_queue.available_put()) {
            _event_queue.put(e);
            _event_queue_semaphore.signal();
        } else {
            TUPP_LOG(LOG_ERROR, "HCD event queue full!");
        }
    };
}

[[noreturn]] void usb_host_controller::run() {
    while (true) {
        _event_queue_semaphore.wait();
        while (_event_queue.available_get()) {
            host_events e;
            _event_queue.get(e);
            _state->process_event(e);
        }
    }
}
