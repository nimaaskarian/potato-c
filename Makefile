include config.mk

all: potatod potatoctl

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp build/potatod ${DESTDIR}${PREFIX}/bin
	cp build/potatoctl ${DESTDIR}${PREFIX}/bin

potatod: src/lib/*.c src/lib/signal.h src/potatod.c
	$(CC) src/lib/*.c src/potatod.c -o build/potatod

potatoctl: src/lib/signal.h src/potatoctl.c
	$(CC) src/potatoctl.c -o build/potatoctl


# all: potato
