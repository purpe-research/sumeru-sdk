TARGETS  = libgatt.a
OBJECTS  = att.o crypto.o uuid.o gatt.o gattrib.o btio.o log.o utils.o gatt.o
CFLAGS  += -static -Wall -Ibluez $$(pkg-config --cflags glib-2.0)

CFLAGS  += -DVERSION='"5.25"'
CXXFLAGS = $(CFLAGS)

vpath %.c bluez/attrib
vpath %.c bluez/src
vpath %.c bluez/src/shared
vpath %.c bluez/lib
vpath %.c bluez/btio

all: $(TARGETS)

libgatt.a: $(OBJECTS)
	$(RM) $@
	$(AR) r $@ $^
	ranlib $@

.PHONY: clean
clean:
	rm -f *.o *.a *~
