//
// Created by andreas on 07.07.26.
//
#ifndef USB_HOST_TEST_USB_HOST_CONTROLLER_H
#define USB_HOST_TEST_USB_HOST_CONTROLLER_H

#include "usb_hcd_interface.h"
#include "usb_host_controller_states.h"
#include "usb_log.h"
using enum usb_log::log_level;

#include "FIFO.h"
#include "task.h"

#include <cstdio>


class usb_host_controller : public task {
public:
    friend class host_disconnected;
    friend class host_get8;

    explicit usb_host_controller(usb_hcd_interface & driver);

private:

    [[noreturn]] void run() override;

    // Host states
    host_disconnected _host_disconnected;
    host_get8         _host_get8;

    // The current host state
    hc_state * _state {nullptr};

    inline void set_state(hc_state * s) {
        if (_state) _state->leave();
        _state = s;
        if (_state) _state->enter();
    }

    usb_hcd_interface & _driver;
    FIFO<host_events> _event_queue {100};
};

#endif //USB_HOST_TEST_USB_HOST_CONTROLLER_H
