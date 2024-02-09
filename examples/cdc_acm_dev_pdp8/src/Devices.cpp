#include "Devices.h"
#include "CPU.h"

Devices::Devices() {
    for (int i=0; i < 10; i++)
        devices[i] = 0;
}

void Devices::addDevice(Device_Base *dev) {
    for (int i=0; i < 10; i++)
        if (devices[i] == 0) {
            devices[i] = dev;
            break;
        }
}

void Devices::processIotInstruction(CPU & cpu) {
    int IR = cpu.getIR();
    uint16_t dev   = (IR & 00770) >> 3;
    uint16_t pulse = (IR & 00007);
    int i;

    for (i=0; i < 10; i++) {
        if (devices[i] != 0) {
            if (devices[i]->getDeviceNumber() == dev) {
                devices[i]->processPulse(pulse, cpu);
                break;
            }
        }
    }
    if (i==10) {
        //cout << "Unknown device: " << oct << dev << endl;
    }
}

bool Devices::intRequest() {
    for (int i=0; i < 10; i++) {
        if (devices[i] != 0) {
            if (devices[i]->intRequest())
                return true;
        }
    }
    return false;
}

void Devices::cycle() {
    for (int i=0; i < 10; i++) {
        if (devices[i] != 0)
            devices[i]->cycle();
    }
}

void Devices::clearFlags() {
    for (int i=0; i < 10; i++) {
        if (devices[i] != 0)
            devices[i]->clearFlag();
    }
}
