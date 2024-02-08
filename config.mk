CC = gcc
PREFIX = /usr/local
VERSION = 0.6
MANPREFIX = ${PREFIX}/share/man
CFLAGS = -std=gnu99 -O3 -finline-functions -flto -funroll-loops -Wall -Wno-deprecated-declarations
LDFLAGS = -lm
DEBFLAGS = -g
