SRC = clock.c
CC = cc
BIN = cliclock
INSTALLPATH = /usr/local/bin/
CFLAGS = -Wall -std=c99
LDFLAGS = -lcurses


cliclock : ${SRC}

	@echo "build ${SRC}"
	@echo "CC ${LDFLAGS} -o${BIN} ${SRC}"
	@${CC} ${LDFLAGS} -o${BIN} ${SRC}

install : ${BIN}

	@echo "installing binary file to ${INSTALLPATH}${BIN}"
	@mv ${BIN} ${INSTALLPATH}
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

