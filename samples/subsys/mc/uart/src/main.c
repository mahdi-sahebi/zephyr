/*
 * Copyright (c) 2026 Mahdi Sahebi
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <errno.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>

#include <zephyr/mc/comm.h>

#if DT_HAS_ALIAS(mc_uart)
#define MC_UART_NODE DT_ALIAS(mc_uart)
#else
#error "MC UART sample requires a mc-uart devicetree alias"
#endif

static const struct device *const uart = DEVICE_DT_GET(MC_UART_NODE);

static int uart_receive(void *context, uint8_t *data, size_t capacity)
{
	ARG_UNUSED(context);
	size_t count = 0U;
	while (count < capacity && uart_poll_in(uart, &data[count]) == 0) {
		count++;
	}
	return count == 0U ? -EAGAIN : (int)count;
}

static int uart_transmit(void *context, const uint8_t *data, size_t length)
{
	ARG_UNUSED(context);
	for (size_t i = 0U; i < length; i++) {
		uart_poll_out(uart, data[i]);
	}
	return (int)length;
}

int main(void)
{
	if (!device_is_ready(uart)) {
		return 1;
	}
	static uint8_t storage[512] __aligned(8);
	struct mc_comm *comm;
	struct mc_comm_config config = {
		.receive = uart_receive,
		.transmit = uart_transmit,
		.frame_size = 64U,
		.receive_capacity = 64U,
	};
	if (mc_comm_init(&comm, storage, sizeof(storage), &config) != 0) {
		return 1;
	}
	printk("MC UART loopback ready\n");
	while (true) {
		(void)mc_comm_update(comm);
		k_msleep(1U);
	}
}
