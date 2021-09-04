#include <errno.h>

#include <sumeru/cpu/csr.h>
#include <sumeru/cpu/memctl.h>
#include <sumeru/constant.h>

#include "device.h"
#include "spi.h"

#define MAX_SPI_DEVICES		5

typedef struct spi_softc 
{
    unsigned int		sc_unit;
    spi_user_intr_callback_t	sc_cb;
    volatile unsigned int	sc_io_pending;
    

    register_write_fn_t		sc_write_reg_ctrl1;
    register_write_fn_t		sc_write_reg_ctrl2;

    char			*sc_last_buf;
    unsigned int		sc_last_len;
    unsigned char		sc_clk_sel;
    unsigned char		sc_last_aux;

} spi_softc_t;


#define DEFINE_UART_CSR_FN(unit) \
    static void unit ## _write_reg_ctrl1(unsigned int v) \
    { \
	return csr_ ## unit ## _ctrl_1_write(v); \
    } \
    \
    static void unit ## _write_reg_ctrl2(unsigned int v) \
    { \
	return csr_ ## unit ## _ctrl_2_write(v); \
    }


DEFINE_UART_CSR_FN(spi0);
DEFINE_UART_CSR_FN(spi1);
DEFINE_UART_CSR_FN(spi2);
DEFINE_UART_CSR_FN(spi3);
DEFINE_UART_CSR_FN(spi4);

static spi_softc_t spi_softc_array[MAX_SPI_DEVICES] = 
{
    { 
	0, 0, 0,
	spi0_write_reg_ctrl1, spi0_write_reg_ctrl2,
	0, 0, SPI_CLK_SEL_36M, 0
    },
    { 
	1, 0, 0,
	spi1_write_reg_ctrl1, spi1_write_reg_ctrl2,
	0, 0, SPI_CLK_SEL_36M, 0
    },
    { 
	2, 0, 0,
	spi2_write_reg_ctrl1, spi2_write_reg_ctrl2,
	0, 0, SPI_CLK_SEL_36M, 0
    },
    { 
	3, 0, 0,
	spi3_write_reg_ctrl1, spi3_write_reg_ctrl2,
	0, 0, SPI_CLK_SEL_36M, 0
    },
    { 
	4, 0, 0,
	spi4_write_reg_ctrl1, spi4_write_reg_ctrl2,
	0, 0, SPI_CLK_SEL_36M, 0
    }
};


static void 
spi_intr_handler(spi_softc_t *sc)
{
    sc->sc_io_pending = 0;
    if (sc->sc_cb)
	(*sc->sc_cb)(sc->sc_unit);
}


#define F_PREAMBLE \
    spi_softc_t *sc; \
    if (unit >= MAX_SPI_DEVICES) { \
	errno = ENOENT; \
	return -1; \
    } \
    sc = spi_softc_array + unit


int
spi_set_user_intr_callback(unsigned int unit, spi_user_intr_callback_t h)
{
    F_PREAMBLE;
    sc->sc_cb = h;
    return 0;
}


int
spi_set_speed(unsigned int unit, unsigned char clk_sel)
{
    F_PREAMBLE;
    sc->sc_clk_sel = clk_sel;
    return 0;
}


int
dev_alloc_spi(unsigned int unit, intr_dispatch_entry_t *e)
{
    F_PREAMBLE;
    e->handler = (device_intr_handler_t) spi_intr_handler;
    e->act = sc;
    return 0;
}


int
spi_async_transceive(
            unsigned int unit,
            char *buf, unsigned int len, 
            unsigned char aux)
{
    F_PREAMBLE;
    len &= 0xfffff;
    flush_dcache_range(buf, buf + len);
    /* aux(0,1) - cs_states, aux(2) - wronly */
    aux &= 0x7;
    sc->sc_last_aux = aux;
    sc->sc_last_buf = buf;
    sc->sc_last_len = len;
    sc->sc_write_reg_ctrl1(((unsigned int)buf) | 
	    			(sc->sc_clk_sel << SPI_CLKSEL_SHIFT));
    sc->sc_io_pending = 1;
    sc->sc_write_reg_ctrl2((aux << SPI_AUX_SHIFT) | len);
    return 0;
}


int
spi_async_transceive_done(unsigned int unit)
{
    F_PREAMBLE;

    if (sc->sc_io_pending == 1)
	return -1;

    if ((sc->sc_last_aux & 0x4) == 0)
        flush_dcache_range(sc->sc_last_buf, sc->sc_last_buf + sc->sc_last_len);

    return 0;
}


int
spi_transceive(
            unsigned int unit,
            char *buf, unsigned int len,
            unsigned char aux)
{
    if (spi_async_transceive(unit, buf, len, aux) != 0)
       return -1;

    while (spi_async_transceive_done(unit) != 0)
	;

    return 0;
}

