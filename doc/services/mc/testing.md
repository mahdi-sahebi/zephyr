<!--
SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
SPDX-License-Identifier: Apache-2.0
-->

# MC testing

This page describes the MC protocol tests currently ported to Zephyr and the
nRF54L15 DK hardware integration test.

## Workspace setup

Run the commands from a west workspace that contains this Zephyr checkout.
When this checkout is not the workspace's registered Zephyr repository, set
`ZEPHYR_BASE` explicitly:

```sh
export ZEPHYR_BASE=/run/media/mahdi/Storage/repositories/rasna/mc_message/zephyr
```

The workspace must contain the Zephyr modules and a Zephyr SDK installation.
Check the setup with:

```sh
west --version
west list
```

## Native in-memory tests

The `mc_comm` and `mc_msg` tests create two protocol endpoints in one test
application. Their transport is an in-memory, bidirectional test link; no
UART pins or second board are required.

Run both suites:

```sh
west twister \
  -p native_sim/native/64 \
  -T tests/subsys/mc/comm \
  -T tests/subsys/mc/msg \
  --inline-logs
```

The same tests can be built for one nRF54L15 DK. This validates the protocol
on the target CPU, but still uses the in-memory transport:

```sh
west twister \
  -p nrf54l15dk/nrf54l15/cpuapp \
  -T tests/subsys/mc/comm \
  -T tests/subsys/mc/msg \
  --device-testing \
  --device-serial /dev/ttyACM0
```

To build and flash one suite manually:

```sh
west build -b nrf54l15dk/nrf54l15/cpuapp \
  tests/subsys/mc/comm \
  -d build/mc-comm-nrf54l15 \
  --pristine=always
west flash -d build/mc-comm-nrf54l15
```

Replace `comm` with `msg` and use a different build directory for the message
suite.

## Two-board UART integration test

`tests/subsys/mc/uart` runs one endpoint on each of two nRF54L15 DKs. It
exercises `mc_comm` framing and acknowledgements and sends an `mc_msg` payload
and zero-payload signal over the physical UART.

Connect the boards with crossed data lines:

```text
Board A TX -> Board B RX
Board A RX <- Board B TX
Board A GND -- Board B GND
```

The test overlay uses UART30 for the protocol and keeps UART20 for the console.
Verify the UART30 pin routing on the specific DK before wiring it.

Build and flash Board A as the sender:

```sh
west build -b nrf54l15dk/nrf54l15/cpuapp \
  tests/subsys/mc/uart \
  -d build/mc-uart-sender \
  --pristine=always
west flash -d build/mc-uart-sender
```

Build and flash Board B as the receiver:

```sh
west build -b nrf54l15dk/nrf54l15/cpuapp \
  tests/subsys/mc/uart \
  -d build/mc-uart-receiver \
  --pristine=always \
  -DCONFIG_MC_UART_SENDER=n
west flash -d build/mc-uart-receiver
```

Open both console UARTs at 115200 baud. Successful execution prints:

```text
MC UART sender: PASS
MC UART receiver: PASS
```

The sender and receiver are separate firmware images. A single DK with a
local TX-to-RX jumper is not the configuration tested by this integration
application.

## Current coverage and limitations

Implemented coverage includes:

- Invalid configuration, storage, and API argument checks.
- Normal `mc_comm` transfer and bounded timeout/retry behavior.
- CRC-corrupted frame rejection.
- Zero-length `mc_comm` payloads and `mc_msg` signals.
- Message subscription, duplicate subscription, unsubscribe, capacity, normal
  message delivery, and unknown message IDs.
- nRF54L15 build targets for the in-memory suites.
- Two-board nRF54L15 UART message and signal integration.

The complete original microcode test plan is not yet fully ported. The
following scenarios still require dedicated deterministic tests:

- Partial reads and writes.
- Duplication, reordering, loss, and temporary disconnection scripts.
- Sequence-number wraparound.
- Multiple independent protocol instances.
- Large or fragmented messages and receive-pool exhaustion.
- Oversized-message recovery.
- Callback lifetime and exactly-once delivery checks.
- Multiple callback IDs and byte-order-specific assertions.
- QEMU ARM and RISC-V portability runs.
- Physical UART disconnect and recovery behavior.

Therefore, the current tests validate the core protocol paths and the UART
smoke/integration path, but they do not yet represent every test case from the
original microcode test suite.
