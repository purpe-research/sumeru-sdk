GNU GPL Version 3				SDK Version 1.0


	        The SUMERU Software Development Kit
		___________________________________



	Sumeru is a RISC-V "rv32im" single-board-computer (SBC). 
This directory contains a SDK for Sumeru. This SDK is based on the 
stock RISC-V toolchain and libc (newlib) provided by the RISC-V 
foundation.

    www.riscv.org
    https://github.com/riscv/riscv-gcc
    https://purpe.com/sumeru


Instructions for compiling the toolchain locally are available at

    https://purpe.com/sumeru/sdk/doc/toolchain.html



QUICKSTART

a) source the env.sh script in the root directory of this SDK, e.g.,

	> source ./env.sh


    This will setup all the necessary paths and environment variables
required to use the SDK, the Makefile templates and tools.

    The SDK GCC toolchain will be available with the prefix

	riscv32-unknown-elf-

   For e.g.,
	
	$ riscv32-unknown-elf-gcc helloworld.c


c) Programs can be compiled and run on the SBC using 
   the following procedure

	$ riscv32-unknown-elf-gcc -o helloworld helloworld.c
	$ riscv32-unknown-elf-objcopy -O binary helloworld helloworld.bin
	$ sumeru --run-app helloworld.bin


d) If you desire to write your program to the onboard FLASH memory so
   that it is available permanently and executed automatically on reset,
   do the following

	$ sumeru --flash-app helloworld.bin


e) Documentation is available in the doc subdirectory and at

	https://www.purpe.com/sumeru/sdk/doc/index.html



Demonstration Programs
======================

This SDK includes few demonstration programs in the src/app sub-directory.


src/app/baremetal
-----------------

	A baremetal programming example. This program will periodically 
blink the led and print "Hello World!" on the serial console. The 3 PWM
ports will be set to produce a 50% duty cycle at 140.625 KHz.

	This demo program does not use the standard C library (-lc) nor
the C runtime (crt0.S) it does not have any dependencies except its own two
source files (entry.S and baremetal.c)



src/app/helloworld
------------------

	The helloworld program is functionally equivalent to the baremetal
program.  It is written entirely in C and is more terse.
    




--------------------------------------------------------------------------

    To run these demos change to an app  directory and execute 'make run'

      For e.g.,

	$ cd src/app/helloworld
	$ make run

    To write the program to FLASH memory execute 'make flash-kernel'

      For e.g.,

	$ cd src/kernel/helloworld
	$ sumeru --flash-app helloworld.bin




Makefile Templates
=================


    If you use GNU Make and would like to use this SDK's make templates,
    kindly refer to the Makefiles in the app and lib subdirectories and 
    create your own Makefile based on those.

    For example a Makefile for an application would be something as follows,

    	include ${SUMERU_DIR}/conf/sumeru-pre.mk
	PROG = myapp
	CFILES = myapp.c
	include ${SUMERU_DIR}/conf/sumeru-app-post.mk

    and a library Makefile

    	include ${SUMERU_DIR}/conf/sumeru-pre.mk
	PROG = libmylib.a
	CFILES = mylib.c
	include ${SUMERU_DIR}/conf/sumeru-lib-post.mk






Thank you for using our computer, may the force be with you!


sumeru-sdk@purpe.com

Rohit Jalan
r0h17@purpe.com
+91-98932-53310

