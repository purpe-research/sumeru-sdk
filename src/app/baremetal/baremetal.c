#include <sumeru/cpu/constant.h>
#include <sumeru/cpu/csr.h>
#include <sumeru/cpu/memctl.h>

/* Interrupt signaling flags */

volatile unsigned int g_uart0_tx_intr_pending;
volatile unsigned int g_uart0_rx_intr_pending;


/* UART buffers must be placed within the first 8MB of RAM */

static const unsigned int g_uart0_tx_buffer_loc = 0x100;
static const unsigned int g_uart0_rx_buffer_loc = 0x200;

/*
 * Read upto 255 bytes from uart0.
 * Routine will block till all bytes are read.
 *
 *
 * Note: In normal, non-baremetal-mode aka. c-runtime mode, 
 * reads & writes are done asynchronously and therefore
 * calls will not block if data / free-space is 
 * available in the device receive / transmit buffer.
 *
 * The ISR routine in entry.S will signal the
 * occurance of an interrupt to this code via
 * a flag.
 */

int
uart0_read(char *buf, unsigned int len)
{
    char *rx_buf = (char *)g_uart0_rx_buffer_loc;

    len &= 0xff;

    g_uart0_rx_intr_pending = 1;
    csr_uart0_rx_write((g_uart0_rx_buffer_loc << 8) | len);
    while (g_uart0_rx_intr_pending == 1)
        ;

    /* Copy data from Receive buffer, flushing cache lines */
    for (unsigned int i = 0; i < len; ++i, buf++, rx_buf++) {
        if ((i & 0xf) == 0) {
            flush_dcache_line((unsigned int)rx_buf);
        }
        *buf = *rx_buf;
    }
        
    return 0;
}


/*
 * Write upto 255 bytes to uart0.
 * Routine will block till all bytes are written.
 *
 * Note: In normal, non-baremetal-mode aka. c-runtime mode, 
 * reads & writes are done asynchronously and therefore
 * calls will not block if data / free-space is 
 * available in the device receive / transmit buffer.
 * 
 * The ISR routine in entry.S will signal the
 * occurance of an interrupt to this code via
 * a flag.
 */

int
uart0_write(char *buf, unsigned int len)
{
    char *tx_buf = (char *)g_uart0_tx_buffer_loc;

    len &= 0xff;

    /* Copy data to Trasmit buffer, flushing cache lines */
    for (unsigned int i = 0; i < len; ++i, buf++, tx_buf++) {
        *tx_buf = *buf;
        if ((i & 0xf) == 0xf) {
            flush_dcache_line(((unsigned int)tx_buf) & 0xfffffff0);
        }
    }

    flush_dcache_line(((unsigned int)tx_buf) & 0xfffffff0);

    g_uart0_tx_intr_pending = 1;
    csr_uart0_tx_write((g_uart0_tx_buffer_loc << 8) | len);
    while (g_uart0_tx_intr_pending == 1)
        ;
        
    return 0;
}

static char *msg = "Hello World!\n";

int
main(void)
{
    /* Set LED GPIO (bit 1) line direction to output, turn led-off (1) */
    /* LED will be blinked by timer interrupt handler */

    csr_gpio_dir_set(1);
    csr_gpio_out_set(1);

    /* Generate 50% duty cycle, frequency = 140.625 KHz, PWM signal on 
     * all three pwm units (0, 1, 2)
     */
    csr_pwm_write(0xc0c0c081);

    /* Enable timer ISR */
    /* For timing calculations see sumeru/constant.h */
    /* 65536000 = ~900 ms */

    csr_timer0_ctrl_write(65536000 | 0xf);

    while (1) {
	uart0_write(msg, 13);

	/* delay for a bit */
	for (int i = 0; i < 10485760; ++i)
	    ;
    }

    /* Not reached */
    return 0;
}
