include ${SUMERU_DIR}/conf/sumeru-pre.mk

all: crt0.o
 
crt0.o: crt0.S interrupt.S
	$(GCC) -c -o crt0.o crt0.S

install: crt0.o
	cp crt0.o ${SUMERU_DIR}/lib
	cp sys-struct-offset.h ${SUMERU_DIR}/include/sumeru

clean:
	rm -f crt0.o
