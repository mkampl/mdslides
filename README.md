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
- Shell command execution within slides

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

### Linux (Ubuntu/Debian)
```bash
# Install dependencies
sudo apt update
sudo apt install -y cmake build-essential libncurses-dev

# Clone and build
git clone <repository-url>
cd markdown-slide-presenter
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
./slides ../README.md
```

---

## Usage

### Basic Usage
```bash
# Run with markdown file
./slides presentation.md
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
- Enter - Execute shell commands on current slide
- 'u' / 'd' - Scroll shell output up/down

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

### Character Replacements

The application includes built-in fallback replacements for:
- German Umlauts (ä→ae, ö→oe, ü→ue, ß→ss)
- Common symbols (→→->, ←→<-, •→*)
- French accents (é→e, è→e, ç→c, à→a)
- Spanish characters (ñ→n, í→i, ó→o)
- Many Unicode symbols and characters


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

```markdown
### System Information
```$uname -a
```

```$df -h
```
```

Commands are executed when you press Enter on the slide.

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

## Troubleshooting

### Common Issues

#### "ncurses not found"
```bash
# Ubuntu/Debian
sudo apt install libncurses-dev
```

#### Unicode Characters Not Displaying
- The application automatically detects UTF-8 support
- If characters appear as replacements, your terminal may not support UTF-8
- Try using a modern terminal emulator
- Check your LANG environment variable: `echo $LANG`

#### Character Replacement Issues
- The application uses built-in fallbacks automatically

---

## Development

### Project Structure
```
markdown-slide-presenter/
├── slides.cpp           # Main source code
├── CMakeLists.txt       # Build configuration
├── README.md            # Documentation
```
---

## Dependencies

### Runtime Dependencies
- **ncurses**: Terminal user interface library
- **C++ Runtime**: Standard library support

### Build Dependencies
- **CMake**: Build system generator (3.15+)
- **C++ Compiler**: GCC 7+, Clang 7+, or MSVC 2019+


---

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

---

## License

This project is open source. See the source code for details.
