//    _   _             _    _  _____ ____
//   | | (_)           | |  | |/ ____|  _ \   _     _
//   | |_ _ _ __  _   _| |  | | (___ | |_) |_| |_ _| |_
//   | __| | '_ \| | | | |  | |\___ \|  _ < _   _|_   _|
//   | |_| | | | | |_| | |__| |____) | |_) | |_|   |_|
//    \__|_|_| |_|\__, |\____/|_____/|____/
//                __/ |
//               |___/
//
// This file is part of tinyUSB++, C++ based and easy to
// use library for USB host/device functionality.
// (c) A. Terstegge  (Andreas.Terstegge@gmail.com)
//
#include "cpu_rp2350.h"
#include "task.h"
#include "usb_hcd.h"
#include "usb_log.h"
#include <cstring>
#include <cstdio>

using namespace _USB_;
using namespace _RESETS_;
using namespace _USB_DPRAM_;
using enum usb_log::log_level;

static uint8_t * const EPX_BUFFER         = (uint8_t *)&USB_DPRAM + 0x180;
static uint8_t * const INT_EP_BUFFER_BASE = (uint8_t *)&USB_DPRAM + 0x1C0;

usb_hcd::usb_hcd() : _xfer_buffer(nullptr) {
    // Reset usb controller
    RESETS_CLR.RESET.USBCTRL <<= 1;
    while (!RESETS.RESET_DONE.USBCTRL);

    // Clear any previous state in dpram just in case
    memset(&USB_DPRAM, 0, 4096);

    // Mux the controller to the onboard usb phy
    USB_SET.USB_MUXING.SOFTCON <<= 1;
    USB_SET.USB_MUXING.TO_PHY <<= 1;

    // Force VBUS detect so the host thinks it supplies power
    USB_SET.USB_PWR.VBUS_DETECT_OVERRIDE_EN <<= 1;
    USB_SET.USB_PWR.VBUS_DETECT <<= 1;

    // Enable the USB controller in host mode and
    // disable the physical isolation (new for RP2350)
    USB_SET.MAIN_CTRL.HOST_NDEVICE <<= 1;
    USB_SET.MAIN_CTRL.CONTROLLER_EN <<= 1;
    USB_SET.SIE_CTRL.VBUS_EN <<= 1; // Forward VBUS to device port (jumper on board)
    USB_CLR.MAIN_CTRL.PHY_ISO <<= 1;

    // Enable pulldown resistors
    USB_SET.SIE_CTRL.PULLDOWN_EN   <<= 1;
    USB_SET.SIE_CTRL.KEEP_ALIVE_EN <<= 1;
    USB_SET.SIE_CTRL.EP0_INT_1BUF  <<= 1;

    // Configure EP1 for control transfers (EPX mechanism)
    uint16_t epx_offset = (uint16_t)(EPX_BUFFER - (uint8_t *)&USB_DPRAM);
    USB_DPRAM.EP1_IN_CONTROL.ENABLE             = 1;
    USB_DPRAM.EP1_IN_CONTROL.ENDPOINT_TYPE      = EP_CONTROL_ENDPOINT_TYPE__Control;
    USB_DPRAM.EP1_IN_CONTROL.BUFFER_ADDRESS     = epx_offset;
    USB_DPRAM.EP1_IN_CONTROL.INTERRUPT_PER_BUFF = 1;
    USB_DPRAM.EP1_OUT_CONTROL.ENABLE             = 1;
    USB_DPRAM.EP1_OUT_CONTROL.ENDPOINT_TYPE      = EP_CONTROL_ENDPOINT_TYPE__Control;
    USB_DPRAM.EP1_OUT_CONTROL.BUFFER_ADDRESS     = epx_offset;
    USB_DPRAM.EP1_OUT_CONTROL.INTERRUPT_PER_BUFF = 1;

    // Enable interrupts
    USB_SET.INTE.HOST_CONN_DIS   <<= 1;
    USB_SET.INTE.TRANS_COMPLETE  <<= 1;
    USB_SET.INTE.BUFF_STATUS     <<= 1;
    USB_SET.INTE.ERROR_RX_TIMEOUT <<= 1;
    USB_SET.INTE.ERROR_DATA_SEQ  <<= 1;
    USB_SET.INTE.ERROR_CRC       <<= 1;
}

void usb_hcd::irq_enable(bool e) {
    TUPP_LOG(LOG_INFO, "irq_enable(%b)", e);
    if (e) {
        cpu_rp2350::clear_pending_peri_irq(USBCTRL_IRQ_IRQn);
        cpu_rp2350::enable_peri_irq(USBCTRL_IRQ_IRQn);
    } else {
        cpu_rp2350::disable_peri_irq(USBCTRL_IRQ_IRQn);
    }
}

