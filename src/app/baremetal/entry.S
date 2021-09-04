#include <sumeru/cpu/constant.h>
#include <sumeru/constant.h>

#define INTR_ID_UNUSED	0xff

#define IVEC(id) \
	sw a0,0(zero); \
	li a0,id; \
	j asm_handle_interrupt; \
	nop 

.section .text.ivector
.globl _start, main, timer_intr_handler
_start:

/* Vector Table for default CPU peripheral configuration */

ivec:
ivec_boot:
    j asm_entry
    nop 
    nop
    nop
ivec_timer0:
    IVEC(INTR_ID_1);
ivec_uart0_tx:
    IVEC(INTR_ID_2);
ivec_uart0_rx:
    IVEC(INTR_ID_3);
ivec_spi0:
    IVEC(INTR_ID_UNUSED);
ivec_spi1:
    IVEC(INTR_ID_UNUSED);
ivec_uart1_tx:
    IVEC(INTR_ID_UNUSED);
ivec_uart1_rx:
    IVEC(INTR_ID_UNUSED);
ivec_spi2:
    IVEC(INTR_ID_UNUSED);
ivec_spi3:
    IVEC(INTR_ID_UNUSED);
ivec_spi4:
    IVEC(INTR_ID_UNUSED);
ivec_i2c0:
    IVEC(INTR_ID_UNUSED);
ivec_i2c1:
    IVEC(INTR_ID_UNUSED);
ivec_uart2_tx:
    IVEC(INTR_ID_UNUSED);
ivec_uart2_rx:
    IVEC(INTR_ID_UNUSED);
ivec_gpio:
    IVEC(INTR_ID_UNUSED);


## BOOT ENTRY ROUTINE
##
##

asm_entry:

    # Initialize ivector address
    la sp,ivec
    csrrw zero,CSR_REG_IVECTOR_ADDR,sp

    # Setup the stack at 0x1000
    # stack pointer points to next free for use 
    lui sp,0x1

    # Initialize global pointer
    .option push
    .option norelax
1:  auipc gp, %pcrel_hi(__global_pointer$)
    addi  gp, gp, %pcrel_lo(1b)
    .option pop

    call    main

asm_entry_done:
    j asm_entry_done

#
# Interrupt handling strategy of this program
# is to set the a flag on interrupt, 
# The user (C code) waits on a flag after
# a uart transmit and receive for the
# opreration to complete.
# 
# In the normal C runtime mode this is all 
# abstracted away by the device libraries 
# and OS code.
#
#
asm_handle_interrupt:
    sw a1,4(zero)
    li a1,INTR_ID_1
    beq a0,a1,1f 
    li a1,INTR_ID_2
    beq a0,a1,2f 
    li a1,INTR_ID_3
    beq a0,a1,3f 
    j intr_return


# 
# 0x3e80000 = 65536000 = ~900 ms 
# for timing calculations refer to
# <sumeru/constant.h> and
# <sumeru/cpu/constant.h>

1:
    li a0,(0x3e80000 | 0xf)
    csrrw zero,CSR_REG_TIMER0_CTRL,a0
    csrrsi a0,CSR_REG_GPIO_INPUT,0	
    andi a0,a0,1
    beq zero,a0,setbit
    csrrci zero,CSR_REG_GPIO_OUT,1
    j intr_return
setbit:
    csrrsi zero,CSR_REG_GPIO_OUT,1
    j intr_return

2:
    la a0,g_uart0_tx_intr_pending
    sw zero,0(a0)
    j intr_return
3:
    la a0,g_uart0_rx_intr_pending
    sw zero,0(a0)

intr_return:
    lw a1,4(zero)
    csrrsi a0,CSR_REG_CTX_PCSAVE,0
    csrrw  zero,CSR_REG_CTX_PCSWITCH,a0
    lw a0,0(zero)
    csrrwi zero,CSR_REG_SWITCH,0

#******* not reached ***********

