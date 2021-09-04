#ifndef __SUMERU_DEV_UART_H
#define __SUMERU_DEV_UART_H

#define UART_FLAG_RX_STARTED            (1 << 1)
#define UART_FLAG_TX_STARTED            (1 << 2)

int	uart_rx_update(unsigned int unit);
int	uart_tx_update(unsigned int unit);

int	uart_rx_start(unsigned int unit);
int	uart_tx_start(unsigned int unit);
int	uart_peek(unsigned int unit);

int	uart_read(unsigned int unit, char *buf, unsigned int len);
int	uart_write(unsigned int unit, const char *buf, unsigned int len);


/* TX and RX baud are set to be the same, the hardware
 * allows them to be different if required.
 */

int	uart_set_baud(unsigned int unit, unsigned int baud);

#endif
