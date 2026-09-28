# MC test application for the nRF54L15 DK

This repository contains a small hardware test application at
`samples/subsys/mc/nrf54l15dk_test`. It runs an in-memory loopback, so one
nRF54L15 DK is sufficient and no UART wires are required. It exercises
`mc_comm` framing and acknowledgements, `mc_msg` subscriptions, and
zero-length message signals.

## Build and flash

From the Zephyr repository directory, with the Zephyr SDK and `west` set up:

```sh
west build -b nrf54l15dk/nrf54l15/cpuapp \
  samples/subsys/mc/nrf54l15dk_test \
  -d build/mc-nrf54l15dk-test --pristine=always
west flash -d build/mc-nrf54l15dk-test
```

The board target is the application core: `nrf54l15dk/nrf54l15/cpuapp`.

## Observe the result

Open the DK USB CDC console at 115200 baud (the device name depends on the
host operating system):

```sh
screen /dev/ttyACM0 115200
```

A successful run prints:

```text
MC TEST: PASS
```

This application does not use the UART sample's `mc-uart` devicetree alias
and does not require a second board or external UART wiring.

## Build-only check

```sh
west build -b nrf54l15dk/nrf54l15/cpuapp \
  samples/subsys/mc/nrf54l15dk_test \
  -d build/mc-nrf54l15dk-test --pristine=always
```
