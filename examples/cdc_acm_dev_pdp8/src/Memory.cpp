#include "Memory.h"

uint16_t Memory::inc(uint16_t address) {
    uint16_t tmp = coreMemory[address];
    tmp += 1;
    tmp &= 07777;
    coreMemory[address] = tmp;
    return tmp;
}

