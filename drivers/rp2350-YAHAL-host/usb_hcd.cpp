#include "cpu_rp2350.h"
#include "usb_hcd.h"
#include "usb_log.h"
#include <cstring>
#include <cassert>
#include <cstdio>

using namespace _USB_;
using namespace _RESETS_;
using namespace _USB_DPRAM_;
using enum usb_log::log_level;

static uint8_t * const EPX_BUFFER        = (uint8_t *)&USB_DPRAM + 0x180;
static uint8_t * const INT_EP_BUFFER_BASE = (uint8_t *)&USB_DPRAM + 0x1C0;

usb_hcd::usb_hcd() : _xfer_buffer(nullptr) {
    RESETS_CLR.RESET.USBCTRL <<= 1;
    while (!RESETS.RESET_DONE.USBCTRL);
    memset(&USB_DPRAM, 0, 4096);
    USB_SET.USB_MUXING.SOFTCON <<= 1;
    USB_SET.USB_MUXING.TO_PHY  <<= 1;
    USB_SET.USB_PWR.VBUS_DETECT_OVERRIDE_EN <<= 1;
    USB_SET.USB_PWR.VBUS_DETECT             <<= 1;
    USB_SET.SIE_CTRL.VBUS_EN <<= 1; // Enable VBUS drive (required for host to actually drive the bus!)
    USB_SET.MAIN_CTRL.CONTROLLER_EN <<= 1;
    USB_SET.MAIN_CTRL.HOST_NDEVICE  <<= 1;
    USB_CLR.MAIN_CTRL.PHY_ISO      <<= 1;
    USB_SET.SIE_CTRL.PULLDOWN_EN   <<= 1;
    USB_SET.SIE_CTRL.KEEP_ALIVE_EN <<= 1;
    USB_SET.SIE_CTRL.EP0_INT_1BUF  <<= 1;
    USB_SET.INTE.HOST_CONN_DIS   <<= 1;
    USB_SET.INTE.BUFF_STATUS     <<= 1;
    USB_SET.INTE.TRANS_COMPLETE  <<= 1;
    USB_SET.INTE.ERROR_RX_TIMEOUT <<= 1;
    USB_SET.INTE.ERROR_DATA_SEQ  <<= 1;
    USB_SET.INTE.ERROR_CRC       <<= 1;
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
    TUPP_LOG(LOG_INFO, "usb_hcd: port_reset()");
    uint8_t speed = USB.SIE_STATUS.SPEED;
    if (speed == 1) USB_SET.SIE_CTRL.KEEP_ALIVE_EN <<= 1;
    else            USB_SET.SIE_CTRL.SOF_EN <<= 1;
    USB_SET.SIE_CTRL.RESET_BUS <<= 1;
}

