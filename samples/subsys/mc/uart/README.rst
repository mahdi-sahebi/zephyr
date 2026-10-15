MC UART transport
=================

This sample exercises ``mc_comm`` and ``mc_msg`` over a physical UART between
two nRF54L15 DKs. It sends a message and a zero-payload signal, then reports
``MC UART: PASS``.

Do not use the console UART for the protocol link. The board overlay selects
UART30 for the protocol and leaves UART20 available for the console. Verify
the UART30 pin routing on the DK before connecting the boards.

Connect the boards as follows::

   Board A TX -> Board B RX
   Board A RX <- Board B TX
   Board A GND -- Board B GND

Build the sender with the default configuration::

   west build -b nrf54l15dk/nrf54l15/cpuapp \
     samples/subsys/mc/uart -d build/mc-uart-sender --pristine=always

Build the receiver with ``MC_UART_SENDER`` disabled::

   west build -b nrf54l15dk/nrf54l15/cpuapp \
     samples/subsys/mc/uart -d build/mc-uart-receiver \
     --pristine=always -DCONFIG_MC_UART_SENDER=n

Flash each build to its corresponding board with ``west flash``. Open both
console UARTs at 115200 baud. The sender prints ``MC UART: PASS`` after the
message and signal are transmitted; the receiver prints it after receiving
the message.
