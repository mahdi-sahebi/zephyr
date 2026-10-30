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

/**
 * @brief Get the storage size required by a communication instance.
 *
 * @param config Communication configuration.
 * @return Required size in bytes, or 0 for an invalid configuration.
 */
size_t mc_comm_required_size(const struct mc_comm_config *config);

/**
 * @brief Initialize a communication instance in caller-owned storage.
 *
 * @param instance Output instance pointer.
 * @param storage Suitably aligned storage.
 * @param storage_size Available storage size in bytes.
 * @param config Communication configuration.
 * @retval 0 Instance initialized.
 * @retval -EINVAL Invalid argument or configuration.
 * @retval -ENOMEM Storage is insufficient or misaligned.
 */
int mc_comm_init(struct mc_comm **instance, void *storage, size_t storage_size,
		 const struct mc_comm_config *config);

/**
 * @brief Process one transport frame.
 *
 * @param instance Communication instance.
 * @retval 0 Frame processed or no frame available.
 * @retval -EINVAL Invalid instance.
 * @retval -EBADMSG Invalid frame.
 * @return Other errors returned by the transport callbacks.
 */
int mc_comm_update(struct mc_comm *instance);

/**
 * @brief Send one application payload and wait for acknowledgment.
 *
 * @param instance Communication instance.
 * @param data Payload buffer, or NULL for an empty payload.
 * @param length Payload length in bytes.
 * @param timeout_us Timeout in microseconds.
 * @retval 0 Payload acknowledged.
 * @retval -EINVAL Invalid instance or payload buffer.
 * @retval -EMSGSIZE Payload exceeds the frame capacity.
 * @retval -ETIMEDOUT Acknowledgment timed out.
 * @return Other errors returned by the transport callbacks.
 */
int mc_comm_send(struct mc_comm *instance, const void *data, size_t length, uint32_t timeout_us);

/**
 * @brief Receive one application payload.
 *
 * @param instance Communication instance.
 * @param data Output payload buffer.
 * @param capacity Output buffer capacity in bytes.
 * @param length Output payload length in bytes.
 * @param timeout_us Timeout in microseconds.
 * @retval 0 Payload received.
 * @retval -EINVAL Invalid argument.
 * @retval -ENOSPC Output buffer is too small.
 * @retval -ETIMEDOUT No payload arrived before the timeout.
 * @return Other errors returned by the transport callbacks.
 */
int mc_comm_receive(struct mc_comm *instance, void *data, size_t capacity, size_t *length,
		    uint32_t timeout_us);

/**
 * @brief Wait until the pending transmission is acknowledged.
 *
 * @param instance Communication instance.
 * @param timeout_us Timeout in microseconds.
 * @retval 0 No transmission is pending.
 * @retval -EINVAL Invalid instance.
 * @retval -ETIMEDOUT Transmission is still pending at the timeout.
 */
int mc_comm_flush(struct mc_comm *instance, uint32_t timeout_us);

#ifdef __cplusplus
}
#endif

#endif
