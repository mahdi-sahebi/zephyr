/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <zephyr/mc/pool.h>
#include <zephyr/ztest.h>

static uint8_t storage[1024] __aligned(64);
static uint8_t other_storage[1024] __aligned(64);

static const struct mc_pool_cfg default_config = {
	.item_size = 5U,
	.capacity = 3U,
	.alignment = 32U,
};

static struct mc_pool *make_pool(void)
{
	struct mc_pool *pool = NULL;

	memset(storage, 0xa5, sizeof(storage));
	zassert_ok(mc_pool_init(&pool, storage, sizeof(storage), &default_config));
	zassert_not_null(pool);
	return pool;
}

ZTEST(mc_pool, test_configuration_and_overflow)
{
	struct mc_pool_cfg config = default_config;
	const size_t header = mc_pool_req_size(&config) - config.capacity * config.alignment;

	zassert_true(header > 0U);
	zassert_equal(mc_pool_req_size(NULL), 0U);
	config.item_size = 0U;
	zassert_equal(mc_pool_req_size(&config), 0U);
	config = default_config;
	config.capacity = 0U;
	zassert_equal(mc_pool_req_size(&config), 0U);
	config = default_config;
	config.alignment = 0U;
	zassert_equal(mc_pool_req_size(&config), 0U);
	config.alignment = 3U;
	zassert_equal(mc_pool_req_size(&config), 0U);

	config = default_config;
	config.item_size = SIZE_MAX;
	zassert_equal(mc_pool_req_size(&config), 0U);
	config = default_config;
	config.capacity = SIZE_MAX;
	zassert_equal(mc_pool_req_size(&config), 0U);
	config = default_config;
	config.capacity = (SIZE_MAX >> 1) / (2U * sizeof(size_t)) + 1U;
	config.alignment = (SIZE_MAX >> 1) + 1U;
	zassert_equal(mc_pool_req_size(&config), 0U);
	config = default_config;
	config.item_size = SIZE_MAX / 4U;
	config.capacity = 8U;
	zassert_equal(mc_pool_req_size(&config), 0U);

	config.item_size = 1U;
	config.capacity = 1U;
	config.alignment = 1U;
	zassert_true(mc_pool_req_size(&config) > 0U);
}

ZTEST(mc_pool, test_initialization_errors)
{
	struct mc_pool *pool = NULL;
	struct mc_pool_cfg config = default_config;
	size_t required = mc_pool_req_size(&config);

	zassert_equal(mc_pool_init(NULL, storage, sizeof(storage), &config), -EINVAL);
	zassert_equal(mc_pool_init(&pool, NULL, sizeof(storage), &config), -EINVAL);
	zassert_equal(mc_pool_init(&pool, storage, sizeof(storage), NULL), -EINVAL);
	config.alignment = 3U;
	zassert_equal(mc_pool_init(&pool, storage, sizeof(storage), &config), -EINVAL);
	config = default_config;
	config.capacity = SIZE_MAX;
	zassert_equal(mc_pool_init(&pool, storage, sizeof(storage), &config), -EINVAL);
	config = default_config;
	zassert_equal(mc_pool_init(&pool, storage + 1U, sizeof(storage) - 1U, &config), -ENOMEM);
	zassert_equal(mc_pool_init(&pool, storage, required - 1U, &config), -ENOMEM);
	zassert_is_null(pool);
	zassert_ok(mc_pool_init(&pool, storage, required, &config));
}

ZTEST(mc_pool, test_add_remove_and_reuse)
{
	struct mc_pool *pool = make_pool();
	void *items[3];
	void *reused = NULL;
	void *unchanged = storage;

	zassert_equal(mc_pool_add(NULL, &reused), -EINVAL);
	zassert_equal(mc_pool_add(pool, NULL), -EINVAL);
	for (size_t i = 0U; i < 3U; ++i) {
		zassert_ok(mc_pool_add(pool, &items[i]));
		zassert_equal((uintptr_t)items[i] % default_config.alignment, 0U);
		memset(items[i], (int)(i + 1U), default_config.item_size);
	}
	zassert_equal(mc_pool_add(pool, &unchanged), -ENOSPC);
	zassert_equal_ptr(unchanged, storage);
	zassert_equal(((uint8_t *)items[0])[0], 1U);
	zassert_equal(((uint8_t *)items[2])[0], 3U);
	zassert_ok(mc_pool_remove(pool, items[1]));
	zassert_ok(mc_pool_add(pool, &reused));
	zassert_equal_ptr(reused, items[1]);
	zassert_ok(mc_pool_remove(pool, items[2]));
	zassert_ok(mc_pool_remove(pool, items[0]));
	zassert_ok(mc_pool_remove(pool, reused));
	zassert_equal(mc_pool_remove(pool, reused), -ENOENT);
	zassert_ok(mc_pool_add(pool, &reused));
	zassert_equal_ptr(reused, items[1]);
}

ZTEST(mc_pool, test_invalid_removal)
{
	struct mc_pool *pool = make_pool();
	struct mc_pool *other = NULL;
	void *first = NULL;
	void *second = NULL;
	void *foreign = NULL;

	zassert_ok(mc_pool_init(&other, other_storage, sizeof(other_storage), &default_config));
	zassert_ok(mc_pool_add(other, &foreign));
	zassert_equal(mc_pool_remove(NULL, foreign), -EINVAL);
	zassert_equal(mc_pool_remove(pool, NULL), -EINVAL);
	zassert_equal(mc_pool_remove(pool, foreign), -EINVAL);
	zassert_equal(mc_pool_remove(pool, storage), -EINVAL);
	zassert_ok(mc_pool_add(pool, &first));
	zassert_equal(mc_pool_remove(pool, (uint8_t *)first + 1U), -EINVAL);
	zassert_equal(mc_pool_remove(pool, (uint8_t *)first + 3U * default_config.alignment),
		      -EINVAL);
	zassert_equal(mc_pool_remove(pool, (uint8_t *)first + default_config.alignment), -ENOENT);
	zassert_ok(mc_pool_add(pool, &second));
	zassert_ok(mc_pool_remove(pool, second));
	zassert_equal(mc_pool_remove(pool, second), -ENOENT);
	zassert_ok(mc_pool_remove(pool, first));
}

ZTEST(mc_pool, test_clear)
{
	struct mc_pool *pool = make_pool();
	void *first = NULL;
	void *second = NULL;
	void *again = NULL;

	zassert_equal(mc_pool_clear(NULL), -EINVAL);
	zassert_ok(mc_pool_clear(pool));
	zassert_ok(mc_pool_add(pool, &first));
	*(uint8_t *)first = 0x5aU;
	zassert_ok(mc_pool_add(pool, &second));
	zassert_ok(mc_pool_remove(pool, second));
	zassert_ok(mc_pool_clear(pool));
	zassert_equal(mc_pool_remove(pool, first), -ENOENT);
	zassert_equal(mc_pool_remove(pool, second), -ENOENT);
	zassert_ok(mc_pool_add(pool, &again));
	zassert_equal_ptr(again, first);
	zassert_equal(*(uint8_t *)again, 0x5aU);
	zassert_ok(mc_pool_add(pool, &again));
	zassert_equal_ptr(again, second);
}

ZTEST_SUITE(mc_pool, NULL, NULL, NULL, NULL, NULL);
