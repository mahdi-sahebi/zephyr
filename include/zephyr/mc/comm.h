/*
 * Copyright (c) 2026 Mahdi Sahebi
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_MC_COMM_H_
#define ZEPHYR_INCLUDE_MC_COMM_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct mc_comm;

typedef int (*mc_comm_receive_t)(void *context, uint8_t *data, size_t capacity);
typedef int (*mc_comm_transmit_t)(void *context, const uint8_t *data, size_t length);

struct mc_comm_config {
	mc_comm_receive_t receive;
	mc_comm_transmit_t transmit;
	void *context;
	size_t frame_size;
	size_t receive_capacity;
	uint8_t retry_limit;
};

/** Return the storage required by an instance, or 0 for invalid settings. */
size_t mc_comm_required_size(const struct mc_comm_config *config);

/** Initialize an instance in caller-provided, suitably aligned storage. */
int mc_comm_init(struct mc_comm **instance, void *storage, size_t storage_size,
		 const struct mc_comm_config *config);

/** Process transport input and pending retransmissions. */
int mc_comm_update(struct mc_comm *instance);

/** Send one application payload, waiting up to timeout_us for progress. */
int mc_comm_send(struct mc_comm *instance, const void *data, size_t length,
		 uint32_t timeout_us);

/** Receive one queued application payload. */
int mc_comm_receive(struct mc_comm *instance, void *data, size_t capacity,
		    size_t *length, uint32_t timeout_us);

/** Wait until all pending transport state has completed. */
int mc_comm_flush(struct mc_comm *instance, uint32_t timeout_us);

#ifdef __cplusplus
}
#endif

#endif
