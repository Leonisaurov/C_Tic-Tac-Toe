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

## AI Game Mode - Pollinations BYOP (Bring Your Own Pollen)

This game uses **Pollinations.ai** for AI moves via the **Pollinations Pay / BYOP (Bring Your Own Pollen)** system. This allows users to pay for their own AI usage through a secure OAuth-like Device Flow.

### How it works

1. **Developer registers an App** at [enter.pollinations.ai/keys](https://enter.pollinations.ai/keys) → **App** → **Create**
   - Get a **Publishable App Key** (`pk_...`)
   - This key identifies your application

2. **User authorizes the app** via Device Flow (no browser redirect needed for CLI):
   - Enter the App Key (`pk_...`) in the game menu: **"Pollinations App Key (pk_): "**
   - Select **"Authorize with Pollinations"**
   - The game displays a **User Code** (e.g., `ABCD-1234`) and a **URL** (e.g., `https://enter.pollinations.ai/device?user_code=ABCD-1234`)
   - Open the URL in any browser, enter the code, and approve the authorization
   - The game polls for completion and receives a **User Secret Key** (`sk_...`)

3. **AI requests use the User Key** (`sk_...`)
   - Each user pays for their own usage from their Pollinations balance
   - The developer's App Key only identifies the app, doesn't pay for usage
   - Keys are saved to `~/.config/tictactoe/config.json` for future sessions

### Setup Instructions

#### 1. Create a Pollinations App Key (Developer)
1. Go to [enter.pollinations.ai/keys](https://enter.pollinations.ai/keys)
2. Click **App** tab → **Create**
3. Enter a name for your app (e.g., "TicTacToe AI")
4. Copy the **App Key** (starts with `pk_`)

#### 2. Configure in Game
1. Run the game: `./tictactoe`
2. Navigate to **"Pollinations App Key (pk_): "**
3. Press Enter and paste your `pk_...` key
4. Navigate to **"Authorize with Pollinations"** and press Enter
5. Follow the on-screen instructions:
   - Open the displayed URL in your browser
   - Enter the shown User Code
   - Click **Allow** to authorize
6. Wait for "¡Autorización exitosa!" - your User Key (`sk_...`) is now saved

#### 3. Play vs AI
- Select **"Start Game VS AI"**
- Press **'r'** during your turn to request an AI move
- The AI will use your authorized `sk_...` key

### Configuration File
Settings are automatically saved to `~/.config/tictactoe/config.json`:
- Player symbols and colors
- Pollinations App Key (`pk_...`)
- Pollinations User Key (`sk_...`)

### Requirements
- **libcurl** with OpenSSL support
- **libjansson** for JSON parsing
- Internet connection for AI moves and authorization

### Troubleshooting
- **"App Key debe comenzar con 'pk_'"**: Ensure you're using the App Key, not a User Key
- **"Tiempo agotado"**: Authorization expires in 30 minutes; try again
- **Network errors**: Check internet connection and firewall
- **Invalid move from AI**: The AI sometimes makes mistakes; just press 'r' again
