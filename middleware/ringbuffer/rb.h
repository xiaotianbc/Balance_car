#ifndef rb_HDR_H
#define rb_HDR_H

#include <string.h>
#include <stdint.h>


typedef struct {
    uint8_t *buff;   //指向缓冲区的指针
    uint32_t size;   //buffer大小，真正能保存的数据为size-1
    uint32_t r;  //读指针，当r==w时，buffer为空，当w==r-1，缓冲区为满
    uint32_t w;    //写指针，当r==w时，buffer为空，当w==r-1，缓冲区为满
} rb_t;

uint8_t rb_init(rb_t *buff, void *buffdata, uint32_t size);

/* 读写操作*/
uint32_t rb_write(rb_t *buff, const void *data, uint32_t bytes_to_write);

uint32_t rb_read(rb_t *buff, void *data, uint32_t bytes_to_read);

/* 缓冲区数据信息 */
uint32_t rb_get_free_len(rb_t *buff);

uint32_t rb_get_len_can_read(rb_t *buff);


#endif /* rb_HDR_H */
