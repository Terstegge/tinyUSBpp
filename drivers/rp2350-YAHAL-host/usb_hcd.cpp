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
#include "usb_hcd.h"
#include "usb_log.h"
#include <cstring>
#include <cassert>

using namespace _USB_;
using namespace _RESETS_;
using namespace _USB_DPRAM_;
using enum usb_log::log_level;

static uint8_t * const EPX_BUFFER = (uint8_t *)&USB_DPRAM + 0x180;
static uint8_t * const INT_EP_BUFFER_BASE = (uint8_t *)&USB_DPRAM + 0x1C0;

usb_hcd::usb_hcd()
: _xfer_buffer(nullptr)
{
    RESETS_CLR.RESET.USBCTRL <<= 1;
    while (!RESETS.RESET_DONE.USBCTRL);

    memset(&USB_DPRAM, 0, 4096);

    USB_SET.USB_MUXING.SOFTCON  <<= 1;
    USB_SET.USB_MUXING.TO_PHY   <<= 1;

    USB_SET.MAIN_CTRL.CONTROLLER_EN  <<= 1;
    USB_SET.MAIN_CTRL.HOST_NDEVICE   <<= 1;   // Device=0, Host=1
    USB_CLR.MAIN_CTRL.PHY_ISO       <<= 1;

    USB_SET.SIE_CTRL.VBUS_EN        <<= 1;
    USB_SET.SIE_CTRL.PULLDOWN_EN    <<= 1;
    USB_SET.SIE_CTRL.KEEP_ALIVE_EN  <<= 1;

    uint16_t epx_offset = (uint16_t)(EPX_BUFFER - (uint8_t *)&USB_DPRAM);
    USB_DPRAM.EP1_IN_CONTROL.ENABLE         = 1;
    USB_DPRAM.EP1_IN_CONTROL.ENDPOINT_TYPE  = EP_CONTROL_ENDPOINT_TYPE__Control;
    USB_DPRAM.EP1_IN_CONTROL.BUFFER_ADDRESS = epx_offset;
    USB_DPRAM.EP1_OUT_CONTROL.ENABLE         = 1;
    USB_DPRAM.EP1_OUT_CONTROL.ENDPOINT_TYPE  = EP_CONTROL_ENDPOINT_TYPE__Control;
    USB_DPRAM.EP1_OUT_CONTROL.BUFFER_ADDRESS = epx_offset;

    USB_SET.INTE.HOST_CONN_DIS  <<= 1;
    USB_SET.INTE.BUFF_STATUS    <<= 1;
    USB_SET.INTE.TRANS_COMPLETE <<= 1;
}

void usb_hcd::irq_enable(bool e) {
    if (e) {
        cpu_rp2350::clear_pending_peri_irq(USBCTRL_IRQ_IRQn);
        cpu_rp2350::enable_peri_irq(USBCTRL_IRQ_IRQn);
    } else {
        cpu_rp2350::disable_peri_irq(USBCTRL_IRQ_IRQn);
    }
}

bool usb_hcd::port_connected() {
    return USB.SIE_STATUS.SPEED != 0;
}

void usb_hcd::port_reset() {
    TUPP_LOG(LOG_INFO, "usb_hcd: resetting bus");
    USB_SET.SIE_CTRL.RESET_BUS <<= 1;
}

void usb_hcd::assign_address(uint8_t addr) {
    TUPP_LOG(LOG_INFO, "usb_hcd: assign_address(%d)", addr);
    USB.ADDR_ENDP.ADDRESS = addr;
}

bool usb_hcd::control_xfer(uint8_t daddr,
                            TUPP::setup_packet_t & request,
                            uint8_t * buffer,
                            std::function<void(hcd_xfer_result_t)> complete_cb) {
    TUPP_LOG(LOG_INFO, "usb_hcd: control_xfer(daddr=%d)", daddr);

    USB.ADDR_ENDP.ADDRESS  = daddr;
    USB.ADDR_ENDP.ENDPOINT = 0;

    memcpy((void *)&USB_DPRAM.SETUP_PACKET_LOW, &request, 8);

    _xfer_buffer      = buffer;
    _xfer_complete_cb = complete_cb;

    USB_DPRAM.EP0_OUT_BUFFER_CONTROL.AVAILABLE_0 = 0;
    USB_DPRAM.EP0_IN_BUFFER_CONTROL.AVAILABLE_0  = 0;

    USB_SET.SIE_CTRL.SEND_SETUP  <<= 1;
    USB_SET.SIE_CTRL.START_TRANS <<= 1;

    return true;
}

