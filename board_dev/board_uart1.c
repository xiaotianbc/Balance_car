//
// Created by xiaotian on 2022/9/25.
//
#include "main.h"

#include "stm32f4xx_dma.h"

//串口1DMA接收缓冲区
//uint8_t usart1_rx_dma_buffer[2048];

//串口1中断接收，环形缓冲区
uint8_t usart1_rx_rxne_rb[2048];
rb_t usart1_rb_handle; //句柄

/**
 * 本文件提供了基于UART1 (PA9-PA10)的串口通信功能
 */


void board_uart1_send_char(char c) {
    /*写入字节到数据寄存器*/
    USART1->DR = c;

    /*等待： TXE：发送数据寄存器为空*/
    while ((USART1->SR & (1 << 7)) == 0);
}

/**
 * 串口发送字符串
 * @param s
 */
void board_uart1_send_string(const char *s) {
    //字符串的末尾是'\0'，如果不是则说明没到末尾
    while (*s != '\0') {
        board_uart1_send_char(*s);
        s++;
    }
}

/**开发板串口1读取一个字节
 * @param c 要写入的字节
 * @return 读取到数据返回1，没有读取到数据返回0
 */
uint8_t board_uart1_get_char(char *c) {
    /*定义一个最大延迟时间，如果这个时间之内没有读取到数据，就退出函数，避免死等*/
    uint32_t max_wait_time = 0xffff;

    /*检查环形缓冲区是否有数据*/
    while (rb_get_len_can_read(&usart1_rb_handle) == 0 && max_wait_time) {
        max_wait_time--;
    }
    //如果max_wait_time等于0，说明超过了最大延迟时间
    if (!max_wait_time)
        return 0;
    //程序走到这里说明有数据准备被读取
    rb_read(&usart1_rb_handle, c, 1);
    return 1;
}


/**
 * 串口1初始化
 * @param baudrate 波特率
 */
