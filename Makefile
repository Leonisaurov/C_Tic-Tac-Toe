BINARY ?= "tictactoe"
DEV ?= FALSE
BIN_DIR ?= /usr/local/bin
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
FLAGS ?= -lcurl -ljansson -L.

ifeq ($(DEV),TRUE)
	COMPILE_CMD = ${CC} -fsanitize=address ${FLAGS}
else
	COMPILE_CMD = ${CC}
endif

%.o: %.c
	$(COMPILE_CMD) -c -o $@ $<

${BINARY}: ${OBJS}
	$(COMPILE_CMD) ${OBJS} -o $@ ${FLAGS}

run: ${BINARY}
	./${BINARY}

install: ${BINARY}
	cp -i ${BINARY} ${BIN_DIR}

uninstall:
	rm -i ${BIN_DIR}/${BINARY}

clean:
	rm -rfv ${BINARY} ${OBJS}
