//
// The Teletype input class.
//
#ifndef DEVICETTI_H_
#define DEVICETTI_H_

#include "Device_Base.h"
#include "usb_cdc_acm_device.h"

class Device_TTI : public Device_Base {
public:

    Device_TTI(usb_cdc_acm_device & acm) : FLAG(false), c(0), _acm(acm) { };
    unsigned short getDeviceNumber();
    void processPulse(unsigned int pulse, CPU & cpu);
    void cycle();
    void clearFlag();
    bool intRequest();

    virtual ~Device_TTI() { }

private:
    char toUpper(char c);
    bool FLAG;
    uint8_t c;
    usb_cdc_acm_device & _acm;
};

#endif /* DEVICETTI_H_ */
