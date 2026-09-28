.. SPDX-License-Identifier: Apache-2.0

ID-based messages
#################

``mc_msg`` builds on ``mc_comm``. A message contains a 16-bit message ID, a
16-bit big-endian payload length, and the payload. A zero-length payload is a
signal. Applications register callbacks for IDs and must keep callback-owned
data only for the duration of the callback.

Both the communication instance and subscription storage are placed in the
single caller-provided region returned by ``mc_msg_required_size()``.
