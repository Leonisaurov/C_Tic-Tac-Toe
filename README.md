# Tic Tac Toe
A simple tic-tac-toe game, with basic TUI usage, written in C, using the standard C library and also the termios library, plus the libcurl and libjansson libraries to run the AI mode.

## Compiling
Basically, you only need make and a C compiler. 
But depending on your distribution, you should install some dependencies like libcurl and libjansson.

### Installing dependencies 
On archlinux-like systems: 
```bash
pacman -S curl jansson
```

On ubuntu-like systems:
```bash
apt install libcurl4-openssl-dev libjansson-dev
```

On android (via termux):
```bash
apt install libcurl libjansson
```

### Build
Build the project:
```bash
make compile
```
Run the project:
```bash
make run
```
Install on the machine:
```bash
make install
```
It will install the game on '/usr/local/bin' and requires superuser permissions.
If you want to install it somewhere else, for example if you do not have superuser permissions, just run:
```bash
make install BIN_DIR="~/.local/bin"
```
Just make sure that the BIN_DIR is in your PATH variable.

<details>
<summary>On termux:</summary>

Due to the special arrangement of folders in termux, it can be installed in this way on the system:
```bash
make install BIN_DIR="/data/data/com.termux/files/usr/bin"
```
</details>

Uninstall the game:
```bash 
make uninstall
```
### Play
Just run the resulting binary named "tictactoe"
```bash
./tictactoe
```

If you are installing it on your machine, you can just do:
```bash
tictactoe
```

For development, you can also use the make command:
```bash
make run
```

#### Controls
The controls are intuitive:
 - Arrow keys to move the cursor (up, down, left, right)
 - Enter to make the move
 - 'q' key to quit the game
 - ESC key to go from the game to the config menu
 - 'r' to request the AI move in AI game mode 
The cursor highlights the selected cell.

## AI game mode 
To play the AI game mode, you should have a Google GEMINI api key, set on the GEMINI_API_KEY environment variable, the api key should have access to the 'gemini-2.0-flash-thinking-exp-01-21' model.
