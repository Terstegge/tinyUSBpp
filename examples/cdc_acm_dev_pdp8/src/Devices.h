//
// A class to handle all kinds of PDP-8 devices
//
#ifndef DEVICES_H_
#define DEVICES_H_

#include "Device_Base.h"
class CPU;

class Devices {

public:
    Devices();
    void addDevice(Device_Base * dev);
    void processIotInstruction(CPU & cpu);
    bool intRequest();
    void cycle();
    void clearFlags();

private:
    Device_Base *devices[10];
};

#endif /* DEVICES_H_ */
