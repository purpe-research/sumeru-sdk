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

clean:
	rm -f $(OBJFILES) $(PROG).bin $(PROG) $(CLEANFILES) \
	    compile.log install.log

install: $(PROG)
	cp $(PROG) ${SUMERU_DIR}/bin