bool usb_hcd::set_int_ep_handler(uint8_t daddr, uint8_t ep_addr,
                                  std::function<void(uint8_t*, uint16_t)> handler) {
    for (int i = 0; i < 15; ++i) {
        if (_int_eps[i].used &&
            _int_eps[i].daddr   == daddr &&
            _int_eps[i].ep_addr == ep_addr) {
            _int_eps[i].data_handler = handler;
            TUPP_LOG(LOG_INFO, "usb_hcd: registered handler for daddr=%d ep=0x%x",
                      daddr, ep_addr);
            return true;
        }
    }
    TUPP_LOG(LOG_WARNING, "usb_hcd: set_int_ep_handler - no matching slot found "
              "for daddr=%d ep=0x%x", daddr, ep_addr);
    return false;
}

usb_endpoint * usb_hcd::create_endpoint(
        uint8_t         daddr,
        uint8_t         ep_addr,
        ep_attributes_t type,
        uint16_t        packet_size,
        uint8_t         interval) {
    (void)type; (void)interval;

    int slot = -1;
    for (int i = 0; i < 15; ++i) {
        if (!_int_eps[i].used) { slot = i; break; }
    }
    if (slot < 0) {
        TUPP_LOG(LOG_ERROR, "usb_hcd: no free interrupt endpoint slots!");
        return nullptr;
    }

    TUPP_LOG(LOG_INFO, "usb_hcd: create_endpoint slot=%d daddr=%d ep=0x%x",
              slot, daddr, ep_addr);

    _int_eps[slot].used        = true;
    _int_eps[slot].daddr       = daddr;
    _int_eps[slot].ep_addr     = ep_addr;
    _int_eps[slot].packet_size = packet_size;
    _int_eps[slot].hw_buffer   = INT_EP_BUFFER_BASE + (slot * 64);

    ADDR_ENDP__t * addr_endp = (&USB.ADDR_ENDP_1) + slot;
    addr_endp->ADDRESS  = daddr;
    addr_endp->ENDPOINT = ep_addr & 0x0f;
    addr_endp->INTEP_DIR = (ep_addr & 0x80) ? 0 : 1; // 0=IN, 1=OUT

    uint16_t offset = (uint16_t)(_int_eps[slot].hw_buffer - (uint8_t *)&USB_DPRAM);
    EP_CONTROL_t * ep_ctrl = (&USB_DPRAM.EP2_IN_CONTROL) + (slot * 2);
    ep_ctrl->ENABLE         = 1;
    ep_ctrl->ENDPOINT_TYPE  = EP_CONTROL_ENDPOINT_TYPE__Interrupt;
    ep_ctrl->BUFFER_ADDRESS = offset;

    USB.INT_EP_CTRL.INT_EP_ACTIVE |= (1 << slot);

    TUPP_LOG(LOG_WARNING, "usb_hcd::create_endpoint: hardware configured, "
              "but no usb_endpoint wrapper yet - returns nullptr "
              "(use set_int_ep_handler() to receive reports for now)");
    return nullptr;
}

extern "C" {
void USBCTRL_IRQ_Handler(void) {
    if (USB.INTS.HOST_CONN_DIS) {
        if (usb_hcd::inst().port_connected()) {
            if (usb_hcd::inst().connect_handler) {
                usb_hcd::inst().connect_handler();
            }
        } else {
            if (usb_hcd::inst().disconnect_handler) {
                usb_hcd::inst().disconnect_handler();
            }
        }
    }

    if (USB.INTS.TRANS_COMPLETE) {
        USB_CLR.SIE_STATUS.TRANS_COMPLETE <<= 1;
        if (usb_hcd::inst()._xfer_complete_cb) {
            hcd_xfer_result_t result;
            result.success    = true;
            result.actual_len = USB_DPRAM.EP0_IN_BUFFER_CONTROL.LENGTH_0;
            usb_hcd::inst()._xfer_complete_cb(result);
        }
    }

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
