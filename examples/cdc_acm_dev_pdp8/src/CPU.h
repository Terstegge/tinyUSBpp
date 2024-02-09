// This class simulates a PDP-8 CPU with registers,
// instructions, operators and interrupt processing.
// Only approximately 300 lines of code do the job :)

#ifndef CPU_H
#define CPU_H

#include <cstdint>
#include "Devices.h"
#include "Memory.h"

const int AND = 00;
const int TAD = 01;
const int ISZ = 02;
const int DCA = 03;
const int JMS = 04;
const int JMP = 05;
const int IOT = 06;
const int OPR = 07;

class CPU
{
public:
    CPU(Memory & m, Devices & d);
    void Reset();
    void run();
    bool singleStep(); // Execute a single CPU cycle.

    // Getter / Setter
    inline bool getIEFF()     { return IEFF; }
    inline int  getIR()       { return MD;   }
    inline int  getAC()       { return AC;   }
    inline void setAC(int ac) { AC = ac;     }
    inline void skip()        { SKIP = true; }

    // Hardware references
    //////////////////////
    Memory  & memory;
    Devices & devices;

private:

    // Current state of CPU
    ///////////////////////
    enum MAJOR_STATE
    {
        FETCH = 4, DEFER = 2, EXECUTE = 1
    };
    MAJOR_STATE nextState;

    // Main registers
    /////////////////
    int PC; // 12 bit Program Counter. Holds address of next instruction to fetch.
    int IR; //  3 bit Instruction Register. Contains actual instruction.
    int AC; // 12 bit ACummulator
    int MA; // 12 bit Memory Address to operate on.
    int MQ; // 12 bit Memory Quotient register.

    // Bus states
    /////////////
    int MD;
    int DATA;

    // Flags
    ////////
    bool L;     //  1 bit Link register.
    bool SKIP;
    bool RUN;   // Run flag
    bool IEFF;  // Interrupt Enable FlipFlop;
    bool IIFF;  // Interrupt Inhibit FlipFlop
    bool GT;    // Greater Flag

    void fetch();
    void defer();
    void execute();
    bool handleInterrupt();

    void operate_instruction_group1();
    void operate_instruction_group2();
    void operate_instruction_group3();
    void iot_instruction();

    // Microcode instructions
    inline void cla() { AC = 0; }
    inline void cll() { L = false; }
    inline void cma() { AC ^= 07777; AC &= 07777; }
    inline void cml() { L = !L; }
    inline void iac() { AC++; L ^= (bool) (AC & 010000);  AC &= 07777; }
    inline void bsw() { AC = ((AC >> 6) | (AC << 6)) & 07777; }
    inline void ral() { AC <<= 1; AC |= L ? 1 : 0; L = AC & 010000; AC &= 07777; }
    inline void rar() { bool t = AC & 01; AC >>= 1; AC |= L ? 04000 : 0; L = t;  }
    inline void r3l() { AC = ((AC << 3) | (AC >> 9)) & 07777; }
    inline void osr() { AC |= 05252; } // dummy value
    inline void hlt() { RUN = false; }
    inline void skon(){ SKIP = IEFF; IEFF = false; }
    inline void ion() { IEFF = IIFF = true; }
    inline void iof() { IEFF = false; }
    inline void sgt() { if (GT) PC = (PC+1) & 07777; }
};

#endif /* CPU_H */
