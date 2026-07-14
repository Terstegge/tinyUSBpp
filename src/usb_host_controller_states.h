//
// Created by andreas on 07.07.26.
//

#ifndef USB_HOST_TEST_USB_HOST_CONTROLLER_STATES_H
#define USB_HOST_TEST_USB_HOST_CONTROLLER_STATES_H

enum host_events {
    DEVICE_CONNECTED,
    DEVICE_DISCONNECTED,
    TRANSFER_COMPLETE
};

class usb_host_controller;

class hc_state {
public:
    explicit hc_state(usb_host_controller * c) : _context{c} {}
    virtual void enter() {}
    virtual void leave() {}

    virtual void process_event(host_events) {}

protected:
    usb_host_controller * _context;
};


class host_disconnected : public hc_state {
public:
    explicit host_disconnected(usb_host_controller * c) : hc_state(c) { }
    void process_event(host_events e) override;
    void enter() override;
    void leave() override;
};

class host_get8 : public hc_state {
public:
    explicit host_get8(usb_host_controller * c) : hc_state(c) { }
    void enter() override;
    void process_event(host_events e) override;
private:
    uint8_t _buffer[8] {0};
};

#endif //USB_HOST_TEST_USB_HOST_CONTROLLER_STATES_H
