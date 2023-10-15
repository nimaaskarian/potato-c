include config.mk

all: potatod potatoctl

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp build/potatod ${DESTDIR}${PREFIX}/bin
	cp build/potatoctl ${DESTDIR}${PREFIX}/bin

potatod: src/config.h src/lib/timer.c src/lib/utils.c src/lib/signal.h src/potatod.c
	$(CC) src/lib/timer.c src/lib/utils.c src/potatod.c -o build/$@

potatoctl: src/config.h src/lib/signal.h src/potatoctl.c
	$(CC) src/potatoctl.c -o build/$@

src/config.h: 
	cp src/config.def.h $@

clean:
	rm src/config.h
	rm build/*

uninstall:
	rm ${DESTDIR}${PREFIX}/bin/potatod
	rm ${DESTDIR}${PREFIX}/bin/potatoctl
