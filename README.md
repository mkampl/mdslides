# Markdown Slide Presenter

A terminal-based presentation tool that renders markdown files as interactive slides with animations, themes, and shell command execution.

---

## Features

### Core Functionality
- Markdown parsing and rendering
- Unicode character support (UTF-8) with ASCII fallback
- Multiple themes (Dark, Light, Matrix, Retro)
- Slide animations (Fade-in, Slide-in, Typewriter)
- Navigation controls
- Progress bar and timer
- Interactive shell command execution with popup windows

### Supported Markdown Elements
- Headers (H1, H2, H3)
- Bullet points and numbered lists
- Bold text formatting
- Code blocks
- Interactive shell commands

---

## Installation

### Prerequisites
- CMake 3.15 or higher
- C++17 compatible compiler
- ncurses library
- cmark-gfm library

### Linux (Ubuntu/Debian)
```bash
# Install dependencies
sudo apt update
sudo apt install -y cmake build-essential libncurses-dev libcmark-gfm-dev

# Clone and build
git clone git@github.com:mkampl/mdslides.git
cd mdslides
mkdir build && cd build
cmake ..
make

# Install (optional)
sudo make install
```

---

## Quick Start

### Building from Source
```bash
# Universal build steps
mkdir build && cd build
cmake ..
make

# Test the build
./mdslides ../README.md
```

---

## Usage

### Basic Usage
```bash
# Run with markdown file
./mdslides presentation.md
```

### Markdown Format
```markdown
# Title Slide
This is the main title

---

## Content Slide
- First bullet point
- Second bullet point
- **Bold text example**

---

### Code Example
```cpp
int main() {
    return 0;
}
```

---

### Shell Commands
```$ls -la
```

```$date
```
---

## Navigation Controls

### Slide Navigation
- Right Arrow / Space / 'l' - Next slide
- Left Arrow / Backspace / 'h' - Previous slide
- 'g' - Go to specific slide number
- Home / '0' - First slide
- End / '$' - Last slide

### Shell Commands
- Enter - Select and execute shell commands
- ↑/↓ - Navigate between multiple shell commands on a slide
- Escape - Cancel shell command selection
- In popup: ↑/↓, PgUp/PgDn - Scroll output
- In popup: Escape - Close popup window

### Display Options
- 't' - Cycle through themes
- 'a' - Toggle animations
- 'T' - Toggle timer display
- 'r' - Refresh/redraw screen

### Other Controls
- 'h' or '?' - Show help screen
- 'q' or Escape - Quit application

---

## Character Support

The application automatically detects Unicode (UTF-8) support in your terminal:

### Unicode Mode (UTF-8)
- Native display of Unicode characters
- Full international character support
- Special symbols and characters displayed as intended

### ASCII Fallback Mode
- Automatic character replacement when Unicode is not supported
- Built-in replacement map for common characters
- Optional external configuration file support

---

## Themes

- **Dark**: Cyan titles on black background
- **Light**: Blue titles on white background  
- **Matrix**: Green on black terminal style
- **Retro**: Yellow and cyan retro computing style

The current mode (UTF-8 or ASCII) is displayed in the header for reference.

---

## Advanced Features

### Shell Command Execution
Use special code blocks to execute shell commands:

```$uname -a
```

```$df -h
```

Commands are executed in a popup window when you press Enter. For slides with multiple shell commands, use arrow keys to select which command to execute.

### Animation Types
- **Fade-in**: Gradual appearance effect
- **Slide-in**: Elements slide from right to left
- **Typewriter**: Character-by-character typing effect

---

## Platform-Specific Notes

### Linux
- Requires ncurses development headers
- UTF-8 support in most modern terminals
- Works in most terminal emulators

---

## Development

### Project Structure
```
markdown-slide-presenter/
├── src/
│   ├── main.cc                    # Main application entry point
│   ├── slide_renderer.cc          # Main slide rendering logic
│   ├── ncurses_renderer.cc        # NCurses-based terminal rendering
│   ├── markdown_parser.cc         # Markdown parsing with cmark-gfm
│   ├── slide_element.cc           # Slide element data structures
│   ├── theme_config.cc            # Theme configuration
│   ├── shell_command_selector.cc  # Shell command selection system
│   └── shell_popup.cc             # Shell command popup window
├── include/
│   ├── slide_renderer.hh          # Main renderer interface
│   ├── ncurses_renderer.hh        # NCurses renderer header
│   ├── markdown_parser.hh         # Markdown parser header
│   ├── slide_element.hh           # Slide element definitions
│   ├── theme_config.hh            # Theme configuration header
│   ├── shell_command_selector.hh  # Shell command selector header
│   └── shell_popup.hh             # Shell popup header
├── CMakeLists.txt                 # Build configuration
└── README.md                      # Documentation
```
---

## Dependencies

## Dependencies

This project uses the following third-party libraries:

- **[FTXUI](https://github.com/ArthurSonzogni/FTXUI)** - C++ Functional Terminal User Interface library (MIT License)
- **[ncurses](https://invisible-island.net/ncurses/)** - Terminal UI library (MIT-style License)
- **[cmark-gfm](https://github.com/github/cmark-gfm)** - GitHub Flavored Markdown parser (BSD-2-Clause License)



---

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

---

## License

This project is licensed under the BSD-3-Clause License - see the [LICENSE](LICENSE) file for details.

Third-party licenses can be found in [LICENSE-THIRD-PARTY](LICENSE-THIRD-PARTY).