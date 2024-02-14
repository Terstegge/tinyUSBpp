# tinyUSB++
Small and easy to use USB library written in C++

As the name implies, tinyUSB++ is closely related to the TinyUSB
project. As TinyUSB, tinyUSB++ tries to be an open-source cross-platform
USB Host/Device stack for embedded system, designed to be memory-safe
with no dynamic allocation.

Still there are some relevant differences:

* tinyUSB++ is written in C++, which offers more powerfull
  programming paradigms (inheritance, virtual methods, lambdas, ...).
  The source structure are several smaller classes, each implementing
  a specific part of the USB descriptors or USB functionality.

* tinyUSB++ does not include an additional layer like TinyUSB, where
  all interrupt events are defered to a central queue, which is then
  handled by a non-IRQ task function. tinyUSB++ uses differnent strategies
  depending on the concrete driver class. A CDC device for example will
  store its data in FIFOs, which will then be accessed by the user code.
  A MSC device will have a handler function, which can then be called from
  a task/thread of the RTOS or in an endless loop.

* tinyUSB++ handles _all_ USB descriptor stuff internally, so adding new
  functionality to an existing program (e.g. one additional ACM device)
  is just on line of code. No time consuming manual preparation of the
  new USB descriptors is needed. This also means that tinyUSB++ will not
  store the USB descriptors in flash but in RAM. Since USB descriptors 
  are usually only a few hundret bytes in size, this should not be a 
  problem. But it gives the user the freedom to modify the USB descriptors
  during runtime.
 