/********************************************************************************
 * MVP(Multicore Virtual Platform)
 * (C) Copyright CASLab.EE.NCKU 2009
 * All Right Reserved
 *
 * the ARM SP810 status & system control module of MVP emulator
 *
 * Shin-Ying Lee(sing@casmail.ee.ncku.edu.tw)
 * Computer Architecture & System Laboratory
 * Dept. of Electrical Engineering & Inst. of Computer and Communication Engineering
 * National Cheng Kung University, Tainan, Taiwan
 * 2009.09.08
 ********************************************************************************/

/* import header files */
#include <cstring>
#include <iostream>
#include <sysctrl_0.h>
#include <bit_opt.h>

using namespace std;

/* the constructor */
SYSCTRL_0::SYSCTRL_0(sc_module_name name, uint32_t mapping_size, uint32_t sys_id, uint32_t proc_id): ahb_slave_if(mapping_size), sc_module(name)
{
    this->SYS_ID = sys_id;
    this->SYS_PROC_ID_0 = proc_id;
    this->SYS_PROC_ID_1 = 0xFF000000;
    led = 0;
    lock = 0;
    flag = 0;
    counter_24M = 0;
    counter_100 = 0;
    sys_pldctl = 0;

    SC_METHOD(run);
    sensitive << clk.pos();
}

/* running the system controller counter */
void SYSCTRL_0::run(void)
{
    /********************************************************************************
     * NOTE:
     * Originally, the clock rate is running under 100Hz & 24MHz
     * but our simulation clock rate is 4MHz
     * The incremental value should be fixed here
     ********************************************************************************/

    /* the 100Hz system counter */
    static int div = 0;

    if (div >= 40000) {
        counter_100++;
        div = 0;
    }

    else {
        div++;
    }

    /* the 24MHz system counter */
    counter_24M += 6;
}

/* read registers of the system SYSCTRL_0 module */
bool SYSCTRL_0::read_0(uint32_t* data, uint32_t addr, int size)
{
    bool result = true;

    switch (addr) {
        case 0x0000:
            *data = SYS_ID;
            break;  // board ID
        case 0x0008:
            *data = led;
            break;         // the LEDs
        case 0x000c:                                // OSC0
        case 0x0010:                                // OSC1
        case 0x0014:                                // OSC2
        case 0x0018:                                // OSC3
        case 0x001c:
            *data = 0;
            break;           // OSC4
        case 0x0020:
            *data = lock;
            break;
        case 0x0024:
            *data = counter_100;        // 100Hz counter
        case 0x0030:
            *data = flag;
            break;
        case 0x004c:
            *data = 0;
            break;           // flash
        case 0x0050:
            *data = 0x1000;
            break;      // CLCD
        case 0x005c:
            *data = counter_24M;
            break; // 24MHz counter
        case 0x0070:
            *data = 0;
            break;
        case 0x0080:
            *data = 0;
            break;
        case 0x0084:
            *data = SYS_PROC_ID_0;
            break;    // processor ID 0
        case 0x0088:
            *data = SYS_PROC_ID_1;
            break;    // processor ID 1
            // for Realview EB rev "B", spec is for rev "C"
            // http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.faqs/ka4132.html
        case 0x00d8:
            printm(d_sysctrl_0, "read realvieweb interrupt mode");
            *data = sys_pldctl;
            break;
        default:
            printb(d_sysctrl_0, "read unknown: 0x%.4x", addr);
            result = false;
            break;
    }

    return result;
}

/* write registers of the system SYSCTRL_0 module */
bool SYSCTRL_0::write_0(uint32_t data, uint32_t addr, int size)
{
    bool result = true;
    uint8_t int_mode;

    switch (addr) {
        case 0x0008:
            led = data;
            break;
        case 0x000c:                            // OSC0
        case 0x0010:                            // OSC1
        case 0x0014:                            // OSC2
        case 0x0018:                            // OSC3
        case 0x001c:
            break;                  // OSC4
        case 0x0020:
            lock = data;
            break;
        case 0x0030:
            flag = data;
            break;
        case 0x004c:                            // flash
        case 0x0050:                            // CLCD
        case 0x0070:
            break;
            // for Realview EB rev "B", spec is for rev "C"
            // http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.faqs/ka4132.html
        case 0x00d8:
            sys_pldctl = data;
            int_mode = SMM(data, 24, 22);
            printm(d_sysctrl_0, "write to realvieweb rev B interrupt mode: 0x%X", int_mode);
            break;
            //case 0x0080:    break;
        default:
            printb(d_sysctrl_0, "write unknown: 0x%.4x\n", addr);
            result = false;
            break;
    }

    return result;
}

bool SYSCTRL_0::local_access(bool write, uint32_t addr, uint32_t& data, unsigned int length)
{
    uint32_t local_address = addr & get_address_mask();

    if (write) {
        return this->write_0(data, local_address, length);
    }

    else {
        return this->read_0(&data, local_address, length);
    }
}
