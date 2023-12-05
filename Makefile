include config.mk


SRC = src
OBJ = obj
BIN = bin
DOC = doc
DEB = debug
TESTS_NAME = tests
SHARED_DIR = shared

SRC_CTL = ${SRC}/potatoctl.c ${SRC}/utils.c ${SRC}/client.c ${SRC}/socket.c ${SRC}/timer.c ${SRC}/pidfile.c
SRC_D = ${SRC}/timer.c ${SRC}/potatod.c ${SRC}/utils.c ${SRC}/socket.c ${SRC}/pidfile.c
SRC_TUI = ${SRC}/potatotui.c ${SRC}/timer.c ${SRC}/utils.c ${SRC}/socket.c ${SRC}/client.c ${SRC}/todo.c ${SRC}/ncurses-utils.c ${SRC}/pidfile.c

OBJ_CTL = $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRC_CTL))
OBJ_D = $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRC_D))
OBJ_TUI = $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRC_TUI))

OBJ_CTL_DEB = $(patsubst $(SRC)/%.c,$(OBJ)/%.odeb,$(SRC_CTL))
OBJ_D_DEB = $(patsubst $(SRC)/%.c,$(OBJ)/%.odeb,$(SRC_D))
OBJ_TUI_DEB = $(patsubst $(SRC)/%.c,$(OBJ)/%.odeb,$(SRC_TUI))

SRC_TESTS = ${SRC}/tests.c ${SRC}/timer.c ${SRC}/utils.c
OBJ_TESTS = $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRC_TESTS))

D_NAME = potd
CTL_NAME = potctl
TUI_NAME = potui

BINS_PATHS = ${BIN}/${D_NAME} ${BIN}/${CTL_NAME} ${BIN}/${TUI_NAME} 

MD_DOCS = $(wildcard ${DOC}/*.md)
MAN_PAGES = $(patsubst %.md,%.1,$(MD_DOCS))

$(OBJ)/%.o: $(SRC)/%.c
	mkdir -p $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ)/%.odeb: $(SRC)/%.c
	mkdir -p $(OBJ)
	$(CC) $(CFLAGS) ${DEBFLAGS} -c $< -o $@

all: options ${BINS_PATHS}

doc: ${MD_DOCS}
	for man in ${MD_DOCS}; do \
		pandoc $$man -s -t man -o $${man/.md}.1; \
	done


test: ${DEB}/${TESTS_NAME}
	@echo Running tests:
	@./${DEB}/${TESTS_NAME} && echo Tests are looking good!

install: all install_options ${MAN_PAGES}
	mkdir -p ${DESTDIR}${PREFIX}/bin
	for bin in ${BINS_PATHS}; do \
		cp -f $$bin ${DESTDIR}${PREFIX}/bin; \
	done

	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	for man_page in ${MAN_PAGES}; do \
		sed "s/VERSION/${VERSION}/g" < $$man_page > ${DESTDIR}${MANPREFIX}/man1/$$(basename $$man_page); \
	done


${DEB}/${TESTS_NAME}: ${OBJ_TESTS}
	mkdir -p ${DEB}
	$(CC) -o ${DEB}/${TESTS_NAME} ${OBJ_TESTS}

${OBJ_CTL} ${OBJ_D} ${OBJ_TUI} ${OBJ_TESTS} ${OBJ_DEBUG}: include/signal.h config.h config.mk

install_options:
	@echo potato install options:
	@echo "DESTDIR  = ${DESTDIR}"
	@echo "PREFIX   = ${PREFIX}"
	@echo "MANPREFIX   = ${MANPREFIX}"

options:
	@echo potato build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

${BIN}/${D_NAME}: ${OBJ_D}
	mkdir -p $(BIN)
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ ${OBJ_D}

${BIN}/${CTL_NAME}: ${OBJ_CTL}
	mkdir -p $(BIN)
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ ${OBJ_CTL}

${BIN}/${TUI_NAME}: ${OBJ_TUI}
	mkdir -p $(BIN)
	${CC} ${CFLAGS} ${LDFLAGS} `pkg-config --libs --cflags ncurses` -o $@ ${OBJ_TUI}


debug: ${DEB}/${D_NAME} ${DEB}/${CTL_NAME} ${DEB}/${TUI_NAME}

${DEB}/${D_NAME}: ${OBJ_D_DEB}
	mkdir -p $(DEB)
	$(CC) ${CFLAGS} ${LDFLAGS} ${DEBFLAGS} -o $@ ${OBJ_D_DEB}

${DEB}/${CTL_NAME}: ${OBJ_CTL_DEB}
	mkdir -p $(DEB)
	$(CC) ${CFLAGS} ${LDFLAGS} ${DEBFLAGS} -o $@ ${OBJ_CTL_DEB}

${DEB}/${TUI_NAME}: ${OBJ_TUI_DEB}
	mkdir -p $(DEB)
	$(CC) ${CFLAGS} ${LDFLAGS} ${DEBFLAGS} `pkg-config --libs --cflags ncurses` -o $@ ${OBJ_TUI_DEB}

config.h: 
	cp config.def.h $@

clean:
	rm $(OBJ) $(BIN) $(DEB) -rf

uninstall:
	for bin in ${BINS_PATHS}; do \
		rm ${DESTDIR}${PREFIX}/bin/$$(basename $$bin); \
	done

	for man_page in ${MAN_PAGES}; do \
		rm ${DESTDIR}${MANPREFIX}/man1/$$(basename $$man_page); \
	done
