#ifndef UART_H
#define	UART_H

#ifdef	__cplusplus
extern "C" {
#endif

#define UART_FUNC_RX     (1 << 0)
#define UART_FUNC_TX     (1 << 1)
#define UART_FUNC_TR    (UART_FUNC_TX | UART_FUNC_RX)
#define UART_FUNC       UART_FUNC_TX

void uart_init(long baud_rate);
#if (UART_FUNC & UART_FUNC_RX)
unsigned char serial_getc(void);
#endif
void serial_putc(unsigned char ch);
void serial_puts(const char *s);
void serial_put_hex(unsigned int hex);

#ifdef	__cplusplus
}
#endif

#endif /* UART_H */

