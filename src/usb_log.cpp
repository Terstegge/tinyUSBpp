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
// This is a simple logging class to be used within tinyUSB++.
// The user interface are the two macros defined below:
//
#include "usb_log.h"
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cstdlib>

usb_log usb_log::inst;

void usb_log::print(const char *file, int line, log_level l, const char *fmt, ...) {
    char *buffer = _buffer;
    const char *p;
    // Check the level..
    if (l > usb_log::_level) return;
    // Add level string
    strcpy(buffer, _level_str[l]);
    buffer += strlen(buffer);
    // Add user message
    va_list args;
    va_start(args, fmt);
    for (p = fmt; *p; ++p) {
        if (*p != '%') {
            strncat(buffer, p, 1);
        } else {
            switch (*++p) {
                /* string */
                case 's': {
                    strcpy(buffer, "\"");
                    strcat(buffer, va_arg(args, char *));
                    strcat(buffer, "\"");
                    break;
                }
                // integer base 10
                case 'd': {
                    itoa(va_arg(args, int), buffer, 10);
                    break;
                }
                // integer base 16
                case 'x': {
                    itoa(va_arg(args, int), buffer, 16);
                    break;
                }
                // boolean
                case 'b': {
                    if(va_arg(args, int)) {
                        strcpy(buffer, "true");
                    } else {
                        strcpy(buffer, "false");
                    }
                    break;
                }
                default:
                    strncat(buffer, p, 1);
            }
        }
        buffer += strlen(buffer);
    }
    va_end  (args);
    // Add line and filename
    strcat(buffer, " (");
    strcat(buffer, file);
    strcat(buffer, ":");
    buffer += strlen(buffer);
    itoa(line, buffer, 10);
    strcat(buffer, ")");
    // Finally print message
    puts(_buffer);
}
