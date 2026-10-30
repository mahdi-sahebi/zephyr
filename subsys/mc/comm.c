/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>

#include <zephyr/mc/comm.h>

size_t mc_comm_required_size(const struct mc_comm_config *config)
{
	(void)config;
	return 0U;
}

int mc_comm_init(struct mc_comm **instance, void *storage, size_t storage_size,
		 const struct mc_comm_config *config)
{
	(void)instance;
	(void)storage;
	(void)storage_size;
	(void)config;
	return -ENOSYS;
}

int mc_comm_update(struct mc_comm *instance)
{
	(void)instance;
	return -ENOSYS;
}

int mc_comm_send(struct mc_comm *instance, const void *data, size_t length,
		 uint32_t timeout_us)
{
	(void)instance;
	(void)data;
	(void)length;
	(void)timeout_us;
	return -ENOSYS;
}

int mc_comm_receive(struct mc_comm *instance, void *data, size_t capacity,
		    size_t *length, uint32_t timeout_us)
{
	(void)instance;
	(void)data;
	(void)capacity;
	(void)length;
	(void)timeout_us;
	return -ENOSYS;
}

int mc_comm_flush(struct mc_comm *instance, uint32_t timeout_us)
{
	(void)instance;
	(void)timeout_us;
	return -ENOSYS;
}
