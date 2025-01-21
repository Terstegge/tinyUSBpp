# tinyUSB++
Small and easy to use USB library written in C++

tinyUSB++ was inspired by the famous tinyusb library, but tries do
do a couple of things better. Just like tinyusb, tinyUSB++ is an
open-source cross-platform USB Host/Device stack for embedded system,
designed to be memory-safe (with no dynamic allocation) and performant.

Still there are some relevant differences:

* tinyUSB++ is written in C++, which offers more powerfull
  programming paradigms (inheritance, virtual methods, lambdas, ...).
  The source structure consists of several smaller classes, each implementing
  a specific part of the USB descriptor tree or USB functionality.

* tinyUSB++ does not include drivers for LEDs or any other kind of OS
  integration. Instead, tinyUSB++ focuses on the USB functionality
  and leaves the decision how to integrate it into a (multitasking)
  RTOS to the user.

* tinyUSB++ does not include an additional layer like tinyusb, where
  all interrupt events are defered to a central queue, which is then
  handled by a non-IRQ task function. tinyUSB++ uses different strategies
  depending on the concrete driver class. A CDC device for example will
  store its data in FIFOs, which will then be accessed by the user code.
  A MSC device will have a handler function, which can then be called from
  a task/thread of the RTOS or simply in an endless loop. From the
  experience gained so far, this results in better performance.

* tinyUSB++ handles _all_ USB descriptor stuff internally, so adding new
  functionality to an existing program (e.g. one additional ACM device)
  is just on line of code. No time consuming manual preparation of the
  new USB descriptors is needed. This also means that tinyUSB++ will not
  store the USB descriptors in flash but in RAM. Since USB descriptors 
  are usually only a few hundret bytes in size, this should not be a 
  problem. But it gives the user the freedom to modify the USB descriptors
  during runtime.
 
## Examples

There are some examples for the RPi Pico (RP2040) and Pico 2 (R2350) to
demomstrate the CDC ACM and MSC functionality. Running these example is
quite straightforward:

* Create a build folder inside an example folder and change the
  working directory to it.
* Set your PICO_SDK_PATH or uncomment and change the related line in the
  CMakeLists.txt. Also select the pico board type.
* Run 'cmake ..' and 'make'
* Copy the generated UF2 file to the RPi Pico - thats it!

Please report any problems!

## TODOs

tinyUSB++ is far from complete and only a few weeks old. Only the CDC ACM
and MSC device functionality is implemented so far - all other USB classes like
HID or audio are currently missing. The host side is missing completely.
Still these missing parts will be hopefully added soon - driven by the
needs of the users. tinyUSB++ code tries to be modular and readable, so
hopefully extending the existing framework is not too complicated.
