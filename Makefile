

all:
	make -C src/linux
	cp -u src/linux/zforth ./

ONEFILE_FILES := src/linux/zfconf.h src/zforth/zforth.h src/zforth/zforth.c src/linux/main.c
ONEFILE_WORDS := forth/core.zf forth/dict.zf

onefile.c: ${ONEFILE_FILES} ${ONEFILE_WORDS}
	@rm -f $@
	@echo '#!/usr/bin/env -S tcc  -run  -std=c99  -DUSE_READLINE  -lreadline -lm' >> $@
	@echo '#define ONEFILE' >> $@
	@cat ${ONEFILE_FILES} >> $@
	@sed -i 's/^\s*\(#\s*include\s\+["<]zfconf\.h[">].*\)/\/* no \1 *\//g' $@
	@sed -i 's/^\s*\(#\s*include\s\+["<]zforth\.h[">].*\)/\/* no \1 *\//g' $@
	@#cat ${ONEFILE_FILES} | sed ' s/^\s*\(#\s*include\s\+["<]zfconf\.h[">].*\)/\/* no \1 *\//g; s/^\s*\(#\s*include\s\+["<]zforth\.h[">].*\)/\/* no \1 *\//g; ' >> $@
	@echo 'char *hcwords = "' >> $@
	@echo '( ----- default init dict ----- )' >> $@
	@cat ${ONEFILE_WORDS} /dev/null | sed 's/"/\\"/g' >> $@
	@echo '( ----- default init dict ----- )' >> $@
	@echo '"; /* hcwords */' >> $@
	chmod +x $@

onefile: onefile.c
	tcc  $< -o $@  -std=c99  -DUSE_READLINE  -lreadline -lm

clean:
	rm -f onefile
	make -C src/linux clean
	make -C src/atmega8 clean

realclean: clean
	rm -f ./zforth onefile.c
