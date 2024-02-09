#include "CPU.h"

CPU::CPU(Memory & mem, Devices & d) : memory(mem), devices(d) {
    Reset();
}

// Set all registers to initial values and put CPU in running state.
void CPU::Reset() {
    // Initialize main registers
    PC = 0200;
    IR = 0;    // Instruction.
    AC = 0;
    MA = 0200;
    MQ = 0;

    // Initialize flags
    L    = false;
    SKIP = false;
    RUN  = false;
    IEFF = false;
    IIFF = false;
    GT   = false;

    // Next state
    nextState = FETCH;
}

// Run until we encounter a HLT
void CPU::run() {
    while( singleStep() ) ;
}

// Perform a single CPU cycle/state.
bool CPU::singleStep() {
    // Write AC to P2, so we have some 'blinking' :)
//    P2->OUT = AC;

    RUN = true;
    devices.cycle();
    switch (nextState) {
        case FETCH:   { fetch();   break; }
        case DEFER:   { defer();   break; }
        case EXECUTE: { execute(); break; }
    }
    return RUN;
}

void CPU::fetch() {
    /////////////////
    ////// TS 1 /////
    /////////////////
    SKIP = false;
    PC   = MA + 1;

    /////////////////
    ////// TS 2 /////
    /////////////////
    DATA = AC;
    MD   = memory.read(MA);
    IR   = (MD & 07000) >> 9;

    /////////////////
    ////// TS 3 /////
    /////////////////
    int  page         = (MA & 07600);
    int  rel_adr      = (MD & 00177);
    bool current_page = (MD & 00200);
    bool indirect     = (MD & 00400);

    switch (IR)
    {
        // JMP  (PC = MA)
        /////////////////
        case JMP: {
            PC = rel_adr;
            if (current_page) {
                PC |= page;
            }
            break;
        }
        // IOT instructions
        ///////////////////
        case IOT: {
            iot_instruction();
            break;
        }
        // Operate instructions
        ///////////////////////
        case OPR: {
            switch (MD & 0401) {
                case 0000: // fall through
                case 0001: operate_instruction_group1(); break;
                case 0400: operate_instruction_group2(); break;
                case 0401: operate_instruction_group3(); break;
            }
            DATA = AC;
            break;
        }
    }

    /////////////////
    ////// TS 4 /////
    /////////////////
    if (IR < JMP || (IR == JMP && indirect)) {
        MA = rel_adr;
        if (current_page) {
            MA |= page;   // Current page
        }
        nextState = EXECUTE;
        if (indirect) {
            nextState = DEFER;
        }
        return;
    }

    if (handleInterrupt()) return;
    MA = PC;
    if (SKIP) MA++;
    nextState = FETCH;
}

void CPU::defer()
{
    /////////////////
    ////// TS 1 /////
    /////////////////
    MD = memory.read(MA);

    /////////////////
    ////// TS 2 /////
    /////////////////
    if ((MA & 07770) == 010)
        MD = memory.inc(MA);

    /////////////////
    ////// TS 3 /////
    /////////////////
    if (IR == JMP) {
        PC = MD;
        /////////////////
        ////// TS 4 /////
        /////////////////
        if (handleInterrupt()) return;
        MA = PC;
        nextState = FETCH;
    } else {
        MA = MD;
        nextState = EXECUTE;
    }
}

void CPU::execute()
{
    switch (IR) {
        // AND  (AC = AC & M[MA])
        /////////////////////////
        case AND:
            AC &= memory.read(MA);
            break;

        // TAD  (L'AC = AC + M[MA], L is complemented if carry)
        ///////////////////////////////////////////////////////
        case TAD:
            AC += memory.read(MA);
            L ^= (bool) (AC & 010000); // cast necessary !!
            AC &= 07777;
            break;

        // ISZ  (M[MA] = M[MA] + 1, skip if M[MA] == 0)
        ///////////////////////////////////////////////
        case ISZ:
            if (memory.inc(MA) == 0) SKIP = true;
            break;

        // DCA  (M[MA] = AC, AC = 0)
        ////////////////////////////
        case DCA:
            memory.store(MA, AC);
            cla();
            break;

        // JMS  (M[MA] = PC, PC = MA + 1)
        /////////////////////////////////
        case JMS:
            if (SKIP) {
                PC++;
                SKIP = false;
            }  // if INT !!
            memory.store(MA, PC);
            PC = (MA + 1) & 07777;
            break;
    }

    if (handleInterrupt()) return;
    MA = PC;
    if (SKIP) MA++;
    nextState = FETCH;
}

