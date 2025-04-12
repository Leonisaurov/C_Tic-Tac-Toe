BINARY ?= "tictactoe"
DEV ?= FALSE
BIN_DIR ?= /usr/local/bin

ifeq ($(DEV),TRUE)
	COMPILE_CMD = ${CC} *.c -o ${BINARY} -fsanitize=address
else
	COMPILE_CMD = ${CC} *.c -o ${BINARY}
endif

compile: *.c *.h
	$(COMPILE_CMD) -lcurl -ljansson

run: compile
	./${BINARY}

install: compile
	cp -i ${BINARY} ${BIN_DIR}

uninstall:
	rm -i ${BIN_DIR}/${BINARY}

clean:
	rm -rfv ${BINARY}
