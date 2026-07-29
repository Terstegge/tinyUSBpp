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
    USB_CLR.MAIN_CTRL.PHY_ISO <<= 1;

    // Enable pulldown resistors
    USB_SET.SIE_CTRL.PULLDOWN_EN   <<= 1;
    USB_SET.SIE_CTRL.KEEP_ALIVE_EN <<= 1;
    USB_SET.SIE_CTRL.EP0_INT_1BUF  <<= 1;

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
    task::sleep_ms(delay_ms);
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

    // Store buffer and callback for TRANS_COMPLETE handler
    _xfer_buffer      = buffer;
    _xfer_complete_cb = complete_cb;
    _xfer_length      = request.wLength;

    // Set device address and endpoint 0
    USB.ADDR_ENDP.ADDRESS  = daddr;
    USB.ADDR_ENDP.ENDPOINT = 0;

    // Copy setup packet to DPRAM
    memcpy(&USB_DPRAM.SETUP_PACKET_LOW, &request, _xfer_length);

    // Set up SIE_CTRL register
    USB_CLR.SIE_CTRL.SEND_DATA    <<= 1;
    USB_CLR.SIE_CTRL.RECEIVE_DATA <<= 1;
    USB_SET.SIE_CTRL.PREAMBLE_EN  <<= 1;  // for now only low speed device
    USB_SET.SIE_CTRL.KEEP_ALIVE_EN<<= 1;

    // Trigger sending
    SIE_CTRL_t tmp = USB.SIE_CTRL;
    tmp.SEND_SETUP  = 1;
    tmp.START_TRANS = 1;
    USB.SIE_CTRL = tmp;

    while(!_trans_triggered) ;
    _trans_triggered = false;
    TUPP_LOG(LOG_INFO, "Stage 2");

    while(!_trans_triggered) ;
    _trans_triggered = false;
    TUPP_LOG(LOG_INFO, "Stage 3");

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
    TUPP_LOG(LOG_DEBUG, "**** IRQ detected, %x", USB.INTS);

    if (USB.INTS.HOST_CONN_DIS) {
        auto speed = (uint8_t)USB.SIE_STATUS.SPEED;
        USB_CLR.SIE_STATUS.SPEED = 0x3;
        if (speed != 0) {
            if (speed == 1) USB_SET.SIE_CTRL.KEEP_ALIVE_EN <<= 1;
            else            USB_SET.SIE_CTRL.SOF_EN <<= 1;
            if (usb_hcd::inst().connect_handler) usb_hcd::inst().connect_handler(speed);
        } else {
            if (usb_hcd::inst().disconnect_handler) usb_hcd::inst().disconnect_handler();
        }
    }

    // --- Transfer complete (SETUP or DATA phase) ---
    if (USB.INTS.TRANS_COMPLETE) {
        usb_hcd::inst()._trans_triggered = true;
        USB.SIE_STATUS.TRANS_COMPLETE = 1;
        if (USB.SIE_CTRL.SEND_SETUP) {
            // Prepare EPx control register. This register is located at
            // address 0x50100100, which is EP0 buffer 0 in device mode.
            // The EPx control register defines the buffer address
            // (EPX_BUFFER = 0x50100180).
            // The input buffer will be EPX_BUFFER (0x50100180)
            EP_CONTROL_t * epx_ctrl = (EP_CONTROL_t *)0x50100100;
            epx_ctrl->value = 0;
            epx_ctrl->ENABLE = 1;
            epx_ctrl->BUFFER_ADDRESS = (uint16_t)(EPX_BUFFER - (uint8_t  *)&USB_DPRAM);
            epx_ctrl->DOUBLE_BUFFERED = 0;
            epx_ctrl->ENDPOINT_TYPE = EP_CONTROL_ENDPOINT_TYPE__Control;

            // Prepare EPx buffer control register
            USB_DPRAM.EP0_IN_BUFFER_CONTROL.LENGTH_0 = 64;
            USB_DPRAM.EP0_IN_BUFFER_CONTROL.PID_0    = 1;
            USB_DPRAM.EP0_IN_BUFFER_CONTROL.LAST_0   = 1;
            USB_DPRAM.EP0_IN_BUFFER_CONTROL.AVAILABLE_0 = 1;

            __DMB();

            // Trigger Phase 2 (reading from device)
            USB_CLR.SIE_CTRL.SEND_SETUP  <<= 1;
            USB_SET.SIE_CTRL.RECEIVE_DATA <<= 1;
            USB_SET.SIE_CTRL.START_TRANS <<= 1;

        } else {

            // DATA phase done → restore interrupt endpoints and invoke callback
            USB.INT_EP_CTRL = usb_hcd::inst()._saved_int_ep_ctrl;
            if (usb_hcd::inst()._xfer_complete_cb) {
                hcd_xfer_result_t result {0};
                result.success    = true;
                result.actual_len = USB_DPRAM.EP0_IN_BUFFER_CONTROL.LENGTH_0;
                TUPP_LOG(LOG_DEBUG, "actual_len: %d", result.actual_len);
                if (usb_hcd::inst()._xfer_buffer && result.actual_len > 0) {
                    uint8_t * d = usb_hcd::inst()._xfer_buffer;
                    for (uint16_t i = 0; i < result.actual_len && i < 64; i++) {
                        TUPP_LOG(LOG_INFO, "%x", EPX_BUFFER[i]);
                        d[i] = EPX_BUFFER[i];
                    }
                }
                usb_hcd::inst()._xfer_complete_cb(result);
            }
        }
    }

    // --- Errors ---
    if (USB.INTS.ERROR_RX_TIMEOUT ||
        USB.INTS.ERROR_DATA_SEQ ||
        USB.INTS.ERROR_CRC) {
        *((volatile uint32_t*)&USB_CLR.SIE_STATUS) = 0xFFFFFFFFu;
        USB.INT_EP_CTRL = usb_hcd::inst()._saved_int_ep_ctrl;
        if (usb_hcd::inst()._xfer_complete_cb) {
            hcd_xfer_result_t result {0};
            result.success    = false;
            result.actual_len = 0;
            usb_hcd::inst()._xfer_complete_cb(result);
        }
    }

    // --- Interrupt endpoint data ---
    if (USB.INTS.BUFF_STATUS) {
        uint32_t buffs = USB.BUFF_STATUS;
        for (int slot = 0; slot < 15; ++slot) {
            uint32_t bit = 1u << ((slot + 1) * 2);
            if (buffs & bit) {
                USB_CLR.BUFF_STATUS = bit;
                auto & ep = usb_hcd::inst()._int_eps[slot];
                if (ep.used && ep.data_handler) {
                    EP_BUFFER_CONTROL_t * bc =
                        (&USB_DPRAM.EP1_IN_BUFFER_CONTROL) + 2 + (slot * 2);
                    ep.data_handler(ep.hw_buffer, (uint16_t)bc->LENGTH_0);
                }
            }
        }
    }
}
}
