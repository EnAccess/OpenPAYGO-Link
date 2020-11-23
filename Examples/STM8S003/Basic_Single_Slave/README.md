# Basic single node example
This example demonstrates the basic communication functionality of an OpenPAYGO Link (v0.1) master and slave node pair. Both nodes are just used to transmit a plain ASCII text:
* Every 5 seconds the master node will send "OpenPAYGO".
* If the string is received correctly the slave node will blink its LED and reply with "Link".
* Finally the master node will also blink its LED it the received string was correct.

## Demonstration Steps
In order to test the correct operation of the node the following steps should be followed:

1. Power on the target node making sure it is not connected to any other board.
2. Connect the target to a computer with an ST-Link.
3. Open a terminal and navigate to the folder containing *main.c*.
4. Clean the project with `make clean`.
5. Build the project with `make build`.
6. Flash the project with `make flash`.

The node should be ready.

## Operation
It doesn't matter if the nodes are powered on or not before connecting the data bus, but it is **very important** that both nodes are attached to the same ground. At this point there are two alternatives that should work transparently to the user:
1. Power on the boards and then connect the LIN pins of the respective LIN transceivers.
2. Connect the LIN pins of the LIN transceivers together and then power on both boards (simultaneously or not).

*The worst case scenario is when all the nodes are powered at the same time, for example after a global power down, because there is a higher chance of collision on the bus. Nevertheless the firmware has been developed to be safe in this kind of situations.*

After the connection the expected behavior is:
1. The slave node will check if the bus is idle.
2. The slave node will notify the master that it was connected.
3. The master node will initiate the handshake and configure the slave node.

This takes a couple of milliseconds. Next the message exchange will start and the LEDs on both boards should start blinking every 5 seconds.
