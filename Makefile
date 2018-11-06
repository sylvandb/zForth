

all:
	make -C src/linux
	cp -u src/linux/zforth ./

clean:
	make -C src/linux clean
	make -C src/atmega8 clean

realclean: clean
	rm -f ./zforth
