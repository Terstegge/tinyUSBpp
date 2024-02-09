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
#include "usb_ms_compatible_ID.h"
#include "usb_ms_func_subset.h"
//#include "usb_ms_registry_property.h"
//#include "usb_strings.h"


usb_ms_compatible_ID::usb_ms_compatible_ID(usb_ms_func_subset & func_subset)
: descriptor{_descriptor}, _func_subset{func_subset}
{
    // Set header values
    _descriptor.wLength             = sizeof(USB::ms_compat_id_header_t);
    _descriptor.wDescriptorType     = USB::wDescriptorType_t::DESC_COMPAT_ID;
    _func_subset.add_compatible_ID(this);
}

