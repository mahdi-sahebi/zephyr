.. SPDX-License-Identifier: Apache-2.0

Reliable communication
######################

``mc_comm`` uses caller-owned, suitably aligned storage and application
callbacks for receiving and transmitting bytes. Each instance is owned by one
thread; applications must serialize access from multiple contexts.

The transport carries explicitly encoded frames with a magic value, frame
type, sequence number, payload length, and CRC-16-CCITT. Data frames are
acknowledged. ``mc_comm_update()`` must be called regularly by the owner.

The callback context and all buffers remain owned by the application. A
receive callback should return ``-EAGAIN`` when no bytes are available and a
positive number for the number of bytes received.

The API is experimental and the current implementation supports one pending
received payload per instance. The polling UART sample uses UART30 on the
nRF54L15 DK and leaves UART20 as the console.