void usb_hcd::port_reset(uint8_t delay_ms) {
    TUPP_LOG(LOG_INFO, "port_reset(%d ms)", delay_ms);
    uint8_t speed = USB.SIE_STATUS.SPEED;
    if (speed == 1) {
        USB_SET.SIE_CTRL.KEEP_ALIVE_EN <<= 1;
        USB_SET.SIE_CTRL.SOF_EN <<= 1;
    } else {
        USB_SET.SIE_CTRL.SOF_EN <<= 1;
    }
    USB_SET.SIE_CTRL.RESET_BUS <<= 1;
    task::sleep_ms(delay_ms);
    USB_CLR.SIE_CTRL.RESET_BUS <<= 1;
}

bool usb_hcd::port_connected() {
    return USB.SIE_STATUS.SPEED != 0;
}

void usb_hcd::assign_address(uint8_t addr) {
    TUPP_LOG(LOG_INFO, "usb_hcd: assign_address(%d)", addr);
    USB.ADDR_ENDP.ADDRESS = addr;
}

bool usb_hcd::control_xfer(uint8_t daddr,
                           TUPP::setup_packet_t & request,
                           uint8_t * buffer,
                           std::function<void(hcd_xfer_result_t)> complete_cb) {
    TUPP_LOG(LOG_INFO, "usb_hcd: control_xfer(daddr=%d wLength=%d)", daddr, request.wLength);

    // Set device address and endpoint 0
    USB.ADDR_ENDP.ADDRESS  = daddr;
    USB.ADDR_ENDP.ENDPOINT = 0;

    // Copy setup packet to DPRAM
    volatile uint8_t * dst = (volatile uint8_t *)&USB_DPRAM.SETUP_PACKET_LOW;
    const uint8_t    * src = (const uint8_t *)&request;
    for (int i = 0; i < 8; i++) dst[i] = src[i];

    // Store buffer and callback for TRANS_COMPLETE handler
    _xfer_buffer      = buffer;
    _xfer_complete_cb = complete_cb;
    _xfer_length      = request.wLength;
    _xfer_stage = xfer_stage_t::SETUP; // reset stage for new transfer

    // Clear transaction control bits
    *((volatile uint32_t*)&USB_CLR.SIE_CTRL) = (1u<<1) | (1u<<2) | (1u<<6);
    // Clear TRANS_COMPLETE
    *((volatile uint32_t*)&USB_CLR.SIE_STATUS) = 0xFFFFFFFFu; // clear ALL status bits before new transfer

    // Arm EP0 IN buffer: LAST_BUFF=1, AVAILABLE=1, LENGTH=64
    // (LAST_BUFF is required for TRANS_COMPLETE to fire!)
    *((volatile uint32_t*)&USB_DPRAM.EP0_OUT_BUFFER_CONTROL) = 0;
    *((volatile uint32_t*)&USB_DPRAM.EP0_IN_BUFFER_CONTROL) = (1u<<14) | (1u<<10) | 64u;

    // Disable interrupt endpoints during EPX transfer (RP2350 silicon bug)
    _saved_int_ep_ctrl = USB.INT_EP_CTRL;
    USB.INT_EP_CTRL = 0;

    // Memory barrier: ensure all DPRAM writes are visible to USB SIE
    __DMB();

    // Start SETUP phase: SEND_SETUP + START_TRANS (RMW pattern)
    USB_SET.SIE_CTRL.SEND_SETUP <<= 1;
    USB_SET.SIE_CTRL.START_TRANS <<= 1;

//    uint32_t sie = *((volatile uint32_t*)&USB.SIE_CTRL);
//    sie |= (1u << 1); // SEND_SETUP
//    *((volatile uint32_t*)&USB.SIE_CTRL) = sie;
//    for (volatile int i = 0; i < 12; i++);
//    *((volatile uint32_t*)&USB.SIE_CTRL) = sie | (1u << 0); // START_TRANS

    return true;
}

bool usb_hcd::set_int_ep_handler(uint8_t daddr, uint8_t ep_addr,
                                  std::function<void(uint8_t*, uint16_t)> handler) {
    for (int i = 0; i < 15; ++i) {
        if (_int_eps[i].used &&
            _int_eps[i].daddr   == daddr &&
            _int_eps[i].ep_addr == ep_addr) {
            _int_eps[i].data_handler = handler;
            return true;
        }
    }
    return false;
}

