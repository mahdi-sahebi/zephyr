/*
 * Copyright (c) 2026 Mahdi Sahebi
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/mc/msg.h>

struct link {
	uint8_t data[256];
	size_t length;
	bool full;
	struct mc_comm *peer;
};

static int link_receive(void *context, uint8_t *data, size_t capacity)
{
	struct link *link = context;
	if (!link->full) {
		return -EAGAIN;
	}
	if (capacity < link->length) {
		return -ENOSPC;
	}
	memcpy(data, link->data, link->length);
	size_t length = link->length;
	link->full = false;
	return (int)length;
}

static int link_transmit(void *context, const uint8_t *data, size_t length)
{
	struct link *link = context;
	if (length > sizeof(link->data) || link->full) {
		return -EAGAIN;
	}
	memcpy(link->data, data, length);
	link->length = length;
	link->full = true;
	if (link->peer != NULL) {
		(void)mc_comm_update(link->peer);
	}
	return (int)length;
}

static void received(void *context, uint16_t id, const uint8_t *data, size_t length)
{
	bool *seen = context;
	if (id == 7U && length == 5U && memcmp(data, "hello", 5U) == 0) {
		*seen = true;
	}
}

int main(void)
{
	struct link a_to_b = {0};
	struct link b_to_a = {0};
	struct mc_msg_config config = {
		.comm = {
			.receive = link_receive,
			.transmit = link_transmit,
			.frame_size = 64U,
			.receive_capacity = 64U,
		},
		.message_capacity = 32U,
		.subscription_capacity = 2U,
	};
	static uint8_t a_storage[512] __aligned(8);
	static uint8_t b_storage[512] __aligned(8);
	struct mc_msg *a;
	struct mc_msg *b;
	config.comm.context = &a_to_b;
	if (mc_msg_init(&a, a_storage, sizeof(a_storage), &config) != 0) {
		return 1;
	}
	config.comm.context = &b_to_a;
	if (mc_msg_init(&b, b_storage, sizeof(b_storage), &config) != 0) {
		return 1;
	}
	a_to_b.peer = mc_msg_comm(b);
	b_to_a.peer = mc_msg_comm(a);
	bool seen = false;
	if (mc_msg_subscribe(b, 7U, received, &seen) != 0 ||
		mc_msg_send(a, 7U, "hello", 5U, 1000000U) != 0 ||
		mc_msg_update(b) != 0 || !seen ||
		mc_msg_signal(a, 8U, 1000000U) != 0) {
		return 1;
	}
	printk("MC loopback: PASS\n");
	return 0;
}