void usb_hcd::port_reset_end() {
    TUPP_LOG(LOG_INFO, "usb_hcd: port_reset_end()");
    USB_SET.SIE_CTRL.KEEP_ALIVE_EN <<= 1; // maintain bus after reset
    USB_CLR.SIE_CTRL.RESET_BUS <<= 1;
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
    USB.ADDR_ENDP.ADDRESS  = daddr;
    USB.ADDR_ENDP.ENDPOINT = 0;
    volatile uint8_t * dst = (volatile uint8_t *)&USB_DPRAM.SETUP_PACKET_LOW;
    const uint8_t    * src = (const uint8_t *)&request;
    for (int i = 0; i < 8; i++) dst[i] = src[i];
    printf("  SETUP: %02x %02x %02x %02x %02x %02x %02x %02x\n",
           dst[0],dst[1],dst[2],dst[3],dst[4],dst[5],dst[6],dst[7]);
    _xfer_buffer      = buffer;
    _xfer_complete_cb = complete_cb;
    _xfer_length      = request.wLength;
    // Clear transaction bits
    *((volatile uint32_t*)&USB_CLR.SIE_CTRL) = (1u<<1) | (1u<<2) | (1u<<6);
    *((volatile uint32_t*)&USB_CLR.SIE_STATUS) = 0x00040000u; // Clear TRANS_COMPLETE
    // NO buffer control for SETUP phase — tinyusb confirms SETUP uses only SETUP_PACKET_LOW/HIGH!
    // Disable interrupt endpoints during EPX transfer (RP2350 silicon bug #3533)
    _saved_int_ep_ctrl = USB.INT_EP_CTRL;
    USB.INT_EP_CTRL = 0;
    // Atomic write of SIE_CTRL (mirrors tinyusb sie_start_xfer)
    uint32_t sie = *((volatile uint32_t*)&USB.SIE_CTRL);
    sie |= (1u << 1); // SEND_SETUP
    if (USB.SIE_STATUS.SPEED == 1) sie |= (1u << 6); // PREAMBLE_EN
    *((volatile uint32_t*)&USB.SIE_CTRL) = sie;
    asm volatile("dsb" ::: "memory"); // memory barrier instead of empty loop
    *((volatile uint32_t*)&USB.SIE_CTRL) = sie | (1u << 0); // START_TRANS
    printf("  SIE_CTRL after START_TRANS=0x%08lx\n", *((volatile uint32_t*)&USB.SIE_CTRL));
    printf("  EP1_OUT_BUF=0x%08lx EP1_IN_BUF=0x%08lx\n",
           *((volatile uint32_t*)(0x50100000 + 0x8c)),
           *((volatile uint32_t*)(0x50100000 + 0x88)));
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
    uint32_t ints = *((volatile uint32_t*)&USB.INTS);

    if (ints & 0x01u) { // HOST_CONN_DIS
        uint8_t speed = (uint8_t)USB.SIE_STATUS.SPEED;
        printf("IRQ: HOST_CONN_DIS speed=%d\n", speed);
        *((volatile uint32_t*)&USB_CLR.SIE_STATUS) = 0x300u;
        if (speed != 0) {
            if (speed == 1) USB_SET.SIE_CTRL.KEEP_ALIVE_EN <<= 1;
            else USB_SET.SIE_CTRL.SOF_EN <<= 1;
            if (usb_hcd::inst().connect_handler) usb_hcd::inst().connect_handler();
        } else {
            if (usb_hcd::inst().disconnect_handler) usb_hcd::inst().disconnect_handler();
        }
    }

    if (ints & 0x08u) { // TRANS_COMPLETE
        uint32_t sie = *((volatile uint32_t*)&USB.SIE_CTRL);
        printf("IRQ: TRANS_COMPLETE SIE_CTRL=0x%08lx\n", sie);
        *((volatile uint32_t*)&USB_CLR.SIE_STATUS) = 0x00040000u;

        if (sie & (1u << 1)) {
            // SEND_SETUP still set: SETUP phase done, now start DATA IN phase
            printf("IRQ: SETUP done, starting DATA IN phase\n");
            *((volatile uint32_t*)&USB_CLR.SIE_CTRL) = (1u<<1); // clear SEND_SETUP
            // Arm EP1 IN buffer: AVAILABLE=1, LENGTH=64, PID=1(DATA1), LAST=1
            uint32_t buf_ctrl = (1u << 10) | (1u << 14) | (1u << 13) | 64u;
            *((volatile uint32_t*)&USB_DPRAM.EP1_IN_BUFFER_CONTROL) = buf_ctrl;
            // Trigger DATA IN
            uint32_t sie2 = *((volatile uint32_t*)&USB.SIE_CTRL);
            sie2 |= (1u << 3); // RECEIVE_DATA (bit 3, not 2!)
            if (USB.SIE_STATUS.SPEED == 1) sie2 |= (1u << 6); // PREAMBLE_EN
            *((volatile uint32_t*)&USB.SIE_CTRL) = sie2;
            asm volatile("dsb" ::: "memory"); // memory barrier instead of empty loop
            *((volatile uint32_t*)&USB.SIE_CTRL) = sie2 | (1u << 0); // START_TRANS
        } else {
            // DATA phase done
            printf("IRQ: DATA phase done\n");
            USB.INT_EP_CTRL = usb_hcd::inst()._saved_int_ep_ctrl;
            if (usb_hcd::inst()._xfer_complete_cb) {
                hcd_xfer_result_t result;
                result.success    = true;
                result.actual_len = USB_DPRAM.EP1_IN_BUFFER_CONTROL.LENGTH_0;
                printf("IRQ: actual_len=%d\n", result.actual_len);
                if (usb_hcd::inst()._xfer_buffer && result.actual_len > 0) {
                    volatile uint8_t * s = EPX_BUFFER;
                    uint8_t * d = usb_hcd::inst()._xfer_buffer;
                    for (uint16_t i = 0; i < result.actual_len && i < 64; i++) d[i] = s[i];
                }
                usb_hcd::inst()._xfer_complete_cb(result);
            }
        }
    }

    if (ints & (0x20u | 0x40u | 0x200u)) { // ERROR bits
        printf("IRQ: ERROR INTS=0x%08lx\n", ints);
        *((volatile uint32_t*)&USB_CLR.SIE_STATUS) = 0xFFFFFFFFu;
        USB.INT_EP_CTRL = usb_hcd::inst()._saved_int_ep_ctrl;
        if (usb_hcd::inst()._xfer_complete_cb) {
            hcd_xfer_result_t result;
            result.success    = false;
            result.actual_len = 0;
            usb_hcd::inst()._xfer_complete_cb(result);
        }
    }

    if (ints & 0x10u) { // BUFF_STATUS
        uint32_t buffs = USB.BUFF_STATUS;
        printf("IRQ: BUFF_STATUS=0x%08lx\n", buffs);
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
