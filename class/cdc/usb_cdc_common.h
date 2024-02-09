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
// (c) 2024 A. Terstegge  (Andreas.Terstegge@gmail.com)
//
#ifndef TUPP_USB_CDC_COMMON_H_
#define TUPP_USB_CDC_COMMON_H_

#include "usb_common.h"

namespace USB::CDC {
    //////////////////////////////////
    // Functional Descriptor base type
    //////////////////////////////////
    enum class func_desc_type_t : uint8_t {
        CS_INTERFACE    = 0x24,
        CS_ENDPOINT     = 0x25
    };

    enum class func_desc_subtype_t : uint8_t {
        CDC_FUNC_DESC_HEADER                                    = 0x00,
        CDC_FUNC_DESC_CALL_MANAGEMENT                           = 0x01,
        CDC_FUNC_DESC_ABSTRACT_CONTROL_MANAGEMENT               = 0x02,
        CDC_FUNC_DESC_DIRECT_LINE_MANAGEMENT                    = 0x03,
        CDC_FUNC_DESC_TELEPHONE_RINGER                          = 0x04,
        CDC_FUNC_DESC_TELEPHONE_CALL_AND_LINE_STATE_REPORTING   = 0x05,
        CDC_FUNC_DESC_UNION                                     = 0x06,
        CDC_FUNC_DESC_COUNTRY_SELECTION                         = 0x07,
        CDC_FUNC_DESC_TELEPHONE_OPERATIONAL_MODES               = 0x08,
        CDC_FUNC_DESC_USB_TERMINAL                              = 0x09,
        CDC_FUNC_DESC_NETWORK_CHANNEL_TERMINAL                  = 0x0A,
        CDC_FUNC_DESC_PROTOCOL_UNIT                             = 0x0B,
        CDC_FUNC_DESC_EXTENSION_UNIT                            = 0x0C,
        CDC_FUNC_DESC_MULTICHANNEL_MANAGEMENT                   = 0x0D,
        CDC_FUNC_DESC_CAPI_CONTROL_MANAGEMENT                   = 0x0E,
        CDC_FUNC_DESC_ETHERNET_NETWORKING                       = 0x0F,
        CDC_FUNC_DESC_ATM_NETWORKING                            = 0x10,
        CDC_FUNC_DESC_WIRELESS_HANDSET_CONTROL_MODEL            = 0x11,
        CDC_FUNC_DESC_MOBILE_DIRECT_LINE_MODEL                  = 0x12,
        CDC_FUNC_DESC_MOBILE_DIRECT_LINE_MODEL_DETAIL           = 0x13,
        CDC_FUNC_DESC_DEVICE_MANAGEMENT_MODEL                   = 0x14,
        CDC_FUNC_DESC_OBEX                                      = 0x15,
        CDC_FUNC_DESC_COMMAND_SET                               = 0x16,
        CDC_FUNC_DESC_COMMAND_SET_DETAIL                        = 0x17,
        CDC_FUNC_DESC_TELEPHONE_CONTROL_MODEL                   = 0x18,
        CDC_FUNC_DESC_OBEX_SERVICE_IDENTIFIER                   = 0x19,
        CDC_FUNC_DESC_NCM                                       = 0x1A
    };

    struct __attribute__((__packed__)) func_device_descriptor_t {
        uint8_t             bLength {0};
        func_desc_type_t    bDescriptorType {0};
        func_desc_subtype_t bDescriptorSubType {0};
    };
    static_assert(sizeof(func_device_descriptor_t) == 3);

    ///////////////////////////////
    // Header Functional Descriptor
    ///////////////////////////////
    struct __attribute__((__packed__)) func_desc_header_t : public func_device_descriptor_t {
        uint16_t            bcdCDC {0};
    };
    static_assert(sizeof(func_desc_header_t) == 5);

    //////////////////////////////
    // Union Functional Descriptor
    //////////////////////////////
    template<int N>
    struct __attribute__((__packed__)) func_desc_union_t : public func_device_descriptor_t {
        uint8_t             bControlInterface {0};
        uint8_t             bSubordinateInterface[N] {0};
    };
    static_assert(sizeof(func_desc_union_t<1>) == 5);

    //////////////////////////////////////////
    // Country Selection Functional Descriptor
    //////////////////////////////////////////
    struct __attribute__((__packed__)) func_desc_country_selector_t : public func_device_descriptor_t {
        uint8_t             iCountryCodeReleaseDate {0};
        uint16_t            wCountryCode0 {0};
    };
    static_assert(sizeof(func_desc_country_selector_t) == 6);

    /////////////////////////////////////////////
    // Call Management (CM) Functional Descriptor
    /////////////////////////////////////////////
    struct __attribute__((__packed__)) bmCmCapabilities_t {
        uint8_t     handle_call_management          : 1 {0};
        uint8_t     call_management_via_data_class  : 1 {0};
        uint8_t     reserved                        : 6 {0};
    };

