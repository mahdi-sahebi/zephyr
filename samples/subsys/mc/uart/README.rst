MC UART transport
=================

This sample exercises ``mc_comm`` and ``mc_msg`` over a physical UART between
two nRF54L15 DKs. It sends a message and a zero-payload signal, then reports
``MC UART: PASS``.

See ``setup.md`` for the complete two-board setup, assigned probe IDs, wiring,
build/flash helper, console mapping, and troubleshooting procedure.

This sample intentionally has no default ``prj.conf``. Select exactly one of
``prj_sender.conf`` or ``prj_receiver.conf`` explicitly for every build.

Do not use the console UART for the protocol link. The board overlay selects
UART20/Serial Port 1 on P1.04 TXD1 and P1.05 RXD1 for the protocol. It moves
the console to UART30/Serial Port 0 on P0.00/P0.01, available through VCOM0.
Do not open VCOM1 while testing because it is connected to the protocol pins.

Connect the boards as follows::

   Board A P1.04 TXD1 -> Board B P1.05 RXD1
   Board A P1.05 RXD1 <- Board B P1.04 TXD1
   Board A GND -- Board B GND

Build the sender with the sender configuration::

   west build -b nrf54l15dk/nrf54l15/cpuapp \
     samples/subsys/mc/uart -d build/mc-uart-sender --pristine=always \
     -- -DCONF_FILE=prj_sender.conf

Build the receiver with the receiver configuration::

   west build -b nrf54l15dk/nrf54l15/cpuapp \
     samples/subsys/mc/uart -d build/mc-uart-receiver \
     --pristine=always -- -DCONF_FILE=prj_receiver.conf

Flash each build to its corresponding board with ``west flash``. Open both
console UARTs at 115200 baud. The sender prints ``MC UART: PASS`` after the
message and signal are transmitted; the receiver prints it after receiving
the message.
