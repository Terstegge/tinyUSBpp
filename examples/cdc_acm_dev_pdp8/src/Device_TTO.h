//
// The Teletype output class.
//
#ifndef DEVICETTO_H_
#define DEVICETTO_H_

#include "Device_Base.h"
#include "usb_cdc_acm_device.h"

class Device_TTO : public Device_Base {
public:

    Device_TTO(usb_cdc_acm_device & acm) : FLAG(false), _acm(acm) { }
    unsigned short getDeviceNumber();
    void processPulse(unsigned int pulse, CPU & cpu);
    void cycle();
    void clearFlag();
    bool intRequest();

    virtual ~Device_TTO() { }

private:
    uint8_t c;
    bool FLAG;
    usb_cdc_acm_device & _acm;
};

#endif /* DEVICETTO_H_ */
