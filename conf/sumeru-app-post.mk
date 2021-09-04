all: $(PROG)

TEMP += $(CFILES:.c=.o)
TEMP += $(CXXFILES:.cpp=.o)
TEMP += $(ASMFILES:.S=.o)
OBJFILES = $(sort $(TEMP))

ifdef NO_STDINC
CFLAGS += -nostdinc
endif

ifdef NO_STDLIB
LDFLAGS += -nostdlib
endif

$(PROG): $(OBJFILES)
	$(GCC) $(LDFLAGS) -o $@ $^ $(LDADD)
	${OBJCOPY} -O binary $@ $(@).bin
	# Add a 16 bytes padding to file to help with flush in 
	# bootloader firmware
	dd if=/dev/zero of=$(@).bin oflag=append bs=1 conv=notrunc count=16
	# Write magic to binary, required for auto-loading via BOOTLOADER
	${SUMERU_DIR}/bin/sumeru-binary-header $(@).bin

clean:
	rm -f $(OBJFILES) $(PROG).bin $(PROG) $(CLEANFILES) \
	    compile.log install.log

disa:
	${OBJDUMP} -d ${PROG}

run-app: all
ifdef SUMERU_UART_DEVICE
	    ${SUMERU_DIR}/bin/sumeru -D $(SUMERU_UART_DEVICE) --run-app ${PWD}/${PROG}.bin 
else
	    @echo "Please set SUMERU_UART_DEVICE in your Makefile, e.g.,"
	    @echo "    SUMERU_UART_DEVICE = /dev/ttyUSB0"
endif

flash-app: all
ifdef SUMERU_UART_DEVICE
	    ${SUMERU_DIR}/bin/sumeru -D $(SUMERU_UART_DEVICE) --flash-app ${PWD}/${PROG}.bin 
else
	    @echo "Please set SUMERU_UART_DEVICE in your Makefile, e.g.,"
	    @echo "    SUMERU_UART_DEVICE = /dev/ttyUSB0"
endif


# install does nothing
#
install:

