MC UART integration test
========================

This test keeps the in-memory ``mc_comm`` and ``mc_msg`` tests separate and
validates the same APIs over a physical UART between two nRF54L15 DKs.

Connect the boards as follows::

   Board A TX -> Board B RX
   Board A RX <- Board B TX
   Board A GND -- Board B GND

UART20 (Serial Port 1) is used for the protocol and UART30 (Serial Port 0)
remains the console UART. Connect P1.04/TXD1 to the other board's P1.05/RXD1,
P1.05/RXD1 to the other board's P1.04/TXD1, and connect GND to GND.

Build the sender on Board A::

   west build -b nrf54l15dk/nrf54l15/cpuapp \
     tests/subsys/mc/uart -d build/mc-uart-sender --pristine=always \
     -- -DCONF_FILE=prj_sender.conf
   west flash -d build/mc-uart-sender

Build the receiver on Board B::

   west build -b nrf54l15dk/nrf54l15/cpuapp \
     tests/subsys/mc/uart -d build/mc-uart-receiver --pristine=always \
     -- -DCONF_FILE=prj_receiver.conf
   west flash -d build/mc-uart-receiver

Open both console UARTs at 115200 baud. Both boards should report a PASS
message. The sender transmits a normal message and a zero-payload signal; the
receiver validates both through ``mc_msg``.
