/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Fixed-size Micro Core memory pool.
 * @ingroup mc_pool
 */

#ifndef ZEPHYR_INCLUDE_MC_POOL_H_
#define ZEPHYR_INCLUDE_MC_POOL_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup mc_pool Micro Core memory pool
 * @ingroup memory_management
 * @version 0.1.0
 * @{
 */

struct mc_pool;

/** @brief Fixed-size pool configuration. */
struct mc_pool_cfg {
	size_t item_size; /**< Size of each item in bytes; must be nonzero. */
	size_t capacity;  /**< Maximum number of live items; must be nonzero. */
	size_t alignment; /**< Item alignment in bytes; must be a power of two. */
};

/**
 * @brief Get the size of storage needed by a pool.
 *
 * @param config Pool configuration.
 * @return Required size in bytes, or zero for an invalid or overflowing configuration.
 */
size_t mc_pool_req_size(const struct mc_pool_cfg *config);

/**
 * @brief Initialize a pool in caller-owned storage.
 *
 * The storage and returned pool must remain valid until the caller stops using
 * the pool. Callers must serialize operations on the same pool.
 *
 * @param pool Output pool pointer.
 * @param storage Caller-owned storage aligned to the greater of 8 bytes and
 *                @p config alignment.
 * @param storage_size Available storage size in bytes.
 * @param config Pool configuration.
 * @retval 0 Pool initialized.
 * @retval -EINVAL Invalid argument or configuration.
 * @retval -ENOMEM Storage is insufficient or misaligned.
 */
int mc_pool_init(struct mc_pool **pool, void *storage, size_t storage_size,
		 const struct mc_pool_cfg *config);

/**
 * @brief Reserve an item for the caller to fill.
 *
 * The item contents are not initialized. This operation takes constant time.
 *
 * @param pool Initialized pool.
 * @param item Output pointer to the reserved item.
 * @retval 0 Item reserved.
 * @retval -EINVAL Invalid argument.
 * @retval -ENOSPC Pool is full.
 */
int mc_pool_add(struct mc_pool *pool, void **item);

/**
 * @brief Return an item to the pool.
 *
 * The pointer becomes invalid after removal. Once its slot is reused, a stale
 * pointer cannot be distinguished from the new item's pointer. The contents
 * are not erased. This operation takes constant time.
 *
 * @param pool Initialized pool.
 * @param item Pointer returned by @ref mc_pool_add for this pool.
 * @retval 0 Item removed.
 * @retval -EINVAL Invalid pool or pointer outside an item boundary.
 * @retval -ENOENT Item is not currently allocated.
 */
int mc_pool_remove(struct mc_pool *pool, void *item);

/**
 * @brief Return all items to the pool.
 *
 * All item pointers become invalid. The contents are not erased. This
 * operation takes constant time.
 *
 * @param pool Initialized pool.
 * @retval 0 Pool cleared.
 * @retval -EINVAL Invalid pool.
 */
int mc_pool_clear(struct mc_pool *pool);

/** @} */

#ifdef __cplusplus
}
#endif

#endif
