# OpenPAYGO Link STM8S examples
This directory contains a set of basic usage examples of the OpenPAYGO Link package on a STM8SX03 microcontroller. The examples use a minimal custom Hardware Abstraction Layer and require no external libraries. 

Due to the very limited resources of this MCU, these examples are meant to showcase the core features of OpenPAYGO Link used with Nexus Channel Core and provide a basis for people wanting to implement simple use cases into their devices at a low cost. 

## Target
The STM8SX03 is an inexpensive microcontroller with 8kB of flash and 1kB or RAM and a maximum clock speed of 16MHz (in all the examples the clock is set to the default speed of 2MHz). Either the STM8S003F3P6 or the STM8S103F3P6 can be used, the only difference between them being that the first one supports less flash write cycles. There are development boards that include these microcontrollers like the STM8S003 Discovery board from ST or the [STM8S003/103](https://tenbaht.github.io/sduino/hardware/stm8blue/) "blue pill" boards.

## Schematic
Although the provided schematic is for the STMSX03 targets, the connections between the LIN transceiver and any other microcontroller should be equivalent.
![Slave Board Schematic](https://github.com/EnAccess/OpenPAYGO-Link/raw/main/Examples/STM8S003/schematic.png)

## Build/Flash
The projects are built using Make and [Small Device C Compiler](http://sdcc.sourceforge.net/) (SDCC). This compiler supports different targets including a couple of STM8S variants, which need to be defined in the MCU field of the *Makefile.include* file. Either "stm8s003f3" or "stm8s103f3" will work.

It is necessary to clean with before compiling the project for a new/different node (master or slave), otherwise the build will fail. After compiling *main.hex* will be created in the same directory as *main.c*. This file can be used to flash the target.

There are different programming CLI tools available: [ST Visual Programmer](https://www.st.com/en/development-tools/stvp-stm8.html) (STVP) for Windows (which also comes with a GUI) and [stm8flash](https://github.com/vdudouyt/stm8flash) for Linux/MacOS.

## TODO
* Update Makefile.include to add support for Linux/MacOS commands and tools.
