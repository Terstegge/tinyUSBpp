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
// (c) 2023 A. Terstegge  (Andreas.Terstegge@gmail.com)
//
#ifndef _TUPP_USB_CONFIG_H
#define _TUPP_USB_CONFIG_H

// Maximum number of strings in the complete
// device descriptor. One entry will be used
// for the language id.
#ifndef TUPP_MAX_STRINGS
#define TUPP_MAX_STRINGS 10
#endif

// Maximum descriptor size
#ifndef TUPP_MAX_DESC_SIZE
#define TUPP_MAX_DESC_SIZE 256
#endif

// Maximum number of USB configurations per USB device
#ifndef TUPP_MAX_CONF_PER_DEVICE
#define TUPP_MAX_CONF_PER_DEVICE 5
#endif

// Maximum number of USB interfaces per USB configuration
#ifndef TUPP_MAX_INTERF_PER_CONF
#define TUPP_MAX_INTERF_PER_CONF 5
#endif

// Maximum number of USB interface associations per USB configuration
#ifndef TUPP_MAX_ASSOC_PER_CONF
#define TUPP_MAX_ASSOC_PER_CONF 5
#endif

// Maximum number of USB endpoints per USB interface
#ifndef TUPP_MAX_EP_PER_INTERFACE
#define TUPP_MAX_EP_PER_INTERFACE 5
#endif

// Default packet size for USB endpoints
#ifndef TUPP_DEFAULT_PAKET_SIZE
#define TUPP_DEFAULT_PAKET_SIZE 64
#endif

// Default polling interval for USB endpoints
#ifndef TUPP_DEFAULT_POLL_INTERVAL
#define TUPP_DEFAULT_POLL_INTERVAL 10
#endif

// Default number of capabilities in BOS descriptor
#ifndef TUPP_MAX_BOS_CAPABILITIES
#define TUPP_MAX_BOS_CAPABILITIES 2
#endif

// Default size of a MS registry property descriptor
#ifndef TUPP_MS_REG_PROP_SIZE
#define TUPP_MS_REG_PROP_SIZE 200
#endif

// Default number of MS config subsets
#ifndef TUPP_MAX_MS_CONFIG_SUBSETS
#define TUPP_MAX_MS_CONFIG_SUBSETS 1
#endif

// Default number of MS config subsets
#ifndef TUPP_MAX_MS_FUNC_SUBSETS
#define TUPP_MAX_MS_FUNC_SUBSETS 1
#endif

// Default number of registry properties
#ifndef TUPP_MAX_MS_REG_PROP
#define TUPP_MAX_MS_REG_PROP 1
#endif

// Default size of CDC ACM FIFOs
#ifndef TUPP_CDC_ACM_FIFO_SIZE
#define TUPP_CDC_ACM_FIFO_SIZE 256
#endif

// Default block size for MSC devices
#ifndef TUPP_MSC_BLOCK_SIZE
#define TUPP_MSC_BLOCK_SIZE 512
#endif

#endif // _TUPP_USB_CONFIG_H
