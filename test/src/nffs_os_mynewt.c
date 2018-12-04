/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#if !__ZEPHYR__

#include <os/mynewt.h>
#include "fs/fs.h"
#include "fs/fs_if.h"
#include "nffs/nffs.h"
#include <crc/crc16.h>
#include "nffs_test_utils.h"

static struct os_mutex nffs_mutex;
struct nffs_config nffs_config;
struct log nffs_log;

nffs_os_mempool_t nffs_file_pool;
nffs_os_mempool_t nffs_dir_pool;
nffs_os_mempool_t nffs_inode_entry_pool;
nffs_os_mempool_t nffs_block_entry_pool;
nffs_os_mempool_t nffs_cache_inode_pool;
nffs_os_mempool_t nffs_cache_block_pool;

void *nffs_file_mem;
void *nffs_dir_mem;
void *nffs_inode_mem;
void *nffs_block_entry_mem;
void *nffs_cache_inode_mem;
void *nffs_cache_block_mem;

int
nffs_os_mempool_init(void)
{
    int rc;

    assert(nffs_file_mem != NULL);
    rc = os_mempool_init(&nffs_file_pool, nffs_config.nc_num_files,
            sizeof(struct nffs_file), nffs_file_mem, "nffs_file_pool");
    assert(rc == 0);

    rc = os_mempool_init(&nffs_dir_pool, nffs_config.nc_num_dirs,
            sizeof(struct nffs_dir), nffs_dir_mem, "nffs_dir_pool");
    assert(rc == 0);

    rc = os_mempool_init(&nffs_inode_entry_pool, nffs_config.nc_num_inodes,
            sizeof(struct nffs_inode_entry), nffs_inode_mem,
            "nffs_inode_entry_pool");
    assert(rc == 0);

    rc = os_mempool_init(&nffs_block_entry_pool, nffs_config.nc_num_blocks,
            sizeof(struct nffs_hash_entry), nffs_block_entry_mem,
            "nffs_block_entry_pool");
    assert(rc == 0);

    rc = os_mempool_init(&nffs_cache_inode_pool, nffs_config.nc_num_cache_inodes,
            sizeof(struct nffs_cache_inode), nffs_cache_inode_mem,
            "nffs_cache_inode_pool");
    assert(rc == 0);

    rc = os_mempool_init(&nffs_cache_block_pool, nffs_config.nc_num_cache_blocks,
            sizeof(struct nffs_cache_block), nffs_cache_block_mem,
            "nffs_cache_inode_pool");
    assert(rc == 0);

    return rc;
}

void *
nffs_os_mempool_get(nffs_os_mempool_t *pool)
{
    return os_memblock_get(pool);
}

int
nffs_os_mempool_free(nffs_os_mempool_t *pool, void *block)
{
    return os_memblock_put(pool, block);
}

int
nffs_os_flash_read(uint8_t id, uint32_t address, void *dst, uint32_t num_bytes)
{
    return hal_flash_read(id, address, dst, num_bytes);
}

int
nffs_os_flash_write(uint8_t id, uint32_t address, const void *src,
        uint32_t num_bytes)
{
    return hal_flash_write(id, address, src, num_bytes);
}

int
nffs_os_flash_erase(uint8_t id, uint32_t address, uint32_t num_bytes)
{
    return hal_flash_erase(id, address, num_bytes);
}

int nffs_os_flash_info(uint8_t id, uint32_t sector, uint32_t *address,
        uint32_t *size)
{
    assert(0);
    return 0;
}

uint16_t
nffs_os_crc16_ccitt(uint16_t initial_crc, const void *buf, int len, int final)
{
    (void)final;
    return crc16_ccitt(initial_crc, buf, len);
}

const struct nffs_config nffs_config_dflt = {
    .nc_num_inodes = 100,
    .nc_num_blocks = 100,
    .nc_num_files = 4,
    .nc_num_cache_inodes = 4,
    .nc_num_cache_blocks = 64,
    .nc_num_dirs = 4,
};

