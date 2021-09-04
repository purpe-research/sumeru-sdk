## XXX Doing a clean before all is paramount as the two makefiles use
##     same .o filenames

all:
	make -f Makefile.sdfat clean all
	make -f Makefile.unistd clean all

clean:
	make -f Makefile.sdfat clean
	make -f Makefile.unistd clean

install: all
	cp libsumeru_unistd.a libsumeru_unistd_sdfat.a ${SUMERU_DIR}/lib
	cp sumeru_unistd.h ${SUMERU_DIR}/include/sumeru
    
