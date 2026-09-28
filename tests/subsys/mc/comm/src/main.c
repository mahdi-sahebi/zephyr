/*
 * Copyright (c) 2026 Mahdi Sahebi
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include <zephyr/ztest.h>
#include <zephyr/mc/comm.h>

struct test_link {
	uint8_t data[128];
	size_t length;
	bool full;
	struct mc_comm *peer;
};

static int receive(void *context, uint8_t *data, size_t capacity)
{
	struct test_link *link = context;
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
	struct test_link *link = context;
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

ZTEST(mc_comm, test_invalid_configuration)
{
	struct mc_comm_config config = {0};
	zassert_equal(mc_comm_required_size(&config), 0U, NULL);
}

ZTEST(mc_comm, test_loopback_send_receive)
{
	struct test_link a_to_b = {0};
	struct test_link b_to_a = {0};
	struct mc_comm_config config = {
		.receive = receive,
		.transmit = transmit,
		.frame_size = 32U,
		.receive_capacity = 32U,
	};
	uint8_t a_storage[256] __aligned(8);
	uint8_t b_storage[256] __aligned(8);
	struct mc_comm *a;
	struct mc_comm *b;
	zassert_ok(mc_comm_init(&a, a_storage, sizeof(a_storage), &config), NULL);
	config.context = &b_to_a;
	zassert_ok(mc_comm_init(&b, b_storage, sizeof(b_storage), &config), NULL);
	a_to_b.peer = b;
	b_to_a.peer = a;
	config.context = &a_to_b;
	/* Reinitialize A with its transport context after the common setup. */
	zassert_ok(mc_comm_init(&a, a_storage, sizeof(a_storage), &config), NULL);
	a_to_b.peer = b;
	const char payload[] = "payload";
	zassert_ok(mc_comm_send(a, payload, sizeof(payload), 100000U), NULL);
	uint8_t received_data[sizeof(payload)];
	size_t received_length = 0U;
	zassert_ok(mc_comm_receive(b, received_data, sizeof(received_data),
		&received_length, 100000U), NULL);
	zassert_equal(received_length, sizeof(payload), NULL);
	zassert_mem_equal(received_data, payload, sizeof(payload), NULL);
}

ZTEST_SUITE(mc_comm, NULL, NULL, NULL, NULL, NULL);
