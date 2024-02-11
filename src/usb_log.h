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
//   TUPP_LOG(...)  to print a logging message on stdout, and
//   TUPP_LOG_LEVEL(l) to set the logging level (see below).
//
#ifndef TUPP_LOG_H
#define TUPP_LOG_H

enum log_level { LOG_OFF=0, LOG_ERROR=1, LOG_WARNING=2, LOG_INFO=3, LOG_DEBUG=4 };

#define MAX_LINE_LENGTH 200

#ifdef NDEBUG
#define TUPP_LOG(...)
#define TUPP_LOG_LEVEL(level)
#else
#define TUPP_LOG(...)         usb_log::inst.print(__FILE_NAME__, __LINE__, __VA_ARGS__)
#define TUPP_LOG_LEVEL(level) usb_log::inst.setLevel(level)
#endif

class usb_log {
public:
    // The logger is a singleton
    static usb_log inst;
    // Print a single log line. The format specifier
    // is similar to the standard printf function.
    // only %s (string), %d (integer) %x (hex integer)
    // and %b (boolean) are allowed, without a width
    // specification. A "\n" will be added to the output
    // string automatically.
    void print(const char *file, int line, log_level l, const char *fmt, ...);
    // Set the log level
    inline void setLevel(log_level l) { _level = l; }

private:
    // No public access to CTOR
    usb_log() : _level {log_level::LOG_OFF} {}
    // The current log level
    log_level _level;
    char _buffer[MAX_LINE_LENGTH];
    const char * _level_str[5] = {"[OFF] ", "[ERR] ", "[WAR] ", "[INF] ", "[DBG] "};
};

#endif // TUPP_LOG_H
