#ifndef H_CONFIG_
#define H_CONFIG_

#if __ZEPHYR__

typedef struct k_mem_slab nffs_os_mempool_t;

#define NFFS_CONFIG_USE_HEAP            0
#define NFFS_CONFIG_MAX_AREAS           CONFIG_NFFS_FILESYSTEM_MAX_AREAS
#define NFFS_CONFIG_MAX_BLOCK_SIZE      CONFIG_NFFS_FILESYSTEM_MAX_BLOCK_SIZE

#else

/* Default to Mynewt */

typedef struct os_mempool nffs_os_mempool_t;

#define NFFS_CONFIG_USE_HEAP            1
#define NFFS_CONFIG_MAX_AREAS           256
#define NFFS_CONFIG_MAX_BLOCK_SIZE      2048

#endif

#endif
