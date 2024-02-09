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
#ifndef TUPP_USB_MSC_DEFINES_H
#define TUPP_USB_MSC_DEFINES_H

//////////////////////////////
// MSC class specific requests
//////////////////////////////
#define MSC_REQUESTS \
    REQ_MSC_ADSC            = 0x00,    \
    REQ_MSC_GET_REQUEST     = 0xfc,    \
    REQ_MSC_PUT_REQUEST     = 0xfd,    \
    REQ_MSC_GET_MAX_LUN     = 0xfe,    \
    REQ_MSC_BOT_RESET       = 0xff

///////////////////////////////////////
// MSC class specific interface classes
///////////////////////////////////////
#define MSC_INTERFACE_CLASSES \
    IF_CLASS_MSC    = 0x08

//////////////////////////////////////////
// MSC class specific interface subclasses
//////////////////////////////////////////
#define MSC_INTERFACE_SUBCLASSES \
    IF_SUBCLASS_SCSI_NOT_REPORTED   = 0,    \
    IF_SUBCLASS_RBC                 = 1,    \
    IF_SUBCLASS_MMC_5               = 2,    \
    IF_SUBCLASS_UIF                 = 4,    \
    IF_SUBCLASS_SCSI_TRANSPARENT    = 6,    \
    IF_SUBCLASS_LSD_FD              = 7,    \
    IF_SUBCLASS_IEEE_1667           = 8

#define MSC_INTERFACE_PROTOCOLS \
    IF_PROTOCOL_MSC_CBI_COMP_IRQ    = 0x00, \
    IF_PROTOCOL_MSC_CBI_NO_COMP_IRQ = 0x01, \
    IF_PROTOCOL_MSC_BOT             = 0x50, \
    IF_PROTOCOL_MSC_UAS             = 0x62

#endif  // TUPP_USB_MSC_DEFINES_H