    struct __attribute__((__packed__)) func_desc_cm_t : public func_device_descriptor_t {
        bmCmCapabilities_t  bmCapabilities {0};
        uint8_t             bDataInterface {0};
    };
    static_assert(sizeof(func_desc_cm_t) == 5);

    //////////////////////////////////////////////////////////
    // Abstract Control Management (ACM) Functional Descriptor
    //////////////////////////////////////////////////////////
    struct __attribute__((__packed__)) bmAcmCapabilities_t {
        uint8_t     comm_feature_support        : 1 {0};
        uint8_t     line_coding_support         : 1 {0};
        uint8_t     send_break_support          : 1 {0};
        uint8_t     network_connection_support  : 1 {0};
        uint8_t     reserved                    : 4 {0};
    };

    struct __attribute__((__packed__)) func_desc_acm_t : public func_device_descriptor_t {
        bmAcmCapabilities_t bmCapabilities;
    };
    static_assert(sizeof(func_desc_acm_t) == 4);

    /////////////////////////////////////////////////////
    // Direct Line Management (DLM) Functional Descriptor
    /////////////////////////////////////////////////////
    struct __attribute__((__packed__)) bmDlmCapabilities_t {
        uint8_t pulse_support       : 1 {0};
        uint8_t aux_support         : 1 {0};
        uint8_t extra_pulse_setup   : 1 {0};
        uint8_t reserved            : 5 {0};
    };

    struct __attribute__((__packed__)) func_desc_direct_line_t  : public func_device_descriptor_t {
        bmDlmCapabilities_t bmDlmCapabilities;
    };
    static_assert(sizeof(func_desc_direct_line_t) == 4);

    ////////////////////////
    // Line coding structure
    ////////////////////////
    enum class bCharFormat_t : uint8_t {
        STOP_BITS_1     = 0,
        STOP_BITS_15    = 1,
        STOP_BITS_2     = 2
    };

    enum class bParityType_t : uint8_t {
        PARITY_NONE        = 0,
        PARITY_ODD         = 1,
        PARITY_EVEN        = 2,
        PARITY_MARK        = 3,
        PARITY_SPACE       = 4
    };

    struct __attribute__((__packed__)) line_coding_t {
        uint32_t        dwDTERate  { 9600 };
        bCharFormat_t   bCharFormat{ bCharFormat_t::STOP_BITS_1 };
        bParityType_t   bParityType{ bParityType_t::PARITY_NONE };
        uint8_t         bDataBits  { 8 };
    };
    static_assert(sizeof(line_coding_t) == 7);

    ///////////////////////////////////
    // CDC class specific notifications
    ///////////////////////////////////
    enum class bNotification_t : uint8_t  {
        NOTIF_NETWORK_CONNECTION    = 0x00,
        NOTIF_RESPONSE_AVAILABLE    = 0x01,
        NOTIF_AUX_JACK_HOOK_STATE   = 0x08,
        NOTIF_RING_DETECT           = 0x09,
        NOTIF_SERIAL_STATE          = 0x20,
        NOTIF_CALL_STATE_CHANGE     = 0x28,
        NOTIF_LINE_STATE_CHANGE     = 0x23
    };

    struct __attribute__((__packed__)) notification_t {
        recipient_t         recipient : 5 {0};
        type_t              type      : 2 {0};
        direction_t         direction : 1 {0};
        bNotification_t     bNotification {0};
        uint16_t            wValue  {0};
        uint16_t            wIndex  {0};
        uint16_t            wLength {0};
    };
    static_assert(sizeof(notification_t) == 8);

    ///////////////////////////////
    // Serial State notification //
    ///////////////////////////////
    struct __attribute__((__packed__))  bmUartState_t{
        uint16_t    bRxCarrier_DCD  : 1 {0};
        uint16_t    bTxCarrier_DSR  : 1 {0};
        uint16_t    bBreak          : 1 {0};
        uint16_t    bRingSignal     : 1 {0};
        uint16_t    bFraming        : 1 {0};
        uint16_t    bParity         : 1 {0};
        uint16_t    bOverRun        : 1 {0};
        uint16_t    reserved        : 9 {0};
    };

    struct __attribute__((__packed__)) notif_serial_state_t : public notification_t {
        notif_serial_state_t() {
            // Set all known values in base class
            direction       = direction_t::DIR_IN;
            type            = type_t::TYPE_CLASS;
            recipient       = recipient_t::REC_INTERFACE;
            bNotification   = bNotification_t::NOTIF_SERIAL_STATE;
            wValue          = 0;
            wIndex          = 0;
            wLength         = 2;
        }
        // Notification specific attributes
        bmUartState_t   bmUartState;
    };
    static_assert(sizeof(notif_serial_state_t) == 10);

}   // namespace USB::CDC

#endif  // TUPP_USB_CDC_COMMON_H_