bool CPU::handleInterrupt()
{
    if (IIFF) {
        // Next instruction will be fetched, regardless of
        // a pending interrupt.
        IIFF = false;
    } else {
        // Process interrupt request
        if (devices.intRequest() && IEFF) {
            IR = JMS;
            MA = 0;
            IEFF = false;
            nextState = EXECUTE;
            return true;
        }
    }
    return false;
}

// Operate Instructions Group 1
///////////////////////////////
void CPU::operate_instruction_group1() {
    if (MD & 0200) cla();
    if (MD & 0100) cll();
    if (MD & 0040) cma();
    if (MD & 0020) cml();
    if (MD & 0001) iac();
    switch (MD & 016) {
        case 000: break; // No rotate
        case 002: bsw(); break;
        case 006: ral();
        /* fall through */
        case 004: ral(); break;
        case 012: rar();
        /* fall through */
        case 010: rar(); break;
        case 014: AC &= MD; break; //r3l(); break;
        case 016: break; // Do not use
    }
}

// Operate Instructions Group 2
///////////////////////////////
void CPU::operate_instruction_group2() {
    if (MD & 010) {
        SKIP = true;
        if (MD & 0100) SKIP &= !(AC & 04000); // SPA
        if (MD & 0040) SKIP &= !(AC == 0);    // SNA
        if (MD & 0020) SKIP &= !L;            // SZL
    } else {
        SKIP = false;
        if (MD & 0100) SKIP |= (AC & 04000);  // SMA
        if (MD & 0040) SKIP |= (AC == 0);     // SZA
        if (MD & 0020) SKIP |= L;             // SNL
    }
    if (MD & 0200) cla();
    if (MD & 0004) osr();
    if (MD & 0002) hlt();
}

void CPU::operate_instruction_group3() {
    // Operate Instructions Group 3
    ///////////////////////////////
    int t = MQ;
    if (MD & 0200) { cla(); }
    if (MD & 0020) { MQ = AC; cla(); };  // MQL
    if (MD & 0100) { AC |= t; };         // MQA
    if (MD & 0056) { IIFF = true; }
}

void CPU::iot_instruction() {
    uint16_t dev   = (MD & 00770) >> 3;
    uint16_t pulse = (MD & 00007);

    switch (dev) {
        // CPU_alt internal control;
        case 00: {
            switch (pulse) {
                case 00: skon(); break;
                case 01: ion();  break;
                case 02: iof();  break;
                case 03: SKIP = devices.intRequest(); break; // SRQ
                case 04:
                    AC = 0; // GTF
                    if (L)    AC |= 04000;
                    if (GT)   AC |= 02000;
                    if (devices.intRequest()) AC |= 01000;
                    if (IIFF) AC |= 0400;
                    if (IEFF) AC |= 0200;
                    break;
                case 05:
                    L  = (bool) (AC & 04000); // RTF
                    GT = (bool) (AC & 02000);
                    // IIFF = (bool)(AC & 00400);
                    ion(); // IEFF = (bool)(AC & 00200);
                    break;
                case 06: sgt(); break;
                case 07:
                    cla();
                    cll();
                    GT   = false;
                    IEFF = false;
                    devices.clearFlags();
                    break;
            }
            break;
        }
        // Any dev > 0 is a real device...
        default:
            devices.processIotInstruction(*this);
            break;
    }
}

