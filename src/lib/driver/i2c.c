#include <errno.h>

#include <sumeru/cpu/csr.h>
#include <sumeru/cpu/memctl.h>
#include <sumeru/constant.h>

#include "device.h"
#include "i2c.h"

#define MAX_I2C_DEVICES		2

typedef struct i2c_softc 
{
    unsigned int		sc_unit;
    i2c_user_intr_callback_t	sc_cb;
    volatile unsigned int	sc_io_pending;

    register_write_fn_t		sc_write_reg_ctrl1;
    register_write_fn_t		sc_write_reg_ctrl2;
    register_read_fn_t		sc_read_reg_ctrls;

    char			*sc_last_buf;
    unsigned int		sc_last_len;
    unsigned char		sc_clk_sel;
    unsigned char		sc_last_addr;

} i2c_softc_t;

#define DEFINE_UART_CSR_FN(unit) \
    static void unit ## _write_reg_ctrl1(unsigned int v) \
    { \
	csr_ ## unit ## _ctrl_1_write(v); \
    } \
    \
    static void unit ## _write_reg_ctrl2(unsigned int v) \
    { \
	csr_ ## unit ## _ctrl_2_write(v); \
    } \
    static unsigned int unit ## _read_reg_ctrls(unsigned int v) \
    { \
	return csr_ ## unit ## _ctrl_s_read(); \
    }


DEFINE_UART_CSR_FN(i2c0);
DEFINE_UART_CSR_FN(i2c1);

static i2c_softc_t i2c_softc_array[MAX_I2C_DEVICES] = 
{
    { 
	0, 0, 0,
	i2c0_write_reg_ctrl1, i2c0_write_reg_ctrl2,
	i2c0_read_reg_ctrls,
	0, 0, SPI_CLK_SEL_36M, 0
    },
    { 
	1, 0, 0,
	i2c1_write_reg_ctrl1, i2c1_write_reg_ctrl2,
	i2c1_read_reg_ctrls,
	0, 0, SPI_CLK_SEL_36M, 0
    }
};


static void 
i2c_intr_handler(i2c_softc_t *sc)
{
    sc->sc_io_pending = 0;
    if (sc->sc_cb)
	(*sc->sc_cb)(sc->sc_unit);
}


#define F_PREAMBLE \
    i2c_softc_t *sc; \
    if (unit >= MAX_I2C_DEVICES) { \
	errno = ENOENT; \
	return -1; \
    } \
    sc = i2c_softc_array + unit


int
i2c_set_user_intr_callback(unsigned int unit, i2c_user_intr_callback_t h)
{
    F_PREAMBLE;
    sc->sc_cb = h;
    return 0;
}


int
dev_alloc_i2c(unsigned int unit, intr_dispatch_entry_t *e)
{
    F_PREAMBLE;
    e->handler = (device_intr_handler_t) i2c_intr_handler;
    e->act = sc;

    return 0;
}


int
i2c_async_transceive(
	    unsigned int unit,
            char *buf, unsigned int len, 
            unsigned char addr)
{
    F_PREAMBLE;
    len &= 0xffff;

    if ((addr & 1) == 0) {
        flush_dcache_range(buf, buf + len);
    }

    sc->sc_write_reg_ctrl1(((unsigned int)buf) | (sc->sc_clk_sel << I2C_CLKSEL_SHIFT));
    sc->sc_io_pending = 1;
    sc->sc_last_buf = buf;
    sc->sc_last_len = len;
    sc->sc_last_addr = addr;
    sc->sc_write_reg_ctrl2((addr << 16)| len);
    return 0;
}

    
int
i2c_async_transceive_done(unsigned int unit)
{
    F_PREAMBLE;

    int e = sc->sc_read_reg_ctrls();

    if (e == 0 && (sc->sc_last_addr & 1))
        flush_dcache_range(sc->sc_last_buf, sc->sc_last_buf + sc->sc_last_len);

    return e;
}


int
i2c_transceive(
	    unsigned int unit,
            char *buf, unsigned int len, 
            unsigned char addr)
{
   if (i2c_async_transceive(unit, buf, len, addr) != 0)
       return -1;

    while (i2c_async_transceive_done(unit) != 0)
	;

    return 0;
}
