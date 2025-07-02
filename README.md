# qlaunch

qlaunch is a lightweight and efficient application launcher for the X Window System. It allows users to quickly find and launch executables available in their system's PATH by typing a search query.

## Features

*   **Fast and Lightweight**: Designed for minimal resource usage.
*   **Executable Filtering**: Filters and displays executables from your system's PATH as you type.
*   **Customizable Appearance**: Easily configure fonts, colors, and prompt via `config.h`.
*   **Background Execution**: Uses a double-fork mechanism to ensure launched applications run independently in the background.

## Installation

To build and install qlaunch, follow these steps:

1.  **Clone the repository** (if applicable, assuming it's a git repo):
    ```bash
    git clone https://github.com/Soccera1/qlaunch.git
    cd qlaunch
    ```

2.  **Compile and Install**:
    ```bash
    make
    sudo make install
    ```
    By default, `qlaunch` will be installed to `/usr/local/bin/qlaunch`.

## Configuration

qlaunch's appearance and some internal limits can be customized by editing the `config.h` file before compilation.

Key configurable options include:

*   `font_name`: The font to use (e.g., `"9x15"`).
*   `color_fg`, `color_bg`, `color_hl_fg`, `color_hl_bg`: Foreground, background, highlighted foreground, and highlighted background colors (hex codes).
*   `prompt`: The text displayed before the input box (e.g., `">"`).
*   `WINDOW_HEIGHT`: Height of the launcher bar in pixels.
*   `HORZ_PADDING`: Horizontal padding.
*   `CURSOR_WIDTH`: Width of the text input cursor.
*   `MAX_INPUT_LEN`: Maximum length of the user input string.
*   `MAX_ITEMS`: Maximum number of executables to read from PATH.

After modifying `config.h`, you need to recompile and reinstall qlaunch:

```bash
make clean
make
sudo make install
```

## Usage

Once installed, you can run qlaunch by executing:

```bash
qlaunch
```

Typically, you would bind `qlaunch` to a keyboard shortcut in your window manager (e.g., `Alt+Space` or `Super+R`) for quick access.

*   Type to filter executables.
*   Press `Enter` to launch the selected (or first matching) command.
*   Press `Escape` to exit qlaunch.

## License

qlaunch is released under the [GNU General Public License Version 3 (GPLv3)](LICENSE).
