MC loopback
===========

This sample connects two ``mc_comm``/``mc_msg`` instances through a
deterministic in-memory transport. It demonstrates caller-owned storage,
message delivery, subscriptions, and a zero-payload signal.

Build and run with:

.. zephyr-app-commands::
   :zephyr-app: samples/subsys/mc/loopback
   :board: native_sim/native/64
   :goals: build run
   :compact:
