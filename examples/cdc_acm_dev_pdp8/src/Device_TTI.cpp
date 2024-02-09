#include <cassert>
#include "Device_TTI.h"
#include "CPU.h"

unsigned short Device_TTI::getDeviceNumber() {
    return 003;
}

char Device_TTI::toUpper(char c) {
    if (c >= 'a' && c <= 'z')
        return c - ('a'-'A');
    else
        return c;
}

void Device_TTI::cycle() {
    if (!FLAG && _acm.available()) {
        FLAG = true;
        _acm.read(&c, 1);
        c = toUpper(c);
        // Use CTRL-D as RUBOUT
        if (c==4) c = 0x7f;
    }
}

void Device_TTI::clearFlag() {
    FLAG = false;
}

bool Device_TTI::intRequest() {
    return FLAG;
}

void Device_TTI::processPulse(unsigned int pulse, CPU & cpu) {

    switch (pulse) {
    // KCF (Keyboard Clear Flags)
    /////////////////////////////
    case 00: {
        FLAG=false;
        break;
    }
    // KSF (Keyboard Skip if Flag)
    //////////////////////////////
    case 01: {
        if (FLAG) cpu.skip();
        break;
    }
    // KCC (Keyboard Clear and read character)
    //////////////////////////////////////////
    case 02: {
        cpu.setAC(0);
        FLAG = false;
        break;
    }
    // KRB (Keyboard Read and begin next read)
    //////////////////////////////////////////
    case 06: {
        cpu.setAC(0);
        FLAG = false;
    }
    // KRS (Keyboard Read Static)
    /////////////////////////////
    /* fall through */
    case 04: {
        cpu.setAC( cpu.getAC() | c);
        break;
    }
    default:
        assert(0);
        break;
    }

}
