all: $(LIB)

TEMP = $(CFILES:.c=.o)
TEMP += $(CXXFILES:.cpp=.o)
TEMP += $(ASMFILES:.S=.o)
OBJFILES = $(sort $(TEMP))

$(LIB): $(OBJFILES)
	$(RM) -f $@
	$(AR) Dr $@ $^

clean:
	$(RM) $(OBJFILES) $(LIB)

install: $(LIB)
	cp $(LIB) ${SUMERU_DIR}/lib
ifdef INCLUDE_FILES
	    mkdir -p $(SUMERU_DIR)/include/$(INCLUDE_INSTALL_PREFIX)
	    $(foreach fn,$(INCLUDE_FILES),cp $(fn) $(SUMERU_DIR)/include/$(INCLUDE_INSTALL_PREFIX)/ ;)
endif
