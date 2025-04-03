# Tic Tac Toe
A simple Tic Tac Toe game, with a basic TUI usage, written in C, using the standard C library and also the termios library.

## Compiling
It just needs a C compiler, I usually use gcc, but you can use whatever you want.
### Construct

```bash
gcc main.c -o tictactoc
```

### Play
Simply run the resulting binary named "tictactoe"

```bash
./tictactoe
```

#### Controls
The controls are intuitive:
- Arrow keys for move the cursor (Up, Down, Left, Right)
- Enter for make the move

The cursor is placed on the blackest cell, it highlights the selected cell.