void
nffs_config_init(void)
{
    if (nffs_config.nc_num_inodes == 0) {
        nffs_config.nc_num_inodes = nffs_config_dflt.nc_num_inodes;
    }
    if (nffs_config.nc_num_blocks == 0) {
        nffs_config.nc_num_blocks = nffs_config_dflt.nc_num_blocks;
    }
    if (nffs_config.nc_num_files == 0) {
        nffs_config.nc_num_files = nffs_config_dflt.nc_num_files;
    }
    if (nffs_config.nc_num_cache_inodes == 0) {
        nffs_config.nc_num_cache_inodes = nffs_config_dflt.nc_num_cache_inodes;
    }
    if (nffs_config.nc_num_cache_blocks == 0) {
        nffs_config.nc_num_cache_blocks = nffs_config_dflt.nc_num_cache_blocks;
    }
    if (nffs_config.nc_num_dirs == 0) {
        nffs_config.nc_num_dirs = nffs_config_dflt.nc_num_dirs;
    }
}

static int
nffs_open(const char *path, uint8_t access_flags, struct fs_file **out_fs_file)
{
    int rc;
    struct nffs_file *out_file;

    if (!nffs_misc_ready()) {
        rc = FS_EUNINIT;
        goto done;
    }

    rc = nffs_file_open(&out_file, path, access_flags);
    if (rc != 0) {
        goto done;
    }
    *out_fs_file = (struct fs_file *)out_file;

done:
    if (rc != 0) {
        *out_fs_file = NULL;
    }
    return rc;
}

static int
nffs_close(struct fs_file *fs_file)
{
    if (!fs_file) {
        return 0;
    }

    return nffs_file_close((struct nffs_file *)fs_file);
}

static int
nffs_seek(struct fs_file *fs_file, uint32_t offset)
{
    return nffs_file_seek((struct nffs_file *)fs_file, offset);
}

static uint32_t
nffs_getpos(const struct fs_file *fs_file)
{
    return ((const struct nffs_file *)fs_file)->nf_offset;
}

static int
nffs_file_len(const struct fs_file *fs_file, uint32_t *out_len)
{
    return nffs_inode_data_len(
            ((const struct nffs_file *)fs_file)->nf_inode_entry, out_len);
}

static int
nffs_read(struct fs_file *fs_file, uint32_t len, void *out_data,
          uint32_t *out_len)
{
    return nffs_file_read((struct nffs_file *)fs_file, len, out_data, out_len);
}

static int
nffs_write(struct fs_file *fs_file, const void *data, int len)
{
    if (!nffs_misc_ready()) {
        return FS_EUNINIT;
    }

    return nffs_write_to_file((struct nffs_file *)fs_file, data, len);
}

static int
nffs_unlink(const char *path)
{
    if (!nffs_misc_ready()) {
        return FS_EUNINIT;
    }

    return nffs_path_unlink(path);
}

static int
nffs_rename(const char *from, const char *to)
{
    if (!nffs_misc_ready()) {
        return FS_EUNINIT;
    }

    return nffs_path_rename(from, to);
}

static int
nffs_mkdir(const char *path)
{
    if (!nffs_misc_ready()) {
        return FS_EUNINIT;
    }

    return nffs_path_new_dir(path, NULL);
}

static int
nffs_opendir(const char *path, struct fs_dir **out_fs_dir)
{
    if (!nffs_misc_ready()) {
        return FS_EUNINIT;
    }

    return nffs_dir_open(path, (struct nffs_dir **)out_fs_dir);
}

static int
nffs_readdir(struct fs_dir *fs_dir, struct fs_dirent **out_fs_dirent)
{
    return nffs_dir_read((struct nffs_dir *)fs_dir,
            (struct nffs_dirent **)out_fs_dirent);
}

static int
nffs_closedir(struct fs_dir *fs_dir)
{
    return nffs_dir_close((struct nffs_dir *)fs_dir);
}

