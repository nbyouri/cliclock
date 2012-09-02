SRC = clock.c
CC = cc
BIN = cli-clock
INSTALLPATH = /usr/local/bin/
CFLAGS = -Wall -std=c99
LDFLAGS = -lcurses


cli-clock : ${SRC}

	@echo "build ${SRC}"
	@echo "CC ${CFLAGS} ${LDFLAGS} ${SRC}"
	@${CC} ${CFLAGS} ${LDFLAGS} ${SRC} -o ${BIN}

install : ${BIN}

	@echo "installing binary file to ${INSTALLPATH}${BIN}"
	@cp ${BIN} ${INSTALLPATH}
	@chmod 755 ${INSTALLPATH}${BIN}
	@echo "installed"

uninstall :

	@echo "uninstalling binary file (${INSTALLPATH}${BIN})"
	@rm -f ${INSTALLPATH}${BIN}
	@echo "${BIN} uninstalled"
clean :

	@echo "cleaning ${BIN}"
	@rm ${BIN}
	@echo "${BIN} cleaned"

