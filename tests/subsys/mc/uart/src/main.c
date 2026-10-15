/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/mc/msg.h>
#include <zephyr/sys/printk.h>
#include <zephyr/ztest.h>

#if DT_HAS_ALIAS(mc_uart)
#define MC_UART_NODE DT_ALIAS(mc_uart)
#else
#error "MC UART test requires a mc-uart devicetree alias"
#endif

static const struct device *const uart = DEVICE_DT_GET(MC_UART_NODE);
static bool message_received;
static bool signal_received;

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

static void on_message(void *context, uint16_t id, const uint8_t *data,
			       size_t length)
{
	ARG_UNUSED(context);

	if (id == 0x42U && length == 5U && memcmp(data, "hello", 5U) == 0) {
		message_received = true;
	}
	if (id == 0x43U && length == 0U) {
		signal_received = true;
	}
}

static bool wait_for_receive(struct mc_msg *msg)
{
	int64_t deadline = k_uptime_get() + 5000;

	while ((!message_received || !signal_received) &&
		k_uptime_get() < deadline) {
		(void)mc_msg_update(msg);
		k_msleep(1U);
	}

	return message_received && signal_received;
}

ZTEST(mc_uart, test_message_transport)
{
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

	zassert_true(device_is_ready(uart), NULL);
	zassert_ok(mc_msg_init(&msg, storage, sizeof(storage), &config), NULL);
	zassert_ok(mc_msg_subscribe(msg, 0x42U, on_message, NULL), NULL);
	zassert_ok(mc_msg_subscribe(msg, 0x43U, on_message, NULL), NULL);

	if (IS_ENABLED(CONFIG_MC_UART_SENDER)) {
		k_msleep(1000U);
		zassert_ok(mc_msg_send(msg, 0x42U, "hello", 5U, 5000000U), NULL);
		zassert_ok(mc_msg_signal(msg, 0x43U, 5000000U), NULL);
		printk("MC UART sender: PASS\n");
	} else {
		zassert_true(wait_for_receive(msg), NULL);
		printk("MC UART receiver: PASS\n");
	}
}

ZTEST_SUITE(mc_uart, NULL, NULL, NULL, NULL, NULL);