usb_endpoint * usb_hcd::create_endpoint(
        uint8_t daddr, uint8_t ep_addr, ep_attributes_t type,
        uint16_t packet_size, uint8_t interval) {
    (void)type; (void)interval;
    int slot = -1;
    for (int i = 0; i < 15; ++i) {
        if (!_int_eps[i].used) { slot = i; break; }
    }
    if (slot < 0) return nullptr;
    _int_eps[slot].used        = true;
    _int_eps[slot].daddr       = daddr;
    _int_eps[slot].ep_addr     = ep_addr;
    _int_eps[slot].packet_size = packet_size;
    _int_eps[slot].hw_buffer   = INT_EP_BUFFER_BASE + (slot * 64);
    ADDR_ENDP__t * addr_endp = (&USB.ADDR_ENDP_1) + slot;
    addr_endp->ADDRESS   = daddr;
    addr_endp->ENDPOINT  = ep_addr & 0x0f;
    addr_endp->INTEP_DIR = (ep_addr & 0x80) ? 0 : 1;
    uint16_t offset = (uint16_t)(_int_eps[slot].hw_buffer - (uint8_t *)&USB_DPRAM);
    EP_CONTROL_t * ep_ctrl = (&USB_DPRAM.EP2_IN_CONTROL) + (slot * 2);
    ep_ctrl->ENABLE             = 1;
    ep_ctrl->ENDPOINT_TYPE      = EP_CONTROL_ENDPOINT_TYPE__Interrupt;
    ep_ctrl->BUFFER_ADDRESS     = offset;
    ep_ctrl->INTERRUPT_PER_BUFF = 1;
    USB.INT_EP_CTRL.INT_EP_ACTIVE |= (1 << slot);
    return nullptr;
}

