MC UART loopback
================

Build for ``nrf54l15dk/nrf54l15/cpuapp`` and connect the selected UART TX pin
to RX on the same DK. Do not use the console UART for the protocol link.
The board-specific overlay must select the available non-console UART before
this sample is run on hardware.

This sample is build-only until the board's physical UART pins are selected
and verified on the target hardware.
