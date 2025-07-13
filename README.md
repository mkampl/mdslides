# Markdown Slide Presenter

A terminal-based presentation tool that renders markdown files as interactive slides with animations, themes, and shell command execution.

---

## Features

### Core Functionality
- Markdown parsing and rendering
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
- ncurses library (platform-specific installation below)

### Linux (Ubuntu/Debian)
```bash
# Install dependencies
sudo apt update
sudo apt install -y cmake build-essential libncurses-dev libncursesw5-dev pkg-config

# Clone and build
git clone <repository-url>
cd markdown-slide-presenter
mkdir build && cd build
cmake ..
make

# Install (optional)
sudo make install
```

### Linux (CentOS/RHEL/Fedora)
```bash
# For CentOS/RHEL 8+
sudo dnf install -y cmake gcc-c++ ncurses-devel pkgconfig

# For older CentOS/RHEL
sudo yum install -y cmake3 gcc-c++ ncurses-devel pkgconfig

# For Fedora
sudo dnf install -y cmake gcc-c++ ncurses-devel pkgconfig

# Build
mkdir build && cd build
cmake ..
make
```

### macOS
```bash
# Install Xcode command line tools
xcode-select --install

# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake ncurses pkg-config

# Build
mkdir build && cd build
cmake ..
make
```

### Windows (MinGW/MSYS2)
```bash
# Install MSYS2 from https://www.msys2.org/
# In MSYS2 terminal:
pacman -S mingw-w64-x86_64-cmake mingw-w64-x86_64-gcc mingw-w64-x86_64-ncurses

# Build
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```

### Windows (Visual Studio)
```bash
# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.bat

# Install ncurses alternative (PDCurses)
./vcpkg install pdcurses:x64-windows

# Build with CMake
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build .
```

---

## Quick Start

### Building from Source
```bash
# Universal build steps for Unix-like systems
mkdir build && cd build
cmake ..
make

# Test the build
./slides ../README.md
```

### Pre-built Binaries
Check the releases page for pre-built binaries for your platform.

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
- End / ' - Last slide

### Shell Commands
- Enter - Execute shell commands on current slide
- 'u' / 'd' - Scroll shell output up/down

### Display Options
- 't' - Cycle through themes
- 'a' - Toggle animations
- 'T' - Toggle timer display
- 'r' - Refresh/redraw screen

### Other Controls
- 'c' - Reload character configuration
- 'h' or '?' - Show help screen
- 'q' or Escape - Quit application

---

## Configuration

### Character Replacement
Create a `slide_config.txt` file for custom character replacements:

```
# Character replacement configuration
# Format: "search" -> "replacement"

# German Umlauts
ä -> ae
ö -> oe
ü -> ue

# Symbols
→ -> ->
← -> <-
• -> *
```

### Themes
- **Dark**: Cyan titles on black background
- **Light**: Blue titles on white background  
- **Matrix**: Green on black terminal style
- **Retro**: Yellow and cyan retro computing style

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
- Wide character support recommended (ncursesw)
- Works in most terminal emulators

### macOS
- Tested on macOS 10.15+ 
- Works with Terminal.app and iTerm2
- Homebrew installation recommended

### Windows
- WSL (Windows Subsystem for Linux) recommended
- Native Windows support via PDCurses
- ConEmu or Windows Terminal recommended

### FreeBSD/OpenBSD
```bash
# FreeBSD
pkg install cmake ncurses pkgconf

# OpenBSD  
pkg_add cmake ncurses pkgconf
```

---

## Troubleshooting

### Common Issues

#### "ncurses not found"
```bash
# Ubuntu/Debian
sudo apt install libncurses-dev libncursesw5-dev

# CentOS/RHEL/Fedora
sudo dnf install ncurses-devel

# macOS
brew install ncurses

# Arch Linux
sudo pacman -S ncurses
```

#### "cmake: command not found"
```bash
# Ubuntu/Debian
sudo apt install cmake

# CentOS/RHEL
sudo dnf install cmake

# macOS
brew install cmake
```

#### Unicode Characters Not Displaying
- Ensure your terminal supports UTF-8
- Use a terminal with Unicode support
- Check the slide_config.txt for character replacements

#### Compilation Errors
- Ensure C++17 compiler support
- Check CMake version (3.15+ required)
- Verify ncurses development headers are installed

---

## Development

### Project Structure
```
markdown-slide-presenter/
├── slides.cpp           # Main source code
├── CMakeLists.txt       # Build configuration
├── README.md            # Documentation
├── slide_config.txt     # Character configuration
└── build/               # Build directory
```

### Building with Custom Options
```bash
# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build with optimizations
cmake -DCMAKE_BUILD_TYPE=Release ..

# Specify custom ncurses location
cmake -DCMAKE_PREFIX_PATH=/custom/path ..
```

### Cross-Platform Development
- Code uses standard C++17 features
- ncurses provides cross-platform terminal handling
- CMake handles platform-specific differences
- Tested on Linux, macOS, Windows (WSL), and BSD

---

## Dependencies

### Runtime Dependencies
- **ncurses**: Terminal user interface library
- **C++ Runtime**: Standard library support

### Build Dependencies
- **CMake**: Build system generator (3.15+)
- **C++ Compiler**: GCC 7+, Clang 7+, or MSVC 2019+
- **pkg-config**: Package configuration (Unix systems)

### Optional Dependencies
- **git**: For cloning the repository
- **make**: Build tool (Unix systems)
- **ninja**: Alternative build tool

---

## Examples

### Simple Presentation
```markdown
# Welcome
This is my presentation

---

## Agenda
- Introduction
- Main content  
- Conclusion

---

## Thank You
Questions?
```

### Technical Presentation
```markdown
# System Status
Live system monitoring

---

## Disk Usage
```$df -h
```

---

## Memory Info
```$free -h
```

---

## Process List
```$ps aux | head -10
```
```

---

## Packaging

### Creating Distribution Packages
```bash
# After building
make package

# This creates platform-specific packages:
# Linux: .tar.gz, .deb, .rpm
# macOS: .tar.gz, .dmg
# Windows: .zip, .exe installer
```

---

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

### Development Guidelines
- Follow C++17 standards
- Maintain cross-platform compatibility
- Test on multiple terminal emulators
- Document new features in README
- Use consistent code formatting

### Testing Platforms
- Ubuntu 20.04+ / Debian 11+
- CentOS 8+ / RHEL 8+ / Fedora 35+
- macOS 10.15+
- Windows 10+ (WSL)
- FreeBSD 13+ / OpenBSD 7+

---

## License

This project is open source. See the source code for details.