void board_uart1_init(u32 baudrate) {
    //GPIO端口设置： TX=PA9, RX=PA10
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    /*使能UART1时钟*/
    RCC->APB2ENR |= (1 << 4);
    /*使能GPIOA时钟*/
    RCC->AHB1ENR |= (1 << 0);

    //然后配置两个GPIO的模式：复用输出和输入
    GPIOA->MODER |= (2 << (9 * 2)); //配置PA9为复用功能模式
    GPIOA->MODER |= (2 << (10 * 2)); //配置PA10为复用功能模式
    GPIOA->OSPEEDR |= 3 << (9 * 2); //配置PA9:TX为高速模式

    //查阅datasheet，可知USART1对于的AF编号为AF7
    //AFR[0]对应端口0到7  AFR[1]对应端口8-15
    GPIOA->AFR[1] |= (7 << (1 * 4)); //配置PA9为AF7 AF7对应的配置是0111b ，十进制为7，左移2*4位

    GPIOA->AFR[1] |= (7 << (2 * 4)); //配置PA10为AF7 AF7对应的配置是0111b ，十进制为7，左移2*4位

    //配置控制寄存器
    USART1->CR1 &= ~(1 << 15); //16 倍过采样

    //配置USART1相关寄存器
    //已知USART1挂在APB2上，APB2的速度是120Mhz,目标波特率为115200,
    USART1->BRR = 120000000 / baudrate;

    USART1->CR1 |= (1 << 3); //使能发送器
    USART1->CR1 |= (1 << 2); //使能接收器

    //调自闭了都没调好，先用中断接收吧
    // USART1->CR3 |= 1 << 6; //针对接收使能 DMA 模式

    // USART1->CR1 |= 1 << 4; //IDLE 中断使能 (IDLE interrupt enable)开启空闲中断
    SETBIT(USART1->CR1, 1 << 5);//RXNE 中断使能 (RXNE interrupt enable)

    //初始化串口中断接收环形缓冲区
    rb_init(&usart1_rb_handle, usart1_rx_rxne_rb, 2048);



    //开启中断
    uint32_t prioritygroup = NVIC_GetPriorityGrouping();
    NVIC_SetPriority(USART1_IRQn, NVIC_EncodePriority(prioritygroup, 2, 2));
    NVIC_EnableIRQ(USART1_IRQn);

    /*配置DMA，查看用户手册可以知道，STM32F4的DMA2-通道4-数据流2为USART1_RX*/
    //这里，，，用寄存器实在调不通，先用标准库试试
#if 0
    RCC->AHB1ENR |= 1 << 22;//DMA2 时钟使能 (DMA2 clock enable)

    DMA2_Stream2->CR &= ~(1 << 0);//禁止数据流，之后才可以对配置和 FIFO 位寄存器编程

    while (DMA2_Stream2->CR & (1 << 0)); //等待数据流就绪标志

    DMA2_Stream2->PAR = (uint32_t) &(USART1->DR); //外设地址 (Peripheral address):USART1->DR
    DMA2_Stream2->M0AR = (uint32_t) usart1_rx_dma_buffer;//读/写数据的存储区 0 的基址
    DMA2_Stream2->NDTR = (uint16_t) 1024;//要传输的数据项数目
    DMA2_Stream2->CR &= ~(7 << 25);//清空通道选择
    DMA2_Stream2->CR |= (4 << 25);//选择通道 4
    // DMA2_Stream2->CR |= 1 << 4;//传输完成中断使能,不使用
    CLRBIT(DMA2_Stream2->CR, 1 << 5);//DMA是流控制器，事实上，只有SDIO支持外设作为流控制器，其他都不用
    DMA2_Stream2->CR &= ~(3 << 6);//数据传输方向:外设到存储器
    DMA2_Stream2->CR &= ~(1 << 8);//禁止循环模式
    DMA2_Stream2->CR &= ~(1 << 9);//外设地址指针固定
    DMA2_Stream2->CR |= (1 << 10);//每次数据传输后，存储器地址指针递增（增量为 MSIZE 值）
    DMA2_Stream2->CR &= ~(3 << 11);//外设数据大小:字节（8 位）
    DMA2_Stream2->CR &= ~(3 << 13);//存储器数据大小:字节（8 位）
    //位 15 PINCOS：外设增量偏移量 ，没有意义，不修改
    DMA2_Stream2->CR |= (3 << 16);//优先级 (Priority level)=非常高
    DMA2_Stream2->CR &= ~(1 << 18);//双缓冲区模式 (Double buffer mode):传输结束时不切换缓冲区
    CLRBIT(DMA2_Stream2->FCR, 1 << 2); //使能直接模式


    DMA2_Stream2->CR |= (1 << 0);//使能数据流
#elif 0
    RCC->AHB1ENR |= 1 << 22;//DMA2 时钟使能 (DMA2 clock enable)
    DMA_DeInit(DMA2_Stream2);
    DMA_Cmd(DMA2_Stream2, DISABLE);
    while (DMA_GetCmdStatus(DMA2_Stream2) == ENABLE);
    DMA_InitTypeDef DMA_InitStruct = {
            .DMA_BufferSize=1024,
            .DMA_Channel=DMA_Channel_4,
            .DMA_DIR=DMA_DIR_PeripheralToMemory, //外设到内存
            .DMA_FIFOMode=DMA_FIFOMode_Disable, //不用FIFO
            .DMA_Memory0BaseAddr=(uint32_t) usart1_rx_dma_buffer,
            .DMA_MemoryBurst=DMA_MemoryBurst_Single, //单次突发传输
            .DMA_MemoryDataSize=DMA_MemoryDataSize_Byte,
            .DMA_MemoryInc=DMA_MemoryInc_Enable, //内存地址自增
            .DMA_Mode=DMA_Mode_Normal, //单次还是循环
            .DMA_PeripheralBaseAddr=(uint32_t) (&(USART1->DR)),
            .DMA_PeripheralBurst=DMA_PeripheralBurst_Single,
            .DMA_PeripheralDataSize=DMA_PeripheralDataSize_Byte,
            .DMA_PeripheralInc=DMA_PeripheralInc_Disable, //外设地址不增加
            .DMA_Priority=DMA_Priority_High
    };
    DMA_Init(DMA2_Stream2, &DMA_InitStruct);
    DMA_Cmd(DMA2_Stream2, ENABLE);
#endif
    //  board_uart1_send_string("_stm32F4:uart1 inited\n");



    USART1->CR1 |= (1 << 13); //使能，全部配置完成后开启
}


void USART1_IRQHandler(void) {
#if 0
    //统计共传输了多少个字节的数据
    uint32_t dma_trans_count = 0;
    extern void uart_data_pack_send(uint8_t *buffer_to_send, uint32_t len);

    /*判断是不是进入空闲中断*/
    if (USART1->SR & (1 << 4)) {
        volatile uint8_t data = USART1->SR;
        data = USART1->DR;// 软件序列清除IDLE标志位

        //   DMA2_Stream2->CR&= ~(1 << 0);//使能数据流
        dma_trans_count = 1024 - DMA2_Stream2->NDTR;

        if (dma_trans_count > 0) {
            //    uart_data_pack_send(usart1_rx_dma_buffer,dma_trans_count);
            board_uart1_send_char(usart1_rx_dma_buffer[0]);
        }
    }
#endif
    if (USART1->SR & (1 << 5)) {
        uint8_t data = USART1->DR;
        rb_write(&usart1_rb_handle, &data, 1);

        if (data == '\n') {

        }
    }
}

void DMA2_Stream2_IRQHandler(void) {

}

/*接收相关逻辑*/

//定义一个缓冲区，用来接收电脑发过来的字符串
uint8_t buffer[2048] = {0};
uint32_t buffer_index = 0;


/**
 * 使用有限状态机；来接收数据包，分为两个字节包头(0x55, 0xAA)，4个字节包长度，真实数据载荷，校检值
 * 校验值采用异或校检，为数据包所有数据一起异或之后的结果
 * 本来想再加个包尾的，没有难度，但是因为考虑到单片机计算校检值可能需要花费一些时间，怕出问题，就暂时省略
 */

