#include <sumeru/constant.h>
#include <sumeru/cpu/csr.h>

#include "device.h"
#include "os.h"


static const char *s_argv[] = {
    OS_VERSION,
    0
};

static const char *s_env[] = {
    "LINES=24",
    "COLUMNS=80",
    "TERM=xterm",
    "TERMCAP=xterm:\
        :am:bs:km:mi:ms:xn:\
        :co#80:it#8:li#24:\
        :Co#8:AL=\E[%dL:DC=\E[%dP:DL=\E[%dM:DO=\E[%dB:IC=\E[%d@:\
        :K1=\EOw:K2=\EOy:K3=\EOu:K4=\EOq:K5=\EOs:LE=\E[%dD:\
        :RI=\E[%dC:UP=\E[%dA:ae=^O:al=\E[L:as=^N:bl=^G:bt=\E[Z:\
        :cd=\E[J:ce=\E[K:cl=\E[H\E[2J:cm=\E[%i%d;%dH:cr=^M:\
        :cs=\E[%i%d;%dr:ct=\E[3g:dc=\E[P:dl=\E[M:do=^J:ec=\E[%dX:\
        :ei=\E[4l:ho=\E[H:ic=\E[@:im=\E[4h:\
        :is=\E7\E[r\E[m\E[?7h\E[?1;3;4;6l\E[4l\E8\E>:\
        :k1=\E[11~:k2=\E[12~:k3=\E[13~:k4=\E[14~:k5=\E[15~:\
        :k6=\E[17~:k7=\E[18~:k8=\E[19~:k9=\E[20~:kD=\177:kI=\E[2~:\
        :kN=\E[6~:kP=\E[5~:kb=^H:kd=\EOB:ke=\E[?1l\E>:kh=\EOH:\
        :kl=\EOD:kr=\EOC:ks=\E[?1h\E=:ku=\EOA:le=^H:md=\E[1m:\
        :me=\E[m\017:mr=\E[7m:nd=\E[C:rc=\E8:sc=\E7:se=\E[27m:\
        :sf=^J:so=\E[7m:sr=\EM:st=\EH:ta=^I:te=\E[2J\E[?47l\E8:\
        :ti=\E7\E[?47h:ue=\E[24m:up=\E[A:us=\E[4m:\
        :vb=\E[?5h\E[?5l:ve=\E[?25h:vi=\E[?25l:vs=\E[?25h:",
    0
};

extern const char **environ;
extern int main(int argc, const char **argv);

static unsigned int s_PERIPH_CFG = 0;
static unsigned int s_DEVUNIT_UART = 0;
static unsigned int s_DEVUNIT_I2C = 0;
static unsigned int s_DEVUNIT_SPI = 0;

static intr_dispatch_entry_t s_INTR_DISPATCH[CPU_IVEC_MAX];

static void
h_unrecognized_intr(void *act)
{
    /* XXX : Do something better here -- logging? */
    return;
}

static intr_dispatch_entry_t unrecognized_intr = { h_unrecognized_intr, 0 };

static void
setup_devices(unsigned int periph)
{
    unsigned int ctr, vnr;

    for (int i = 0; i < CPU_IVEC_MAX; ++i) s_INTR_DISPATCH[i] = unrecognized_intr;

    s_PERIPH_CFG = periph;

    vnr = 1; /* skip over boot intr (0) */

    /* 
     * boot, timer, uart0 and spi0 have fixed interrupts and 
     * are always present / unchanged.
     */
    dev_alloc_timer(0, s_INTR_DISPATCH + vnr++);
    dev_alloc_uart(						/* uart 0 */
		s_DEVUNIT_UART++,
	    	s_INTR_DISPATCH + vnr,
	    	s_INTR_DISPATCH + (vnr + 1));
    vnr += 2;
    dev_alloc_spi(						/* spi0 0 */
		s_DEVUNIT_SPI++,
	    	s_INTR_DISPATCH + vnr);
    ++vnr;

    /* invariant vnr == 5, unit_spi = 1, unit_uart = 1, unit_i2c = 0 */

    for (ctr = 0, vnr = 5; 
	 (ctr < CPU_PERIPH_MAX) && (vnr < (CPU_IVEC_MAX - 1)); 
	 ++ctr) 
    {
	unsigned int x = (periph >> (ctr * 3)) & 0x7;
	switch (x) {
	    case PERIPH_NONE:
		/* note: no increment to vnr++ */
		continue;
	    case PERIPH_SPI:
		dev_alloc_spi(
				s_DEVUNIT_SPI++,
	    			s_INTR_DISPATCH + vnr++);
		break;
	    case PERIPH_I2C:
		dev_alloc_i2c(
				s_DEVUNIT_I2C++,
	    			s_INTR_DISPATCH + vnr++);
		break;
	    case PERIPH_UART:
		dev_alloc_uart(
				s_DEVUNIT_UART++,
	    			s_INTR_DISPATCH + vnr,
	    			s_INTR_DISPATCH + (vnr + 1));
		vnr += 2;
		break;
	    default:
		/* unrecognized interrupts goto default handler */
		break;
	}
    }
    dev_alloc_gpio(0, s_INTR_DISPATCH + (CPU_IVEC_MAX - 1));
    return;
}


static void
start_subsystems()
{
    pwm_subsystem_start();
    timer_subsystem_start();
    uart_subsystem_start();
}



static void
stop_subsystems()
{
    uart_subsystem_stop();
    timer_subsystem_stop();
    pwm_subsystem_stop();
}


void
os_main(unsigned int crt0_version, unsigned int periph_cfg)
{
    setup_devices(periph_cfg);
    enable_all_intr();
    start_subsystems();
    environ = s_env;

    main(sizeof(s_argv) / sizeof(char*), s_argv);

    stop_subsystems();

    /* 
     * crt0 will call exit upon return from os_main.
     * exit will re-execute the cpu bootcode, resulting
     * in the application being rexecuted if it was
     * stored on flash.
     */
}


thread_t*
os_handle_interrupt(unsigned int id)
{
    intr_dispatch_entry_t *e = s_INTR_DISPATCH + id; 
    (*e->handler)(e->act);
    return 0;
}
