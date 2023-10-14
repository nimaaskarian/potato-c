# `pkg-config --libs --cflags dbus-1` 
all: potatod potatoctl

potatod: src/lib/*.c src/lib/signal.h src/potatod.c
	$(CC) src/lib/*.c src/potatod.c -o build/potatod.o

potatoctl: src/lib/signal.h src/potatoctl.c
	$(CC) src/potatoctl.c -o build/potatoctl.o


# all: potato
