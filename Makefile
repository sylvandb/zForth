

all:
	make -C src/linux
	cp -u src/linux/zforth ./

ONEFILE_FILES := src/linux/zfconf.h src/zforth/zforth.h src/zforth/zforth.c src/linux/main.c
ONEFILE_WORDS := forth/core.zf forth/dict.zf

forth/local.zf:
	touch $@

onefile.c: ${ONEFILE_FILES} ${ONEFILE_WORDS} forth/local.zf
	@rm -f $@
	@echo '#!/usr/bin/env -S tcc  -run  -lm -lreadline' >> $@
	@echo '#define USE_READLINE' >> $@
	@echo '#define ONEFILE' >> $@
	@echo '/* tcc -dM -E - < /dev/null */' >> $@
	@cat ${ONEFILE_FILES} >> $@
	#@sed -i 's/^\s*\(#\s*include\s\+["<]zfconf\.h[">].*\)/\/* no \1 *\//g' $@
	#@sed -i 's/^\s*\(#\s*include\s\+["<]zforth\.h[">].*\)/\/* no \1 *\//g' $@
	@#cat ${ONEFILE_FILES} | sed ' s/^\s*\(#\s*include\s\+["<]zfconf\.h[">].*\)/\/* no \1 *\//g; s/^\s*\(#\s*include\s\+["<]zforth\.h[">].*\)/\/* no \1 *\//g; ' >> $@
	@echo 'char *hcwords = "' >> $@
	@echo '( ----- default init dict ----- )' >> $@
	@cat ${ONEFILE_WORDS} forth/local.zf /dev/null | sed 's/"/\\"/g' >> $@
	@echo '( ----- default init dict ----- )' >> $@
	@echo '"; /* hcwords */' >> $@
	chmod +x $@

onefile: onefile.c
	tcc  -D_GNU_SOURCE $< -o $@  -lm -lreadline

clean:
	rm -f onefile
	make -C src/linux clean
	make -C src/atmega8 clean

realclean: clean
	rm -f ./onefile.c
	rm -f ./zforth
