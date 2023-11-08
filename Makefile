include config.mk


SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
DEB_DIR = debug
TESTS_NAME = tests
SHARED_DIR = shared

SRC_CTL = src/potatoctl.c src/utils.c src/client.c src/socket.c src/timer.c
OBJ_CTL = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_CTL))

SRC_D = src/timer.c src/potatod.c src/utils.c src/socket.c
OBJ_D = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_D))

SRC_TUI = src/potatotui.c src/timer.c src/utils.c src/socket.c src/client.c
OBJ_TUI = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_TUI))

SRC_TESTS = src/tests.c src/timer.c src/utils.c
OBJ_TESTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_TESTS))

D_NAME = potd
CTL_NAME = potctl
TUI_NAME = potui

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

all: options ${BIN_DIR}/${D_NAME} ${BIN_DIR}/${CTL_NAME} ${BIN_DIR}/${TUI_NAME} #${DEB_DIR}/${TESTS_NAME} test

test:
	@echo Running tests:
	@./${DEB_DIR}/${TESTS_NAME} && echo Tests are looking good!

install: all install_options
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp ${BIN_DIR}/${D_NAME} ${DESTDIR}${PREFIX}/bin
	cp ${BIN_DIR}/${CTL_NAME} ${DESTDIR}${PREFIX}/bin
	cp ${BIN_DIR}/${TUI_NAME} ${DESTDIR}${PREFIX}/bin

${DEB_DIR}/${TESTS_NAME}: ${OBJ_TESTS}
	mkdir -p ${DEB_DIR}
	$(CC) -o ${DEB_DIR}/${TESTS_NAME} ${OBJ_TESTS}

${OBJ_CTL} ${OBJ_D} ${OBJ_TUI} ${OBJ_TESTS}: include/signal.h config.h config.mk

install_options:
	@echo potato install options:
	@echo "DESTDIR  = ${DESTDIR}"
	@echo "PREFIX   = ${PREFIX}"

options:
	@echo potato build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

${BIN_DIR}/${D_NAME}: ${OBJ_D}
	mkdir -p $(BIN_DIR)
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ ${OBJ_D}

${BIN_DIR}/${CTL_NAME}: ${OBJ_CTL}
	mkdir -p $(BIN_DIR)
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ ${OBJ_CTL}

${BIN_DIR}/${TUI_NAME}: ${OBJ_TUI}
	mkdir -p $(BIN_DIR)
	${CC} ${CFLAGS} ${LDFLAGS} `pkg-config --libs --cflags ncurses` -o $@ ${OBJ_TUI}


debug: ${DEB_DIR}/${D_NAME} ${DEB_DIR}/${CTL_NAME} ${DEB_DIR}/${TUI_NAME}

${DEB_DIR}/${D_NAME}: ${OBJ_D}
	mkdir -p $(DEB_DIR)
	${CC} ${CFLAGS} ${LDFLAGS} ${DEBFLAGS} -o $@ ${OBJ_D}

${DEB_DIR}/${CTL_NAME}: ${OBJ_CTL}
	mkdir -p $(DEB_DIR)
	${CC} ${CFLAGS} ${LDFLAGS} ${DEBFLAGS} -o $@ ${OBJ_CTL}

${DEB_DIR}/${TUI_NAME}: ${OBJ_TUI}
	mkdir -p $(DEB_DIR)
	${CC} ${CFLAGS} ${LDFLAGS} ${DEBFLAGS} `pkg-config --libs --cflags ncurses` -o $@ ${OBJ_TUI}

config.h: 
	cp config.def.h $@

clean:
	rm $(OBJ_DIR) $(BIN_DIR) $(DEB_DIR) -rf

uninstall:
	rm ${DESTDIR}${PREFIX}/bin/${D_NAME}
	rm ${DESTDIR}${PREFIX}/bin/${CTL_NAME}
	rm ${DESTDIR}${PREFIX}/bin/${TUI_NAME}
