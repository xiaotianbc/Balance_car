#include "rb.h"


#define Min(x, y)                   ((x) < (y) ? (x) : (y))

/**
 * 初始化缓冲区
 * @param buff 缓冲区句柄
 * @param buffdata 指向缓冲区的内存地址
 * @param size 缓冲区大小
 * @return 成功返回1
 */
uint8_t rb_init(rb_t *buff, void *buffdata, uint32_t size) {
    if (buff == NULL || buffdata == NULL || size == 0) {
        return 0;
    }
    memset((void *) buff, 0x00, sizeof(*buff));

    buff->size = size;
    buff->buff = buffdata;

    return 1;
}


//定义一个宏，在free等操作之前，检查操作是否合法，避免空指针异常
#define rb_is_ready(b)                 ((b) != NULL && (b)->buff != NULL && (b)->size > 0)


/**
 * rb写数据
 * @param buff 要写入的rb句柄
 * @param data 要写入的数据地址
 * @param bytes_to_write 要写入的数据长度
 * @return 真实写入的长度，如果返回值小于bytes_to_write，说明缓冲区长度不够了
 */
uint32_t rb_write(rb_t *buff, const void *data, uint32_t bytes_to_write) {
    uint32_t tocopy, free;
    volatile uint32_t buff_w_ptr;
    const uint8_t *d = data;

    //防御性操作
    if (!rb_is_ready(buff) || data == NULL || bytes_to_write == 0) {
        return 0;
    }

    /* 计算最大可以写入的数据长度 */
    free = rb_get_free_len(buff);

    //只能写入最大可以被读取的长度与想要写入长度中的最小值
    bytes_to_write = Min(free, bytes_to_write);
    if (bytes_to_write == 0) { //如果不能写入，直接退出
        return 0;
    }


    buff_w_ptr = buff->w;
    //计算从当前的写指针写到缓冲区的末端，总共能写多少个字节
    tocopy = Min(buff->size - buff_w_ptr, bytes_to_write);
    //把数据以当前写指针的位置开始拷贝
    memcpy(&buff->buff[buff_w_ptr], d, tocopy);

    buff_w_ptr += tocopy; //
    bytes_to_write -= tocopy; //想要写的总数据量，减去已经写进去的数据量

    //判断是否已经写完数据了？如果没有写完，需要从缓冲区头(buff->buff)开始写
    if (bytes_to_write > 0) {
        memcpy(buff->buff,//起始地址是缓冲区的头
               &d[tocopy],//因为之前的已经写过了，要写入的内容后移tocopy个长度。
               bytes_to_write //剩下的长度，写进去
        );
        //因为是从数据头开始写bytes_to_write个数据，所以现在的写指针应该指向bytes_to_write
        buff_w_ptr = bytes_to_write;
    }

    //如果当前写指针正好指向缓冲区的末尾，则让他变成从0开始
    if (buff_w_ptr >= buff->size) {
        buff_w_ptr = 0;
    }

    buff->w = buff_w_ptr;//把结构体的写指针更新

    return tocopy + bytes_to_write; //返回总共写入的数据长度
}


/**
 * 环形缓冲区读取
 * @param buff 句柄
 * @param data 读出来的数据要放入的地址
 * @param bytes_to_read 要读取的字节数
 * @return 实际读取成功的字节数
 */
uint32_t rb_read(rb_t *buff, void *data, uint32_t bytes_to_read) {
    uint32_t tocopy, full;
    volatile uint32_t buff_r_ptr;
    uint8_t *d = data;

    //防御性操作
    if (!rb_is_ready(buff) || data == NULL || bytes_to_read == 0) {
        return 0;
    }

    /* 计算最大可以读取的数据长度 */
    full = rb_get_len_can_read(buff);
    bytes_to_read = Min(full, bytes_to_read);
    if (bytes_to_read == 0) { //如果不能读取，直接退出
        return 0;
    }


    buff_r_ptr = buff->r;
    //计算从当前读指针的位置开始读，最多能直接读取多少个字节
    tocopy = Min(buff->size - buff_r_ptr, bytes_to_read);
    memcpy(d, &buff->buff[buff_r_ptr], tocopy);

    //想要读的总数据量，减去已经读的数据量
    buff_r_ptr += tocopy;
    bytes_to_read -= tocopy;

    //判断想要读取的数据有没有读取完，
    if (bytes_to_read > 0) {
        //如果没用读取完，需要从头开始读取
        memcpy(&d[tocopy],//写入的地址跳过已经读取时个数
               buff->buff, //读取的地址从缓冲区头开始
               bytes_to_read);
        //因为是从数据头开始读bytes_to_read个数据，所以现在的读指针应该指向bytes_to_read
        buff_r_ptr = bytes_to_read;
    }

    //如果当前读指针正好指向缓冲区的末尾，则让他变成从0开始
    if (buff_r_ptr >= buff->size) {
        buff_r_ptr = 0;
    }


    buff->r = buff_r_ptr; //把结构体的读指针更新
    return tocopy + bytes_to_read; //返回总共读取的数据长度
}

/**
 * 获取缓冲区剩余长度
 * @param buff 缓冲区句柄
 * @return 剩余长度
 */
uint32_t rb_get_free_len(rb_t *buff) {
    uint32_t size;
    volatile uint32_t w, r;

    if (!rb_is_ready(buff)) {
        return 0;
    }

    /* 使用零时变量，避免操作过程中进入中断，缓冲区被修改*/
    w = buff->w;
    r = buff->r;
    if (w == r) {
        size = buff->size;
    } else if (r > w) {
        size = r - w;
    } else {
        size = buff->size - (w - r);
    }

    /*缓冲区可用长度实际要少1（不可能写满） */
    return size - 1;
}

/**
 * 获取缓冲区里可以读取的长度
 * @return 可以读取的长度
 */
uint32_t rb_get_len_can_read(rb_t *buff) {
    uint32_t size;
    volatile uint32_t w, r;

    if (!rb_is_ready(buff)) {
        return 0;
    }

    /* 使用零时变量，避免操作过程中进入中断，缓冲区被修改*/
    w = buff->w;
    r = buff->r;
    if (w == r) {
        size = 0;
    } else if (w > r) {
        size = w - r;
    } else {
        size = buff->size - (r - w);
    }
    return size;
}
