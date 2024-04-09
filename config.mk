CC = gcc
NAME = potato-c
PREFIX = /usr/local
sysconfigdir =  /usr/share
CONFIG_DIR = ${sysconfigdir}/${NAME}
VERSION = 0.6.3
MANPREFIX = ${PREFIX}/share/man
LDFLAGS = -lm
CFLAGS = -std=gnu99 -O3 -finline-functions -flto -funroll-loops -Wall -Wno-deprecated-declarations -DCONFIG_DIR=\"$(CONFIG_DIR)\"
DEBFLAGS = -g
