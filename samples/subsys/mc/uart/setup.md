# nRF54L15 DK two-board UART setup

This guide builds and runs the `mc_comm`/`mc_msg` UART sample on two
nRF54L15 DKs. UART Port 1 carries protocol data between the boards, while
UART Port 0 remains available for console output on both boards.

## Working directory

Run the build and flash commands from the UART sample directory. In every new
build terminal, first run:

```sh
cd /run/media/mahdi/Storage/repositories/rasna/mc_message/zephyr/samples/subsys/mc/uart
```

You can confirm that you are in the correct directory with:

```sh
pwd
ls build_flash.sh prj_sender.conf prj_receiver.conf
```

The expected `pwd` output is:

```text
/run/media/mahdi/Storage/repositories/rasna/mc_message/zephyr/samples/subsys/mc/uart
```

There is intentionally no default `prj.conf`. Every build must explicitly use
either `prj_sender.conf` or `prj_receiver.conf`; the helper script does this
automatically.

## Assigned boards

| Role | Probe label | ID reported by `nrfutil` | Console |
|---|---:|---:|---|
| Sender | `001057753351` | `1057753351` | VCOM0, currently `/dev/ttyACM2` |
| Receiver | `001057744552` | `1057744552` | VCOM0, currently `/dev/ttyACM0` |

Linux device names can change after reconnecting USB. Run
`nrfutil device list` and use the `vcom: 0` entry for each board's console.

## Required software

Install an nRF Connect SDK toolchain containing:

- `west`
- Zephyr SDK/Arm compiler
- `nrfutil` with device support
- `picocom` or another 115200-baud serial terminal

The tested setup uses the nRF Connect SDK Toolchain v3.4.0 and the local
Zephyr tree in this workspace. Open an nRF Connect SDK terminal before
running the commands. Confirm the tools and boards:

```sh
west --version
nrfutil device list
```

If Linux denies access to `/dev/ttyACM*`, add the current user to the serial
device group, log out, and log back in:

```sh
sudo usermod -aG dialout "$USER"
```

## UART allocation

| Function | Zephyr device | Signals | USB interface |
|---|---|---|---|
| `mc_comm` protocol | UART20 / Serial Port 1 | P1.04 TXD1, P1.05 RXD1 | VCOM1; keep closed |
| Console | UART30 / Serial Port 0 | P0.00 TXD0, P0.01 RXD0 | VCOM0 |

Both UARTs use 115200 baud, 8 data bits, no parity, and one stop bit. The
protocol uses only TX, RX, and GND; RTS/CTS hardware flow control is not used.

Do not open VCOM1 (`/dev/ttyACM1` and `/dev/ttyACM3` in the current mapping)
during the test. Opening it connects the onboard debugger to P1.04/P1.05 and
can electrically interfere with the board-to-board protocol.

## Wiring

Disconnect USB power while changing jumper wires. Remove any previous wires
from P0.00/P0.01 or P1.11/P1.12. Connect exactly three wires:

```text
Sender board (1057753351)          Receiver board (1057744552)

P1.04 / TXD1  ------------------> P1.05 / RXD1
P1.05 / RXD1  <------------------ P1.04 / TXD1
GND           ------------------- GND
```

TX must cross to RX. Do not connect TX to TX or RX to RX. Do not connect 3 V,
5 V, or VDD between the boards; each board is powered by its own USB cable.

## Automated build and flash

Change to the sample directory and make the helper executable once:

```sh
cd /run/media/mahdi/Storage/repositories/rasna/mc_message/zephyr/samples/subsys/mc/uart
chmod +x build_flash.sh
```

Build and flash the receiver first:

```sh
./build_flash.sh receiver
```

Build and flash the sender second:

```sh
./build_flash.sh sender
```

The script uses these role assignments by default:

```sh
SENDER_DEVICE_ID=1057753351
RECEIVER_DEVICE_ID=1057744552
```

They can be overridden without editing the script, for example:

```sh
SENDER_DEVICE_ID=123456789 ./build_flash.sh sender
```

## Manual build and flash commands

First change to the sample directory:

```sh
cd /run/media/mahdi/Storage/repositories/rasna/mc_message/zephyr/samples/subsys/mc/uart
```

Then run the following commands.

Receiver:

```sh
west -z /run/media/mahdi/Storage/repositories/rasna/mc_message/zephyr \
  build -b nrf54l15dk/nrf54l15/cpuapp \
  . -d build_receiver --pristine=always \
  -- -DCONF_FILE=prj_receiver.conf

west flash -d build_receiver --dev-id 1057744552
```

Sender:

```sh
west -z /run/media/mahdi/Storage/repositories/rasna/mc_message/zephyr \
  build -b nrf54l15dk/nrf54l15/cpuapp \
  . -d build_sender --pristine=always \
  -- -DCONF_FILE=prj_sender.conf

west flash -d build_sender --dev-id 1057753351
```

## Run the test

Open only VCOM0. With the current device mapping, use two terminals:

```sh
picocom -b 115200 /dev/ttyACM0  # receiver console
```

```sh
picocom -b 115200 /dev/ttyACM2  # sender console
```

Reset the receiver first, then reset the sender. The sender waits one second
before transmitting.

Expected receiver output:

```text
*** Booting Zephyr OS build ... ***
MC UART receiver ready
MC UART: message received
MC UART: PASS
```

Expected sender output:

```text
*** Booting Zephyr OS build ... ***
MC UART sender ready
MC UART: PASS
```

## Troubleshooting

- `MC UART: FAIL` means the sender did not receive an acknowledgment. Check
  crossed TX/RX wiring, common GND, and reset order.
- If the receiver prints only `MC UART receiver ready`, check that P1.04 on
  the sender reaches P1.05 on the receiver.
- Close all programs using VCOM1. This includes serial monitors in VS Code.
- Use `nrfutil device list` after reconnecting USB because `/dev/ttyACM*`
  numbers may change.
- If a build reports undefined Kconfig symbols, ensure the local Zephyr tree
  is selected with `west -z .../mc_message/zephyr`.
- If flashing reports an invalid CMake cache, rebuild successfully before
  running `west flash`.