static int
nffs_dirent_name(const struct fs_dirent *fs_dirent, size_t max_len,
                 char *out_name, uint8_t *out_name_len)
{
    struct nffs_dirent *dirent = (struct nffs_dirent *)fs_dirent;
    assert(dirent != NULL && dirent->nde_inode_entry != NULL);
    return nffs_inode_read_filename(dirent->nde_inode_entry,
            max_len, out_name, out_name_len);
}

static int
nffs_dirent_is_dir(const struct fs_dirent *fs_dirent)
{
    uint32_t id;
    const struct nffs_dirent *dirent = (const struct nffs_dirent *)fs_dirent;

    assert(dirent != NULL && dirent->nde_inode_entry != NULL);
    id = dirent->nde_inode_entry->nie_hash_entry.nhe_id;
    return nffs_hash_id_is_dir(id);
}

struct fs_ops nffs_ops = {
    .f_open = nffs_open,
    .f_close = nffs_close,
    .f_read = nffs_read,
    .f_write = nffs_write,

    .f_seek = nffs_seek,
    .f_getpos = nffs_getpos,
    .f_filelen = nffs_file_len,

    .f_unlink = nffs_unlink,
    .f_rename = nffs_rename,
    .f_mkdir = nffs_mkdir,

    .f_opendir = nffs_opendir,
    .f_readdir = nffs_readdir,
    .f_closedir = nffs_closedir,

    .f_dirent_name = nffs_dirent_name,
    .f_dirent_is_dir = nffs_dirent_is_dir,

    .f_name = "nffs"
};

/**
 * Initializes internal nffs memory and data structures.  This must be called
 * before any nffs operations are attempted.
 *
 * @return                  0 on success; nonzero on error.
 */
int
nffs_init(void)
{
    int rc;

    log_register("nffs", &nffs_log, &log_console_handler, NULL, LOG_LEVEL_DEBUG);

    nffs_config_init();

    nffs_cache_clear();

    rc = os_mutex_init(&nffs_mutex);
    if (rc != 0) {
        return FS_EOS;
    }

    free(nffs_file_mem);
    nffs_file_mem = malloc(
        OS_MEMPOOL_BYTES(nffs_config.nc_num_files, sizeof (struct nffs_file)));
    if (nffs_file_mem == NULL) {
        return FS_ENOMEM;
    }

    free(nffs_inode_mem);
    nffs_inode_mem = malloc(OS_MEMPOOL_BYTES(
            nffs_config.nc_num_inodes, sizeof (struct nffs_inode_entry)));
    if (nffs_inode_mem == NULL) {
        return FS_ENOMEM;
    }

    free(nffs_block_entry_mem);
    nffs_block_entry_mem = malloc(OS_MEMPOOL_BYTES(
                nffs_config.nc_num_blocks, sizeof (struct nffs_hash_entry)));
    if (nffs_block_entry_mem == NULL) {
        return FS_ENOMEM;
    }

    free(nffs_cache_inode_mem);
    nffs_cache_inode_mem = malloc(OS_MEMPOOL_BYTES(
                nffs_config.nc_num_cache_inodes,
                sizeof (struct nffs_cache_inode)));
    if (nffs_cache_inode_mem == NULL) {
        return FS_ENOMEM;
    }

    free(nffs_cache_block_mem);
    nffs_cache_block_mem = malloc(OS_MEMPOOL_BYTES(
            nffs_config.nc_num_cache_blocks,
            sizeof (struct nffs_cache_block)));
    if (nffs_cache_block_mem == NULL) {
        return FS_ENOMEM;
    }

    free(nffs_dir_mem);
    nffs_dir_mem = malloc(OS_MEMPOOL_BYTES(
            nffs_config.nc_num_dirs, sizeof (struct nffs_dir)));
    if (nffs_dir_mem == NULL) {
        return FS_ENOMEM;
    }

    rc = nffs_misc_reset();
    if (rc != 0) {
        return rc;
    }

    fs_register(&nffs_ops);
    return 0;
}

void
nffs_pkg_init(void)
{
    /* do nothing */
}

#endif /* !__ZEPHYR__ */
