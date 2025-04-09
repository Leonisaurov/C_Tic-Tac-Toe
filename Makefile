BINARY ?= "tictactoe"
DEV ?= FALSE

ifeq ($(DEV),TRUE)
	COMPILE_CMD = ${CC} *.c -o ${BINARY} -fsanitize=address
else
	COMPILE_CMD = ${CC} *.c -o ${BINARY}
endif

compile: *.c *.h
	$(COMPILE_CMD)

run: compile
	./${BINARY}

clean:
	rm -rfv ${BINARY}
