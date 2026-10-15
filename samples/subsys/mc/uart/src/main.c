/*
 * Copyright (c) 2026 Mahdi Sahebi
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <errno.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>

#include <zephyr/mc/msg.h>

#if DT_HAS_ALIAS(mc_uart)
#define MC_UART_NODE DT_ALIAS(mc_uart)
#else
#error "MC UART sample requires a mc-uart devicetree alias"
#endif

static const struct device *const uart = DEVICE_DT_GET(MC_UART_NODE);

static bool message_received;

static void on_message(void *context, uint16_t id, const uint8_t *data,
			       size_t length)
{
	ARG_UNUSED(context);

	if (id == 0x42U && length == 5U && memcmp(data, "hello", 5U) == 0) {
		message_received = true;
		printk("MC UART: message received\n");
	}
}

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
	struct mc_msg *msg;
	struct mc_msg_config config = {
		.comm = {
			.receive = uart_receive,
			.transmit = uart_transmit,
			.frame_size = 64U,
			.receive_capacity = 64U,
		},
		.message_capacity = 32U,
		.subscription_capacity = 2U,
	};

	if (mc_msg_init(&msg, storage, sizeof(storage), &config) != 0 ||
		mc_msg_subscribe(msg, 0x42U, on_message, NULL) != 0) {
		return 1;
	}

	printk("MC UART %s ready\n",
		IS_ENABLED(CONFIG_MC_UART_SENDER) ? "sender" : "receiver");

	if (IS_ENABLED(CONFIG_MC_UART_SENDER)) {
		k_msleep(1000U);
		if (mc_msg_send(msg, 0x42U, "hello", 5U, 5000000U) != 0 ||
			mc_msg_signal(msg, 0x43U, 5000000U) != 0) {
			printk("MC UART: FAIL\n");
			return 1;
		}
		printk("MC UART: PASS\n");
	}

	while (true) {
		(void)mc_msg_update(msg);
		if (!IS_ENABLED(CONFIG_MC_UART_SENDER) && message_received) {
			printk("MC UART: PASS\n");
			message_received = false;
		}
		k_msleep(1U);
	}
}
