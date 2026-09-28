/*
 * Copyright (c) 2026 Mahdi Sahebi
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/mc/msg.h>

struct link {
	uint8_t frame[128];
	size_t length;
	bool pending;
	struct mc_comm *peer;
};

static int link_receive(void *context, uint8_t *data, size_t capacity)
{
	struct link *link = context;

	if (!link->pending) {
		return -EAGAIN;
	}
	if (capacity < link->length) {
		return -ENOSPC;
	}
	memcpy(data, link->frame, link->length);
	link->pending = false;
	return (int)link->length;
}

static int link_transmit(void *context, const uint8_t *data, size_t length)
{
	struct link *link = context;

	if (link->pending || length > sizeof(link->frame)) {
		return -EAGAIN;
	}
	memcpy(link->frame, data, length);
	link->length = length;
	link->pending = true;
	if (link->peer != NULL) {
		(void)mc_comm_update(link->peer);
	}
	return (int)length;
}

static bool message_received;

static void on_message(void *context, uint16_t id, const uint8_t *data,
			       size_t length)
{
	ARG_UNUSED(context);
	message_received = id == 0x42U && length == 5U &&
		memcmp(data, "hello", 5U) == 0;
}

int main(void)
{
	static uint8_t a_storage[512] __aligned(8);
	static uint8_t b_storage[512] __aligned(8);
	struct link a_to_b = {0};
	struct link b_to_a = {0};
	struct mc_msg *a;
	struct mc_msg *b;
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

	config.comm.context = &a_to_b;
	if (mc_msg_init(&a, a_storage, sizeof(a_storage), &config) != 0) {
		printk("MC TEST: init A failed\n");
		return 1;
	}
	config.comm.context = &b_to_a;
	if (mc_msg_init(&b, b_storage, sizeof(b_storage), &config) != 0) {
		printk("MC TEST: init B failed\n");
		return 1;
	}
	a_to_b.peer = mc_msg_comm(b);
	b_to_a.peer = mc_msg_comm(a);

	if (mc_msg_subscribe(b, 0x42U, on_message, NULL) != 0 ||
		mc_msg_send(a, 0x42U, "hello", 5U, 100000U) != 0 ||
		mc_msg_update(b) != 0 || !message_received ||
		mc_msg_signal(a, 0x43U, 100000U) != 0) {
		printk("MC TEST: FAIL\n");
		return 1;
	}

	printk("MC TEST: PASS\n");
	return 0;
}
