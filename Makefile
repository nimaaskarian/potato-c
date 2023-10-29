include config.mk


SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
DEB_DIR = debug

SRC_CTL = src/potatoctl.c
OBJ_CTL = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_CTL))

SRC_D = src/timer.c src/utils.c src/potatod.c
OBJ_D = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_D))

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

all: options bin/potatod bin/potatoctl

install: all install_options
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp bin/potatod ${DESTDIR}${PREFIX}/bin
	cp bin/potatoctl ${DESTDIR}${PREFIX}/bin


${OBJ_CTL} ${OBJ_D}: include/signal.h config.h config.mk

install_options:
	@echo potato install options:
	@echo "DESTDIR  = ${DESTDIR}"
	@echo "PREFIX   = ${PREFIX}"

options:
	@echo potato build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

${BIN_DIR}/potatod: ${OBJ_D}
	mkdir -p $(BIN_DIR)
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ ${OBJ_D}

${BIN_DIR}/potatoctl: ${OBJ_CTL}
	mkdir -p $(BIN_DIR)
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ ${OBJ_CTL}

debug: ${DEB_DIR}/potatod ${DEB_DIR}/potatoctl

${DEB_DIR}/potatod: ${OBJ_D}
	mkdir -p $(DEB_DIR)
	${CC} ${CFLAGS} ${LDFLAGS} ${DEBFLAGS} -o $@ ${OBJ_D}

${DEB_DIR}/potatoctl: ${OBJ_CTL}
	mkdir -p $(DEB_DIR)
	${CC} ${CFLAGS} ${LDFLAGS} ${DEBFLAGS} -o $@ ${OBJ_CTL}

config.h: 
	cp config.def.h $@

clean:
	rm config.h
	rm bin obj -rf

uninstall:
	rm ${DESTDIR}${PREFIX}/bin/potatod
	rm ${DESTDIR}${PREFIX}/bin/potatoctl
