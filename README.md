# OpenPAYGO Link

**Beta release, don't use in production**

Open PAYGO Link (OPLink) is a communication protocol created as an open source standard solution for interfacing Solar Home Systems (SHS) with appliances and accessories. It is defined at hardware and software levels and features a half-duplex communication over a single wire with a multi-drop connection topology. It ensures reliable data transmission, even with different ground potentials between nodes, thanks to an off the shelf transceiver. Although it is not intended to transmit long frames it supports any variable-length payload. Check the compatible targets below.

## What is implemented
* The network uses a master/slave configuration.
* Two kinds of messages are available: request/reply and broadcast.
* The master can automatically discover new slaves and assign them a local address.
* Slave nodes can instantly detect that they have been disconnected, the master requires polling.
* The library API provides a unique ID list of the connected nodes. The mapping from UID to local address is done internally.
* A node can only handle one request at a time.
* Peer-to-peer communication is not supported in this version.
* A custom light version of the OCF standard is included (with tiny CoAP and CBOR libraries).

## What is going to change soon
* The CSMA/CA mechanism will be refined.
* All the messages will require acknowledgment, even broadcasts.
* The network will have a manager and workers instead of a master and slaves.
* Peer-to-peer communication will be supported.

## Plans for the future
* Any node (that supports the feature) could take the role of the manager.
* Allow network merging.

## Compatible MCUs

In order to implement the OpenPAYGO Link in a specific MCU it needs to support 9-bit multiprocessor UART with address mode wake up. Some compatible MCUs are listed below:

* STM8
Supports 8-9 bit address mode with 4 bit addresses
[Reference manual](https://www.st.com/content/ccc/resource/technical/document/reference_manual/9a/1b/85/07/ca/eb/4f/dd/CD00190271.pdf/files/CD00190271.pdf/jcr:content/translations/en.CD00190271.pdf), check _22.3.7 Multi-processor communication_
* STM32
Supports 8-9 bit address mode with 4 bit addresses
[Reference manual](https://www.st.com/resource/en/reference_manual/cd00171190-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf), check _27.3.6 Multiprocessor communication_
* PIC18
Supports only 9 bit address mode
[Datasheet](http://ww1.microchip.com/downloads/en/DeviceDoc/PIC18LF26-27-45-46-47-55-56-57K42-Data-Sheet-40001919C.pdf), check _31.3 Asynchronous Address Mode_
* MSP430
Supports 9 bit address mode
[User guide](https://www.ti.com/lit/ug/slau144j/slau144j.pdf?ts=1588848816104), check _15.3.3.3 Address-Bit Multiprocessor Format_ & _15.3.2 Character Format_
* ATmega382
Supports 8-9 bit address mode
[Datasheet](http://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf), check _19.9 Multi-processor Communication Mode_
* NXP
Supports 9 bit address mode
[Datasheet](https://www.nxp.com/docs/en/data-sheet/LPC802.pdf), check _9.12 USART0/1_
* Nordic (nRF24E1 example)
Supports 9 bit transmission
[Datasheet](http://www.keil.com/dd/docs/datashts/nordic/nrf24e1.pdf), check _10.9 Serial Interface_ & _10.9.5 Multiprocessor Communications_
* Ranesas (RX130 example)
[Hardware manual](https://www.renesas.com/us/en/doc/products/mpumcu/rx100/r01uh0560ej0300-rx130.pdf?key=76ac3ae182fd3686a9ba068a7f3a03de), check _27.4 Multi-Processor Communications Function_
* Maxim Integrated
Supports 9 bit address mode
[User guide](https://pdfserv.maximintegrated.com/en/an/AN6242.pdf), check _7.5.8 Multidrop Mode Support_
* Espressif (ESP8266 & ESP32)
No hardware 9 bit transmission support but there are software serial libraries
[9 bit Software Serial library](https://github.com/ionini/espsoftwareserial)
* Raspberry
Hardware supports 9 bit but official Kernel does not (?)
[9 bit patch](https://patchwork.kernel.org/patch/8498531/), apparently the patch
is implemented already: [tty driver](https://github.com/raspberrypi/linux/blob/rpi-4.19.y/drivers/tty/serial/amba-pl011.c)