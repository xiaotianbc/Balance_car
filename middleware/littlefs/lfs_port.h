//
// Created by xiaotian on 2022/12/25.
//

#ifndef F401_BALANCE_CAR_BOOTLOADER_LFS_PORT_H
#define F401_BALANCE_CAR_BOOTLOADER_LFS_PORT_H

#include "lfs.h"

#define W25Q128_ERASE_GRAN              4096
#define W25Q128_NUM_GRAN                2048    //block数量

void lfs_port_init(void);

/* littlefs操作系统接口 */

int W25Qxx_readLittlefs(const struct lfs_config *c, lfs_block_t block,
                        lfs_off_t off, void *buffer, lfs_size_t size);


int W25Qxx_writeLittlefs(const struct lfs_config *c, lfs_block_t block,
                         lfs_off_t off,const void *buffer, lfs_size_t size);

int W25Qxx_eraseLittlefs(const struct lfs_config *c, lfs_block_t block);


int W25Qxx_syncLittlefs(const struct lfs_config *c );



#endif //F401_BALANCE_CAR_BOOTLOADER_LFS_PORT_H