typedef enum {
    Package_Head,  //包头(0x55, 0xAA)
    Package_Length,//包长度
    Data_Body,     //真实数据载荷
    Data_Verify,   //校检值
} DataPack_FSM;

//首先一开始的状态是准备接收包头
DataPack_FSM status = Package_Head;


//串口数据包接收执行函数，从环形缓冲区里取出数据然后处理
void uart1_datapack_handler(void) {
    static char recv_char = 0;

    static uint32_t data_len = 0;   //用来存储数据包的真实长度
    static uint8_t data_len_recv_flag = 0;  //当正在接收数据包长度时，用来存储已经接收到的flag

    /*判断是否收到数据，如果收到数据再处理*/
    while (board_uart1_get_char(&recv_char)) {
        //判断当前状态
        switch (status) {
            case Package_Head:  //如果是正在等待读取包头
                if (recv_char == 0x55) {
                    //包头共有两个字节，0x55, 0xAA，如果收到的是0x55，就继续读取下一个字节
                    while (!board_uart1_get_char(&recv_char));
                    //成功读取到字节后，判断这个字节是不是0xAA，
                    // 如果不是，说明不是包头，continue然后等下个字节，如果是，就准备接收包长
                    if (recv_char != 0xAA)
                        continue;
                    status = Package_Length;
                    data_len = 0; //开始接收包长之前，先清空
                    data_len_recv_flag = 0;//开始接收包长之前，先清空
                }
                break;
            case Package_Length:
                //data_len_recv_flag一开始是0，接收长度时第一个是最低位，然后依次接收高位
                data_len |= recv_char << data_len_recv_flag;
                data_len_recv_flag++;
                //如果data_len_recv_flag == 2，说明已经接收完2此包长了，准备开始接收数据包
                if (data_len_recv_flag == 2) {
                    status = Data_Body;
                    //清空上次接收到的数据包，准备下一次接收啦
                    memset(buffer, 0, buffer_index);
                    buffer_index = 0;
                }
                break;
            case Data_Body:
                //在接收数据包状态时，每次接收到的数据都放在buffer里
                buffer[buffer_index] = recv_char;
                buffer_index++;
                //如果buffer_index == data_len，说明数据已经收完了，进入校检状态
                if (buffer_index == data_len)
                    status = Data_Verify;
                break;
            case Data_Verify:
                /*
                 * 校检位，这里使用了异或校检：把数据包里的所有数据进行异或，最后获得的结果作为校检位发送
                 * 我们把收到的数据再和数据包里所有数据异或一遍，如果数据是正确的，那么就结果肯定是0
                 * 具体解释可以上网搜索关键词：BCC异或校验
                 */
                //把接收到的这个数据和buffer里所有数据全部异或一遍
                for (int i = 0; i < data_len; ++i) {
                    recv_char ^= buffer[i];
                }
                //如果结果是0，说明接收数据完全正确，进入函数处理数据包
                if (recv_char == 0) {
                    handle_data_pack(buffer, data_len);
                } else {
                    //如果不对，说明过程出现了错误，这里专门建一个错误处理函数
                    handle_error_pack();
                }
                break;
            default:
                break;
        }

    }
}


void handle_data_pack(uint8_t *buf, uint32_t len) {
    uart_data_pack_send(buf, len);

//    for (int i = 0; i < len; ++i) {
//        board_uart1_send_char(buf[i]);
//    }
//    board_uart1_send_string(" over \n");
    status = Package_Head; //重新回去从包头开始接收
}

void handle_error_pack(void) {
    board_uart1_send_string("There are error in UART data pack received !!!\n");
    status = Package_Head; //重新回去从包头开始接收
}

//发送数据缓冲区
uint8_t send_buffer[2048] = {0};


/**
 * 数据包发送
 * @param buffer_to_send 准备要发送的数据包
 * @param len   准备发送的数据长度
 */
void uart_data_pack_send(uint8_t *buffer_to_send, uint32_t len) {
    memset(send_buffer, 0, sizeof(send_buffer));
    send_buffer[0] = 0x55;
    send_buffer[1] = 0xAA;        //前两个字节是包头
    send_buffer[2] = (len >> 0) & 0xff;  //3-6四个字节是数据包长度，采用小端在前格式，先发的是低位
    send_buffer[3] = (len >> 8) & 0xff;
    memcpy(send_buffer + 4, buffer_to_send, len); //把数据包从原位置拷贝到发送缓冲区的第7个位置开始

    uint8_t xor_verify = 0;
    for (int i = 0; i < len; ++i) {
        xor_verify ^= buffer_to_send[i];
    }
    send_buffer[4 + len] = xor_verify;   //最后一个字节放异或校验位

    //然后只要把所有的数据全发过去就完事了
    for (int i = 0; i < 5 + len; ++i) {
        board_uart1_send_char(send_buffer[i]);
    }
}