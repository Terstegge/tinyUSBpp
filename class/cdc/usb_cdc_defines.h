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
#ifndef TUPP_USB_CDC_DEFINES_H
#define TUPP_USB_CDC_DEFINES_H

//////////////////////////////
// CDC class specific requests
//////////////////////////////
#define CDC_REQUESTS \
    REQ_CDC_SEND_ENCAPSULATED_COMMAND   = 0x00,    \
    REQ_CDC_GET_ENCAPSULATED_RESPONSE   = 0x01,    \
    REQ_CDC_SET_COMM_FEATURE            = 0x02,    \
    REQ_CDC_GET_COMM_FEATURE            = 0x03,    \
    REQ_CDC_CLEAR_COMM_FEATURE          = 0x04,    \
                                                   \
    REQ_CDC_SET_AUX_LINE_STATE          = 0x10,    \
    REQ_CDC_SET_HOOK_STATE              = 0x11,    \
    REQ_CDC_PULSE_SETUP                 = 0x12,    \
    REQ_CDC_SEND_PULSE                  = 0x13,    \
    REQ_CDC_SET_PULSE_TIME              = 0x14,    \
    REQ_CDC_RING_AUX_JACK               = 0x15,    \
                                                   \
    REQ_CDC_SET_LINE_CODING             = 0x20,    \
    REQ_CDC_GET_LINE_CODING             = 0x21,    \
    REQ_CDC_SET_CONTROL_LINE_STATE      = 0x22,    \
    REQ_CDC_SEND_BREAK                  = 0x23,    \
                                                   \
    REQ_CDC_SET_RINGER_PARMS            = 0x30,    \
    REQ_CDC_GET_RINGER_PARMS            = 0x31,    \
    REQ_CDC_SET_OPERATION_PARMS         = 0x32,    \
    REQ_CDC_GET_OPERATION_PARMS         = 0x33,    \
    REQ_CDC_SET_LINE_PARMS              = 0x34,    \
    REQ_CDC_GET_LINE_PARMS              = 0x35,    \
    REQ_CDC_DIAL_DIGITS                 = 0x36,    \
    REQ_CDC_SET_UNIT_PARAMETER          = 0x37,    \
    REQ_CDC_GET_UNIT_PARAMETER          = 0x38,    \
    REQ_CDC_CLEAR_UNIT_PARAMETER        = 0x39,    \
    REQ_CDC_GET_PROFILE                 = 0x3A

////////////////////////////////////
// CDC class specific device classes
////////////////////////////////////
#define CDC_DEVICE_CLASSES \
    DEV_CLASS_CDC   = 0x02

///////////////////////////////////////
// CDC class specific interface classes
///////////////////////////////////////
#define CDC_INTERFACE_CLASSES \
    IF_CLASS_CDC         = 0x02, \
    IF_CLASS_CDC_DATA    = 0x0a

//////////////////////////////////////////
// CDC class specific interface subclasses
//////////////////////////////////////////
#define CDC_INTERFACE_SUBCLASSES \
    IF_SUBCLASS_DIRECT_LINE_CONTROL_MODEL         = 1,    \
    IF_SUBCLASS_ABSTRACT_CONTROL_MODEL            = 2,    \
    IF_SUBCLASS_TELEPHONE_CONTROL_MODEL           = 3,    \
    IF_SUBCLASS_MULTICHANNEL_CONTROL_MODEL        = 4,    \
    IF_SUBCLASS_CAPI_CONTROL_MODEL                = 5,    \
    IF_SUBCLASS_ETHERNET_CONTROL_MODEL            = 6,    \
    IF_SUBCLASS_ATM_NETWORKING_CONTROL_MODEL      = 7,    \
    IF_SUBCLASS_WIRELESS_HANDSET_CONTROL_MODEL    = 8,    \
    IF_SUBCLASS_DEVICE_MANAGEMENT                 = 9,    \
    IF_SUBCLASS_MOBILE_DIRECT_LINE_MODEL          = 10,   \
    IF_SUBCLASS_OBEX                              = 11,   \
    IF_SUBCLASS_ETHERNET_EMULATION_MODEL          = 12,   \
    IF_SUBCLASS_NETWORK_CONTROL_MODEL             = 13

#define CDC_INTERFACE_PROTOCOLS \
    IF_PROTOCOL_CDC_COMM_AT                                 = 0x01, \
    IF_PROTOCOL_CDC_COMM_PROTOCOL_AT_PCCA_101               = 0x02, \
    IF_PROTOCOL_CDC_COMM_PROTOCOL_AT_PCCA_101_AND_ANNEXO    = 0x03, \
    IF_PROTOCOL_CDC_COMM_PROTOCOL_AT_GSM_707                = 0x04, \
    IF_PROTOCOL_CDC_COMM_PROTOCOL_AT_3GPP_27007             = 0x05, \
    IF_PROTOCOL_CDC_COMM_PROTOCOL_AT_CDMA                   = 0x06, \
    IF_PROTOCOL_CDC_COMM_PROTOCOL_ETHERNET_EMULATION_MODEL  = 0x07, \
                                                                    \
    IF_PROTOCOL_CDC_DATA_PROTOCOL_ISDN_BRI                  = 0x30, \
    IF_PROTOCOL_CDC_DATA_PROTOCOL_HDLC                      = 0x31, \
    IF_PROTOCOL_CDC_DATA_PROTOCOL_TRANSPARENT               = 0x32, \
    IF_PROTOCOL_CDC_DATA_PROTOCOL_Q921_MANAGEMENT           = 0x50, \
    IF_PROTOCOL_CDC_DATA_PROTOCOL_Q921_DATA_LINK            = 0x51, \
    IF_PROTOCOL_CDC_DATA_PROTOCOL_Q921_TEI_MULTIPLEXOR      = 0x52, \
    IF_PROTOCOL_CDC_DATA_PROTOCOL_V42BIS_DATA_COMPRESSION   = 0x90, \
    IF_PROTOCOL_CDC_DATA_PROTOCOL_EURO_ISDN                 = 0x91, \
    IF_PROTOCOL_CDC_DATA_PROTOCOL_V24_RATE_ADAPTION_TO_ISDN = 0x92, \
    IF_PROTOCOL_CDC_DATA_PROTOCOL_CAPI_COMMAND              = 0x93, \
    IF_PROTOCOL_CDC_DATA_PROTOCOL_HOST_BASED_DRIVER         = 0xFD

#endif  // TUPP_USB_CDC_DEFINES_H
