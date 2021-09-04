#include <sumeru/constant.h>
#include <sumeru/cpu/csr.h>

#include "device.h"
#include "os.h"

void
os_gettimeofday(os_timeval_t *tv)
{
    uint64_t t, z;
    uint32_t x;

    do {
    	x = rdtime_h();
	t = x;
    	t <<= 32;
    	t |= rdtime();
    } while (rdtime_h() != x);

    z = t / CPU_CLK_FREQ;
    tv->tv_sec = z;
    t = t - (z * CPU_CLK_FREQ);
    tv->tv_usec = t / CPU_CLK_TICKS_PER_US;
}


void
os_waitms(unsigned int ms)
{
    struct os_timeval tv;
    uint64_t a, b;

    os_gettimeofday(&tv);
    a = (tv.tv_sec * 1000000) + tv.tv_usec;
    a /= 1000;
    a += ms;
    while (1) {
        os_gettimeofday(&tv);
        b = (tv.tv_sec * 1000000) + tv.tv_usec;
        b /= 1000;
        if (b >= a)
            break;
    }
}
