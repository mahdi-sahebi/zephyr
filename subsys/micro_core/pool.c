/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <zephyr/mc/pool.h>

#define MC_POOL_MIN_ALIGNMENT 8U

struct mc_pool_slot {
	size_t next;
	bool active;
};

struct mc_pool {
	struct mc_pool_slot *slots;
	uint8_t *items;
	size_t capacity;
	size_t stride;
	size_t next_unused;
	size_t free_head;
};

static size_t align_up(size_t value, size_t alignment)
{
	return (value + alignment - 1U) & ~(alignment - 1U);
}

static size_t item_alignment(const struct mc_pool_cfg *config)
{
	return config->alignment > MC_POOL_MIN_ALIGNMENT ? config->alignment
							 : MC_POOL_MIN_ALIGNMENT;
}

static bool valid_config(const struct mc_pool_cfg *config)
{
	return config != NULL && config->item_size > 0U && config->capacity > 0U &&
	       config->alignment > 0U && (config->alignment & (config->alignment - 1U)) == 0U;
}

static size_t items_offset(const struct mc_pool_cfg *config)
{
	return align_up(align_up(sizeof(struct mc_pool), MC_POOL_MIN_ALIGNMENT) +
				config->capacity * sizeof(struct mc_pool_slot),
			item_alignment(config));
}

size_t mc_pool_req_size(const struct mc_pool_cfg *config)
{
	if (!valid_config(config)) {
		return 0U;
	}

	size_t alignment = item_alignment(config);
	size_t header_size = align_up(sizeof(struct mc_pool), MC_POOL_MIN_ALIGNMENT);

	if (config->item_size > SIZE_MAX - (alignment - 1U)) {
		return 0U;
	}
	size_t stride = align_up(config->item_size, alignment);

	if (config->capacity > (SIZE_MAX - header_size) / sizeof(struct mc_pool_slot)) {
		return 0U;
	}
	size_t metadata_end = header_size + config->capacity * sizeof(struct mc_pool_slot);

	if (metadata_end > SIZE_MAX - (alignment - 1U)) {
		return 0U;
	}
	size_t offset = align_up(metadata_end, alignment);

	if (config->capacity > (SIZE_MAX - offset) / stride) {
		return 0U;
	}
	return offset + config->capacity * stride;
}

int mc_pool_init(struct mc_pool **pool, void *storage, size_t storage_size,
		 const struct mc_pool_cfg *config)
{
	if (pool == NULL || storage == NULL || !valid_config(config)) {
		return -EINVAL;
	}
	size_t required = mc_pool_req_size(config);
	size_t alignment = item_alignment(config);

	if (required == 0U) {
		return -EINVAL;
	}
	if (((uintptr_t)storage & (alignment - 1U)) != 0U || storage_size < required) {
		return -ENOMEM;
	}

	struct mc_pool *instance = storage;

	instance->slots =
		(struct mc_pool_slot *)((uint8_t *)storage +
					align_up(sizeof(struct mc_pool), MC_POOL_MIN_ALIGNMENT));
	instance->items = (uint8_t *)storage + items_offset(config);
	instance->capacity = config->capacity;
	instance->stride = align_up(config->item_size, alignment);
	instance->next_unused = 0U;
	instance->free_head = SIZE_MAX;
	*pool = instance;
	return 0;
}

int mc_pool_add(struct mc_pool *pool, void **item)
{
	if (pool == NULL || item == NULL) {
		return -EINVAL;
	}

	size_t index;

	if (pool->free_head != SIZE_MAX) {
		index = pool->free_head;
		pool->free_head = pool->slots[index].next;
	} else {
		if (pool->next_unused == pool->capacity) {
			return -ENOSPC;
		}
		index = pool->next_unused++;
	}
	pool->slots[index].active = true;
	*item = pool->items + index * pool->stride;
	return 0;
}

int mc_pool_remove(struct mc_pool *pool, void *item)
{
	if (pool == NULL || item == NULL) {
		return -EINVAL;
	}

	uintptr_t address = (uintptr_t)item;
	uintptr_t start = (uintptr_t)pool->items;

	if (address < start || address - start >= pool->capacity * pool->stride ||
	    (address - start) % pool->stride != 0U) {
		return -EINVAL;
	}
	size_t index = (address - start) / pool->stride;

	if (index >= pool->next_unused || !pool->slots[index].active) {
		return -ENOENT;
	}
	pool->slots[index].active = false;
	pool->slots[index].next = pool->free_head;
	pool->free_head = index;
	return 0;
}

int mc_pool_clear(struct mc_pool *pool)
{
	if (pool == NULL) {
		return -EINVAL;
	}
	pool->next_unused = 0U;
	pool->free_head = SIZE_MAX;
	return 0;
}
