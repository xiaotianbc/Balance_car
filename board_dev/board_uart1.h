//
// Created by xiaotian on 2022/9/25.
//


#ifndef F401_BALANCE_CAR_BOOTLOADER_BOARD_UART1_H
#define F401_BALANCE_CAR_BOOTLOADER_BOARD_UART1_H






void board_uart1_init(u32 baudrate);
void board_uart1_send_char(char c);
void board_uart1_send_string(const char *s);
uint8_t board_uart1_get_char(char *c);

void uart1_datapack_handler();

void handle_data_pack(uint8_t *buf, uint32_t len);

void handle_error_pack(void);

void uart_data_pack_send(uint8_t *buffer_to_send, uint32_t len);

#endif //F401_BALANCE_CAR_BOOTLOADER_BOARD_UART1_H