extern "C" {
void USBCTRL_IRQ_Handler(void) {

    if (USB.INTS.HOST_CONN_DIS) {
        auto speed = (uint8_t)USB.SIE_STATUS.SPEED;
        *((volatile uint32_t*)&USB_CLR.SIE_STATUS) = 0x300u; // clear SPEED bits
        if (speed != 0) {
            if (speed == 1) USB_SET.SIE_CTRL.KEEP_ALIVE_EN <<= 1;
            else            USB_SET.SIE_CTRL.SOF_EN <<= 1;
            auto & ch = usb_hcd::inst().connect_handler; if (ch) ch();
        } else {
            auto & dh = usb_hcd::inst().disconnect_handler; if (dh) dh();
    // --- Transfer complete ---
        }
    }
    if (USB.INTS.TRANS_COMPLETE) {
        printf("TC\n");
        *((volatile uint32_t*)&USB_CLR.SIE_STATUS) = 0x00040000u;
        printf("stage=%d\n",(int)usb_hcd::inst()._xfer_stage);
        if (usb_hcd::inst()._xfer_stage == usb_hcd::xfer_stage_t::SETUP) {
            // SETUP done - clear first, then arm buffer, then DATA IN
            printf("->DATA\n");
            usb_hcd::inst()._xfer_stage = usb_hcd::xfer_stage_t::DATA;
            USB_CLR.SIE_CTRL.SEND_SETUP  <<= 1;
            USB_CLR.SIE_CTRL.START_TRANS <<= 1;
            __DMB();
            *((volatile uint32_t*)&USB_DPRAM.EP0_IN_BUFFER_CONTROL) = (1u<<14)|(1u<<13)|(1u<<10)|64u;
            __DMB();
            if (USB.SIE_STATUS.SPEED == 1) USB_SET.SIE_CTRL.PREAMBLE_EN <<= 1;
            USB_SET.SIE_CTRL.RECEIVE_DATA <<= 1;
        } else if (usb_hcd::inst()._xfer_stage == usb_hcd::xfer_stage_t::DATA) {
            // DATA done → STATUS OUT
            usb_hcd::inst()._xfer_stage = usb_hcd::xfer_stage_t::STATUS;
            USB_DPRAM.EP1_OUT_BUFFER_CONTROL.LENGTH_0    = 0;
        printf("D->S\n");
            USB_DPRAM.EP1_OUT_BUFFER_CONTROL.PID_0       = 1;
            USB_DPRAM.EP1_OUT_BUFFER_CONTROL.LAST_0      = 1;
            USB_DPRAM.EP1_OUT_BUFFER_CONTROL.AVAILABLE_0 = 1;
            __DMB();
            USB_CLR.SIE_CTRL.RECEIVE_DATA <<= 1;
            USB_SET.SIE_CTRL.SEND_DATA    <<= 1;
            USB_SET.SIE_CTRL.START_TRANS  <<= 1;
        } else {
            // STATUS done → complete
            usb_hcd::inst()._xfer_stage = usb_hcd::xfer_stage_t::SETUP;
            USB.INT_EP_CTRL = usb_hcd::inst()._saved_int_ep_ctrl;
            if (usb_hcd::inst()._xfer_complete_cb) {
                hcd_xfer_result_t result;
        printf("DONE\n");
                result.success    = true;
                result.actual_len = USB_DPRAM.EP0_IN_BUFFER_CONTROL.LENGTH_0;
                if (usb_hcd::inst()._xfer_buffer && result.actual_len > 0) {
                    volatile uint8_t * s = EPX_BUFFER;
                    uint8_t * d = usb_hcd::inst()._xfer_buffer;
                    for (uint16_t i = 0; i < result.actual_len && i < 64; i++) d[i] = s[i];
                }
                usb_hcd::inst()._xfer_complete_cb(result);
            }
        }
    }
    // --- RX Short Packet (DATA IN completed with short packet) ---
    if (USB.INTS.RX_SHORT_PACKET) {
        *((volatile uint32_t*)&USB_CLR.SIE_STATUS) = 0x00001000u;
        if (usb_hcd::inst()._xfer_stage == usb_hcd::xfer_stage_t::DATA) {
            usb_hcd::inst()._xfer_stage = usb_hcd::xfer_stage_t::STATUS;
            USB_DPRAM.EP1_OUT_BUFFER_CONTROL.LENGTH_0    = 0;
            USB_DPRAM.EP1_OUT_BUFFER_CONTROL.PID_0       = 1;
            USB_DPRAM.EP1_OUT_BUFFER_CONTROL.LAST_0      = 1;
            USB_DPRAM.EP1_OUT_BUFFER_CONTROL.AVAILABLE_0 = 1;
            __DMB();
            USB_CLR.SIE_CTRL.RECEIVE_DATA <<= 1;
            USB_SET.SIE_CTRL.SEND_DATA    <<= 1;
            USB_SET.SIE_CTRL.START_TRANS  <<= 1;
        }
    }
    // --- Errors ---
    if (USB.INTS.ERROR_RX_TIMEOUT || USB.INTS.ERROR_CRC) {
        *((volatile uint32_t*)&USB_CLR.SIE_STATUS) = 0xFFFFFFFFu;
        USB.INT_EP_CTRL = usb_hcd::inst()._saved_int_ep_ctrl;
        usb_hcd::inst()._xfer_stage = usb_hcd::xfer_stage_t::SETUP;
        if (usb_hcd::inst()._xfer_complete_cb) {
            hcd_xfer_result_t result {0};
            result.success    = false;
            result.actual_len = 0;
            usb_hcd::inst()._xfer_complete_cb(result);
        }
    }
    // --- BUFF_STATUS (interrupt endpoints) ---
    if (USB.INTS.BUFF_STATUS) {
        uint32_t buffs = USB.BUFF_STATUS;
        // EP0_IN (bit 0) = DATA IN received for control transfer
        if (buffs & 0x5u) { printf("BS=0x%lx\n", buffs); }
        if (buffs & 0x1u) {
            USB_CLR.BUFF_STATUS = 0x1u;
            // DATA phase done via BUFF_STATUS
            if (usb_hcd::inst()._xfer_stage == usb_hcd::xfer_stage_t::DATA) {
                usb_hcd::inst()._xfer_stage = usb_hcd::xfer_stage_t::STATUS;
                USB_DPRAM.EP1_OUT_BUFFER_CONTROL.LENGTH_0    = 0;
                USB_DPRAM.EP1_OUT_BUFFER_CONTROL.PID_0       = 1;
                USB_DPRAM.EP1_OUT_BUFFER_CONTROL.LAST_0      = 1;
                USB_DPRAM.EP1_OUT_BUFFER_CONTROL.AVAILABLE_0 = 1;
                USB_DPRAM.EP1_OUT_BUFFER_CONTROL.FULL_0      = 1;
                __DMB();
                USB_CLR.SIE_CTRL.RECEIVE_DATA <<= 1;
                USB_SET.SIE_CTRL.SEND_DATA    <<= 1;
                USB_SET.SIE_CTRL.START_TRANS  <<= 1;
            }
        }
        for (int slot = 0; slot < 15; ++slot) {
            uint32_t bit = 1u << ((slot + 1) * 2);
            if (buffs & bit) {
                USB_CLR.BUFF_STATUS = bit;
                auto & ep = usb_hcd::inst()._int_eps[slot];
                if (ep.used && ep.data_handler) {
                    EP_BUFFER_CONTROL_t * bc = (&USB_DPRAM.EP1_IN_BUFFER_CONTROL) + 2 + (slot * 2);
                    ep.data_handler(ep.hw_buffer, (uint16_t)bc->LENGTH_0);
                }
            }
        }
    }
}
}
