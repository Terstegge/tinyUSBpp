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
// Template implementation of a generic FIFO with fixed
// size. The FIFO data type and size are the template parameters.
//
#ifndef TUPP_FIFO_H
#define TUPP_FIFO_H

template<typename T, int SIZE>
class fifo {
public:
    fifo()
    : _need_wrap(_buffer + SIZE),
      _get_ptr(_buffer),
      _put_ptr(_buffer) {}

    bool get(T & data) volatile {
        if (_get_ptr == _put_ptr) {
            return false;
        }
        volatile T * nextget =  _get_ptr + 1;
        if (nextget == _need_wrap) {
            nextget =  _buffer;
        }
        data = *_get_ptr;
        _get_ptr = nextget;
        return true;
    }

    bool put(const T & data) volatile {
        volatile T * nextput =  _put_ptr + 1;
        if (nextput == _need_wrap){
            nextput =  _buffer;
        }
        if (nextput == _get_ptr) {
            return false;
        } else {
            *_put_ptr = data;
             _put_ptr = nextput;
        }
        return true;
    }

    int available_get() volatile {
        int res = _put_ptr - _get_ptr;
        if (res < 0) {
            res += SIZE;
        }
        return res;
    }

    int available_put() volatile {
        return SIZE - available_get() - 1;
    }

    void clear() {
        _get_ptr = _buffer;
        _put_ptr = _buffer;
    }

private:

    volatile T   _buffer[SIZE] {};
    volatile T * _need_wrap;
    volatile T * _get_ptr;
    volatile T * _put_ptr;
};

#endif // TUPP_FIFO_H
