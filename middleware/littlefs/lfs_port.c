//
// Created by xiaotian on 2022/12/25.
//

#include "lfs_port.h"
#include "lfs.h"
#include "sfud.h"

static const sfud_flash *f0;

void lfs_port_init(void) {
    if (f0 == NULL) {
        if (sfud_init() != SFUD_SUCCESS) {
            printf("init failed\n");
            return;
        }
        f0 = sfud_get_device_table();
    }
    return;
}


/*
 * @brief littlefs read interface
 * @param [in] c lfs_config数据结构
 * @param [in] block 要读的块
 * @param [in] off 在当前块的偏移
 * @param [out] buffer 读取到的数据
 * @param [in] size 要读取的字节数
 * @return 0 成功 <0 错误
 * @note littlefs 一定不会存在跨越块存储的情况
 */
int W25Qxx_readLittlefs(const struct lfs_config *c, lfs_block_t block,
                        lfs_off_t off, void *buffer, lfs_size_t size) {

    if (block >= W25Q128_NUM_GRAN) //error
    {
        return LFS_ERR_IO;
    }
    if (f0 == NULL)
        return -1;

    sfud_read(f0, block * W25Q128_ERASE_GRAN + off, size, buffer);
    return LFS_ERR_OK;
}



/*
 * @brief littlefs write interface
 * @param [in] c lfs_config数据结构
 * @param [in] block 要读的块
 * @param [in] off 在当前块的偏移
 * @param [out] buffer 读取到的数据
 * @param [in] size 要读取的字节数
 * @return 0 成功 <0 错误
 * @note littlefs 一定不会存在跨越块存储的情况
 */
int W25Qxx_writeLittlefs(const struct lfs_config *c, lfs_block_t block,
                         lfs_off_t off,const void *buffer, lfs_size_t size) {

    if (block >= W25Q128_NUM_GRAN) //error
    {
        return LFS_ERR_IO;
    }

    if (f0 == NULL)
        return -1;

    sfud_write(f0, block * W25Q128_ERASE_GRAN + off, size, buffer);
    return LFS_ERR_OK;
}


/*
 * @brief littlefs 擦除一个块
 * @param [in] c lfs_config数据结构
 * @param [in] block 要擦出的块
 * @return 0 成功 <0 错误
 */
int W25Qxx_eraseLittlefs(const struct lfs_config *c, lfs_block_t block)
{

    if(block >= W25Q128_NUM_GRAN) //error
    {
        return LFS_ERR_IO;
    }
    if (f0 == NULL)
        return -1;
    sfud_erase(f0,block * W25Q128_ERASE_GRAN,W25Q128_ERASE_GRAN);
    return  LFS_ERR_OK;
}

int W25Qxx_syncLittlefs(const struct lfs_config *c )
{
    return  LFS_ERR_OK;
}