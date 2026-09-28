/*
 * Copyright (c) 2026 Mahdi Sahebi
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include <zephyr/ztest.h>
#include <zephyr/mc/msg.h>

struct link {
	uint8_t data[128];
	size_t length;
	bool full;
	struct mc_comm *peer;
};

static int receive(void *context, uint8_t *data, size_t capacity)
{
	struct link *link = context;
	if (!link->full) {
		return -EAGAIN;
	}
	if (capacity < link->length) {
		return -ENOSPC;
	}
	memcpy(data, link->data, link->length);
	link->full = false;
	return (int)link->length;
}

static int transmit(void *context, const uint8_t *data, size_t length)
{
	struct link *link = context;
	if (link->full || length > sizeof(link->data)) {
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

static bool received;

static void callback(void *context, uint16_t id, const uint8_t *data, size_t length)
{
	ARG_UNUSED(context);
	received = id == 7U && length == 5U && memcmp(data, "hello", 5U) == 0;
}

ZTEST(mc_msg, test_message_and_signal)
{
	struct link a_to_b = {0};
	struct link b_to_a = {0};
	struct mc_msg_config config = {
		.comm = {
			.receive = receive,
			.transmit = transmit,
			.frame_size = 64U,
			.receive_capacity = 64U,
		},
		.message_capacity = 32U,
		.subscription_capacity = 2U,
	};
	uint8_t a_storage[512] __aligned(8);
	uint8_t b_storage[512] __aligned(8);
	struct mc_msg *a;
	struct mc_msg *b;
	config.comm.context = &a_to_b;
	zassert_ok(mc_msg_init(&a, a_storage, sizeof(a_storage), &config), NULL);
	config.comm.context = &b_to_a;
	zassert_ok(mc_msg_init(&b, b_storage, sizeof(b_storage), &config), NULL);
	a_to_b.peer = mc_msg_comm(b);
	b_to_a.peer = mc_msg_comm(a);
	received = false;
	zassert_ok(mc_msg_subscribe(b, 7U, callback, NULL), NULL);
	zassert_ok(mc_msg_send(a, 7U, "hello", 5U, 100000U), NULL);
	zassert_ok(mc_msg_update(b), NULL);
	zassert_true(received, NULL);
	zassert_ok(mc_msg_signal(a, 8U, 100000U), NULL);
}

ZTEST_SUITE(mc_msg, NULL, NULL, NULL, NULL, NULL);
