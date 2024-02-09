// Memory interface for the PDP-8 core memory.
// The core memory is mapped to a RAM region
// of the MSP432. The 4096 12-bit words are stored
// internally as 16 bit integers, so only 8K RAM are
// used as the total PDP-8 memory!
#ifndef MEMORY_H_
#define MEMORY_H_

#include <cstdint>

const int NUM_WORDS = 4096;
extern uint16_t coreMemory[4096];

class Memory
{
public:

    // Get value of memory address.
    inline uint16_t read(uint16_t address) const {
        return coreMemory[address];
    }

    // Set value of memory address.
    inline void store(uint16_t address, uint16_t value) {
        coreMemory[address] = value;
    }

    // Increment content of address and return value
    uint16_t inc (uint16_t address);
};

#endif /* MEMORY_H_ */

