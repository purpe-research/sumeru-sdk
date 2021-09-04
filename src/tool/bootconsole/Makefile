PROG = sumeru

all: build

build: ../lib/bluetooth/libgatt.a
	gcc -Wl,--gc-sections -o $(PROG) -g -static \
		-I /usr/include/glib-2.0/gio \
		-I /usr/include/glib-2.0 \
		-I /usr/lib/x86_64-linux-gnu/glib-2.0/include \
		-I /usr/include/bluetooth \
		-I ../lib/bluetooth/bluez \
		-I ../lib/bluetooth \
		bootconsole.c \
		$< \
		/usr/lib/x86_64-linux-gnu/libbluetooth.a \
		-lglib-2.0 -lpthread
	strip $(PROG)

../lib/bluetooth/libgatt.a:
	(cd ../lib/bluetooth; make)

clean:
	rm -f $(PROG)

install:
	cp $(PROG) ${SUMERU_DIR}/bin

