#include <stdlib.h>
#include <errno.h>

#include <sumeru/cpu/constant.h>
#include <sumeru/cpu/csr.h>
#include <sumeru/cpu/memctl.h>
#include <sumeru/constant.h>

#include "device.h"
#include "uart.h"
#include "consprod.h"
#include "timer.h"

#define MAX_UART_DEVICES	3

#define MIN(a,b)        (a <= b ? a : b)

typedef struct uart_softc 
{
    unsigned int		sc_unit;
    volatile unsigned int	sc_io_pending;
    unsigned int 		sc_flags;
    unsigned int 		sc_rx_lastpos;

    register_read_fn_t		sc_read_reg_tx_ctrl;
    register_write_fn_t		sc_write_reg_tx_ctrl;
    register_write_fn_t		sc_write_reg_tx_baud;

    register_read_fn_t		sc_read_reg_rx_ctrl;
    register_write_fn_t		sc_write_reg_rx_ctrl;
    register_write_fn_t		sc_write_reg_rx_baud;

    consprod_t      		sc_tx_cp;
    consprod_t      		sc_rx_cp;

} uart_softc_t;


#define DEFINE_UART_CSR_FN(unit) \
    static unsigned int unit ## _read_reg_tx_ctrl() \
    { \
	return csr_ ## unit ## _tx_read(); \
    } \
    \
    static void unit ## _write_reg_tx_ctrl(unsigned int v) \
    { \
	csr_ ## unit ## _tx_write(v); \
    } \
    \
    static void unit ## _write_reg_tx_baud(unsigned int v) \
    { \
	csr_ ## unit ## _tx_baud_write(v); \
    } \
    \
    \
    static unsigned int unit ## _read_reg_rx_ctrl() \
    { \
	return csr_ ## unit ## _rx_read(); \
    } \
    \
    static void unit ## _write_reg_rx_ctrl(unsigned int v) \
    { \
	csr_ ## unit ## _rx_write(v); \
    } \
    \
    static void unit ## _write_reg_rx_baud(unsigned int v) \
    { \
	csr_ ## unit ## _rx_baud_write(v); \
    }

DEFINE_UART_CSR_FN(uart0);
DEFINE_UART_CSR_FN(uart1);
DEFINE_UART_CSR_FN(uart2);

static uart_softc_t uart_softc_array[MAX_UART_DEVICES] = 
{
    { 
	0, 0, 0, 0,
	uart0_read_reg_tx_ctrl, uart0_write_reg_tx_ctrl,
	uart0_write_reg_tx_baud,
	uart0_read_reg_rx_ctrl, uart0_write_reg_rx_ctrl,
	uart0_write_reg_rx_baud,
	{
	    (char *)UART0_TX_BUF_START, 
	    (char *)UART0_TX_BUF_END, 
	    (char *)(UART0_TX_BUF_START + 1), 
	    (char *)UART0_TX_BUF_START
	},
	{
	    (char *)UART0_RX_BUF_START, 
	    (char *)UART0_RX_BUF_END, 
	    (char *)(UART0_RX_BUF_START + 1), 
	    (char *)UART0_RX_BUF_START
	},
    },
    { 
	1, 0, 0, 0,
	uart1_read_reg_tx_ctrl, uart1_write_reg_tx_ctrl,
	uart1_write_reg_tx_baud,
	uart1_read_reg_rx_ctrl, uart1_write_reg_rx_ctrl,
	uart1_write_reg_rx_baud,
	{
	    (char *)UART1_TX_BUF_START, 
	    (char *)UART1_TX_BUF_END, 
	    (char *)(UART1_TX_BUF_START + 1), 
	    (char *)UART1_TX_BUF_START
	},
	{
	    (char *)UART1_RX_BUF_START, 
	    (char *)UART1_RX_BUF_END, 
	    (char *)(UART1_RX_BUF_START + 1), 
	    (char *)UART1_RX_BUF_START
	},
    },
    { 
	2, 0, 0, 0,
	uart2_read_reg_tx_ctrl, uart2_write_reg_tx_ctrl,
	uart2_write_reg_tx_baud,
	uart2_read_reg_rx_ctrl, uart2_write_reg_rx_ctrl,
	uart2_write_reg_rx_baud,
	{
	    (char *)UART2_TX_BUF_START, 
	    (char *)UART2_TX_BUF_END, 
	    (char *)(UART2_TX_BUF_START + 1), 
	    (char *)UART2_TX_BUF_START
	},
	{
	    (char *)UART2_RX_BUF_START, 
	    (char *)UART2_RX_BUF_END, 
	    (char *)(UART2_RX_BUF_START + 1), 
	    (char *)UART2_RX_BUF_START
	},
    },
};


#define F_PREAMBLE \
    uart_softc_t *sc; \
    if (unit >= MAX_UART_DEVICES) { \
	errno = ENOENT; \
	return -1; \
    } \
    sc = uart_softc_array + unit


static void uart_poll();

int
uart_subsystem_start()
{
    timer_set_poller(0, uart_poll);
    return 0;
}


int
uart_subsystem_stop(unsigned int unit)
{
    timer_set_poller(0, 0);
    return 0;
}


