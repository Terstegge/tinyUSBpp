#include <cassert>
#include "Device_TTO.h"
#include "CPU.h"

unsigned short Device_TTO::getDeviceNumber() {
    return 004;
}

void Device_TTO::cycle() {
}

void Device_TTO::clearFlag() {
    FLAG = false;
}

bool Device_TTO::intRequest() {
    return FLAG;
}

// The Teleprinter FLAG has to stay cleared after a TCF, so
// that the Focal interpreter has a chance to produce new
// characters to output. Only in the case of a TLS/TPC, the
// FLAG will be automatically set if the Teleprinter is ready
// to receive the next character. Here the FLAG is set
// immediately after a character is printed.

void Device_TTO::processPulse(unsigned int pulse, CPU & cpu) {

    switch (pulse) {
    // TFL (Teleprinter Flag set)
    /////////////////////////////
    case 0: {
        FLAG=true;
        break;
    }
    // TSF (Teleprinter Skip if Flag)
    /////////////////////////////////
    case 1: {
        if (FLAG) cpu.skip();
        break;
    }
    // TCF (Teleprinter Clear Flag)
    ///////////////////////////////
    case 2: {
        FLAG=false;
        break;
    }
    // TLS (Teleprinter Load and start)
    ///////////////////////////////////
    case 6:
        FLAG=false;
    // TPC (Teleprinter Print Character)
    ////////////////////////////////////
    /* fall through */
    case 4: {
        c = cpu.getAC() & 0177;
        if (c == 127) c = 7;
        if (c !=  12) _acm.write(&c, 1);
        // Ready to receive the next character
        FLAG=true;
        break;
    }
    default:
        assert(0);
        break;
    }
}
