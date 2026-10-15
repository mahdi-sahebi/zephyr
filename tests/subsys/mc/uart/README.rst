MC UART integration test
========================

This test keeps the in-memory ``mc_comm`` and ``mc_msg`` tests separate and
validates the same APIs over a physical UART between two nRF54L15 DKs.

Connect the boards as follows::

   Board A TX -> Board B RX
   Board A RX <- Board B TX
   Board A GND -- Board B GND

UART30 is used for the protocol and UART20 remains the console UART. Verify
the UART30 pin routing on the boards before connecting them.

Build the sender on Board A::

   west build -b nrf54l15dk/nrf54l15/cpuapp \
     tests/subsys/mc/uart -d build/mc-uart-sender --pristine=always
   west flash -d build/mc-uart-sender

Build the receiver on Board B::

   west build -b nrf54l15dk/nrf54l15/cpuapp \
     tests/subsys/mc/uart -d build/mc-uart-receiver \
     --pristine=always -DCONFIG_MC_UART_SENDER=n
   west flash -d build/mc-uart-receiver

Open both console UARTs at 115200 baud. Both boards should report a PASS
message. The sender transmits a normal message and a zero-payload signal; the
receiver validates both through ``mc_msg``.