int
uart_rx_update(unsigned int unit)
{
    F_PREAMBLE;

    unsigned int x = sc->sc_read_reg_rx_ctrl();
    x = (x >> 8) + (x & 0xff);
    if (x != sc->sc_rx_lastpos) {
	flush_dcache_range((char *)sc->sc_rx_lastpos, (char*) x);
	sc->sc_rx_lastpos = x;
    	if (x == (unsigned int)consprod_get_buffer_end(&sc->sc_rx_cp))
 	    x = (unsigned int)consprod_get_buffer_start(&sc->sc_rx_cp);
	consprod_set_prod(&sc->sc_rx_cp, (char *)x);
    }
    return 0;
}

    
int
uart_tx_update(unsigned int unit)
{
    F_PREAMBLE;

    unsigned int x = sc->sc_read_reg_tx_ctrl();
    x = (x >> 8) + (x & 0xff);
    if (x == (unsigned int)consprod_get_buffer_end(&sc->sc_tx_cp))
	x = (unsigned int)consprod_get_buffer_start(&sc->sc_tx_cp);
    consprod_set_cons(&sc->sc_tx_cp, (char *)x);
    return 0;
}


int
uart_rx_start(unsigned int unit)
{
    int free;
    F_PREAMBLE;

    if (consprod_get_prod(&sc->sc_rx_cp) > consprod_get_cons(&sc->sc_rx_cp)) 
    {
	free = consprod_get_buffer_end(&sc->sc_rx_cp) - 
				consprod_get_prod(&sc->sc_rx_cp);
	if (consprod_get_cons(&sc->sc_rx_cp) == 
				consprod_get_buffer_start(&sc->sc_rx_cp))
	    free = free - 1;
    } else {
	free = consprod_get_cons(&sc->sc_rx_cp) - 
		    		consprod_get_prod(&sc->sc_rx_cp) - 1;
    }

    if (free > 0) {
	free = MIN(free, 255);
	sc->sc_flags |= UART_FLAG_RX_STARTED;
	sc->sc_rx_lastpos = (unsigned int)consprod_get_prod(&sc->sc_rx_cp);
	sc->sc_write_reg_rx_ctrl(
	    ((unsigned int)consprod_get_prod(&sc->sc_rx_cp) << 8) | free);
    }

    return 0;
}


int
uart_tx_start(unsigned int unit)
{
    int free;
    F_PREAMBLE;

    if (consprod_get_cons(&sc->sc_tx_cp) > consprod_get_prod(&sc->sc_tx_cp)) {
	free = consprod_get_buffer_end(&sc->sc_tx_cp) -
			consprod_get_cons(&sc->sc_tx_cp);
	if (consprod_get_prod(&sc->sc_tx_cp) ==  
			consprod_get_buffer_start(&sc->sc_tx_cp))
	    free = free - 1;
    } else {
	free = consprod_get_prod(&sc->sc_tx_cp) - 
		    		consprod_get_cons(&sc->sc_tx_cp) - 1;
    }

    if (free > 0) {
	free = MIN(free, 255);
	flush_dcache_range(
	    consprod_get_cons(&sc->sc_tx_cp), 
	    consprod_get_cons(&sc->sc_tx_cp) + free);
	sc->sc_flags |= UART_FLAG_TX_STARTED;
	sc->sc_write_reg_tx_ctrl(
	    ((unsigned int)consprod_get_cons(&sc->sc_tx_cp) << 8) | free);
    }

    return 0;
}


int
uart_peek(unsigned int unit)
{
    F_PREAMBLE;
    return consprod_peek(&sc->sc_tx_cp);
}


static void 
uart_tx_intr_handler(uart_softc_t *sc)
{
    uart_tx_update(sc->sc_unit);
    sc->sc_flags &= ~UART_FLAG_TX_STARTED;
    uart_tx_start(sc->sc_unit);
}


static void 
uart_rx_intr_handler(uart_softc_t *sc)
{
    uart_rx_update(sc->sc_unit);
    sc->sc_flags &= ~UART_FLAG_RX_STARTED;
    uart_rx_start(sc->sc_unit);
}


int
dev_alloc_uart(unsigned int unit, 
		intr_dispatch_entry_t *e_tx,
		intr_dispatch_entry_t *e_rx)
{
    F_PREAMBLE;

    e_tx->handler = (device_intr_handler_t) uart_tx_intr_handler;
    e_tx->act = sc;
    e_rx->handler = (device_intr_handler_t) uart_rx_intr_handler;
    e_rx->act = sc;

    return 0;
}


int
uart_read(unsigned int unit, char *buf, unsigned int len)
{
    F_PREAMBLE;
    return consprod_read(&sc->sc_rx_cp, buf, len);
}


int
uart_write(unsigned int unit, const char *buf, unsigned int len)
{
    F_PREAMBLE;
    return consprod_write(&sc->sc_tx_cp, buf, len);
}


static void
uart_poll()
{
    uart_softc_t *sc = uart_softc_array;

    for (unsigned int i = 0; i < MAX_UART_DEVICES; ++i, ++sc) {
	if ((sc->sc_flags & UART_FLAG_RX_STARTED) == 0)
	    uart_rx_start(i);
	if ((sc->sc_flags & UART_FLAG_TX_STARTED) == 0)
	    uart_tx_start(i);
    }
}


/*
 * MAX BAUD PERIOD = 2 MHz - due to 16 tick rx start reg in cpu uart
 * MIN BAUD IS = 2048 - due to cpu rx baud regs being 16 bit wide
 */

int
uart_set_baud(unsigned int unit, unsigned int baud)
{
    F_PREAMBLE;

    if (baud < 2048 || baud > (1024 * 1024 * 2)) {
	errno = EINVAL;
	return 0;
    }

    unsigned int tx_baud = (CPU_CLK_FREQ / baud + 1) / 2;
    unsigned int rx_baud_a = tx_baud * 3 - 16;
    unsigned int rx_baud_b = tx_baud * 2;

    sc->sc_write_reg_tx_baud(tx_baud);
    sc->sc_write_reg_rx_baud(rx_baud_a | (rx_baud_b << 16));

    return 0;
}
