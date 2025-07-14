# Markdown Slide Presenter

A modern terminal-based presentation tool that renders markdown files as interactive slides with animations, themes, and shell command execution. Built with FTXUI and cmark for a smooth, responsive experience.

---

## Features

### Core Functionality
- **Modern UI**: Built with FTXUI for smooth, responsive terminal interface
- **Markdown parsing**: Powered by cmark for robust CommonMark support
- **Unicode support**: Full UTF-8 with intelligent ASCII fallback
- **Multiple themes**: Dark, Light, Matrix, and Retro themes
- **Slide animations**: Fade-in, slide-in, and typewriter effects
- **Navigation controls**: Intuitive keyboard navigation
- **Progress tracking**: Progress bar and optional timer
- **Shell execution**: Safe shell command execution within slides

### Supported Markdown Elements
- Headers (H1, H2, H3) with automatic styling
- Bullet points and numbered lists
- Bold text formatting (**text**)
- Code blocks with syntax highlighting
- Interactive shell commands (```$command```)

---

## Installation

### Prerequisites
- **CMake** 3.15 or higher
- **C++17** compatible compiler (GCC 7+, Clang 7+, MSVC 2019+)
- **FTXUI** library
- **cmark** library (CommonMark parser)

### Linux (Ubuntu/Debian)
```bash
# Install system dependencies
sudo apt update
sudo apt install -y cmake build-essential libcmark-dev

# Install FTXUI (you may need to build from source or use vcpkg/conan)
# Option 1: Using vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg && ./bootstrap-vcpkg.sh
./vcpkg install ftxui

# Option 2: Build FTXUI from source
git clone https://github.com/ArthurSonzogni/FTXUI.git
cd FTXUI && mkdir build && cd build
cmake .. && make -j$(nproc)
sudo make install

# Build the presentation tool
git clone https://github.com/yourusername/mdslides.git
cd mdslides
mkdir build && cd build
cmake ..
make -j$(nproc)

# Install (optional)
sudo make install
```

### Using Package Managers

#### With vcpkg
```bash
# Install dependencies
vcpkg install ftxui cmark

# Build with CMake
cmake -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake ..
make
```

#### With Conan
```python
# conanfile.txt
[requires]
ftxui/5.0.0
cmark/0.30.2

[generators]
CMakeDeps
CMakeToolchain
```

```bash
conan install . --build=missing
cmake --preset conan-default
cmake --build --preset conan-default
```

---

## Quick Start

### Basic Usage
```bash
# Run with a markdown file
./mdslides presentation.md

# Show help
./mdslides --help

# Show version
./mdslides --version
```

### Example Markdown Format
```markdown
# Welcome to My Presentation
This is the title slide

---

## Agenda
- Introduction
- **Key Features**
- Live Demo
- Questions

---

### Code Example
```cpp
#include <iostream>

int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
```

---

### Live Shell Commands
Check the current directory:
```$pwd
```

List files:
```$ls -la
```

Show system information:
```$uname -a
```

---

## Navigation

### Slide Navigation
- **→ / Space / l** - Next slide
- **← / Backspace / h** - Previous slide  
- **g** - Go to specific slide number
- **Home / 0** - First slide
- **End / $** - Last slide

### Shell Commands
- **ENTER** - Execute shell commands on current slide
- **u / d** - Scroll shell output up/down

### Display Options
- **t** - Cycle through themes (Dark → Light → Matrix → Retro)
- **a** - Toggle animations on/off
- **T** - Toggle timer display
- **r** - Refresh/redraw screen

### Other Controls
- **h / ?** - Show help screen
- **q / Escape** - Quit application

---

## Advanced Features

### Shell Command Security
The application includes built-in security measures:
- **Command filtering**: Dangerous commands (rm, sudo, etc.) are blocked
- **Input sanitization**: Commands are cleaned before execution  
- **Safe execution**: Commands run in controlled environment
- **Output limits**: Shell output is truncated to prevent overflow

### Unicode and Character Support
- **Automatic detection**: UTF-8 support detected from environment
- **Intelligent fallback**: Automatic character replacement in ASCII mode
- **Built-in replacements**: Comprehensive replacement map for common Unicode characters
- **Mode display**: Current mode (UTF-8/ASCII) shown in header

### Themes
1. **Dark**: Cyan headers on black background (default)
2. **Light**: Blue headers on white background  
3. **Matrix**: Green-on-black terminal style
4. **Retro**: Yellow and cyan retro computing aesthetic

### Animation Types
- **Fade-in**: Gradual appearance with opacity changes
- **Slide-in**: Elements slide from right to left  
- **Typewriter**: Character-by-character typing effect
- **None**: Instant display (toggle with 'a')

---

## Project Structure

```
mdslides/
├── CMakeLists.txt              # Modern CMake build configuration
├── README.md                   # This documentation
├── include/                    # Header files
│   ├── slide_types.hpp         # Core data types and enums
│   ├── markdown_parser.hpp     # Markdown parsing interface
│   ├── slide_renderer.hpp      # FTXUI rendering engine
│   ├── theme_manager.hpp       # Theme system
│   ├── shell_executor.hpp      # Safe shell execution
│   └── presentation_controller.hpp # Main application controller
└── src/                        # Implementation files
    ├── main.cpp                # Application entry point
    ├── markdown_parser.cpp     # cmark-based parser implementation  
    ├── slide_renderer.cpp      # FTXUI rendering implementation
    ├── theme_manager.cpp       # Theme management
    ├── shell_executor.cpp      # Shell command execution
    └── presentation_controller.cpp # Event handling and navigation
```

---

## Dependencies

### Runtime Dependencies
- **FTXUI**: Modern terminal user interface library
- **cmark**: CommonMark markdown parser
- **C++ Standard Library**: C++17 features

### Build Dependencies  
- **CMake**: Build system generator (3.15+)
- **C++ Compiler**: Supporting C++17 standard
- **pkg-config**: For dependency detection

### Optional Dependencies
- **vcpkg** or **Conan**: For easier dependency management

---

## Building from Source

### Traditional Build
```bash
# Clone repository
git clone https://github.com/yourusername/mdslides.git
cd mdslides

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
make -j$(nproc)

# Run
./mdslides ../README.md
```

### Debug Build
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

### Release Build
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

---

## Troubleshooting

### Common Issues

#### "FTXUI not found"
```bash
# Install FTXUI via vcpkg
vcpkg install ftxui

# Or build from source
git clone https://github.com/ArthurSonzogni/FTXUI.git
cd FTXUI && mkdir build && cd build
cmake .. && make && sudo make install
```

#### "cmark not found"  
```bash
# Ubuntu/Debian
sudo apt install libcmark-dev

# Fedora/CentOS
sudo dnf install cmark-devel

# macOS
brew install cmark
```

#### Unicode Characters Not Displaying
- Check your terminal supports UTF-8: `echo $LANG`
- Try setting: `export LANG=en_US.UTF-8`
- The application automatically falls back to ASCII replacements

#### Shell Commands Not Executing
- Commands are filtered for security
- Check that your command doesn't contain dangerous patterns
- Simple commands like `ls`, `pwd`, `date` should work

---

## Contributing

Contributions are welcome! The modular architecture makes it easy to:

- **Add new themes** in `theme_manager.cpp`
- **Extend markdown support** in `markdown_parser.cpp`  
- **Add animations** in `slide_renderer.cpp`
- **Improve shell security** in `shell_executor.cpp`

### Development Setup
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

---

## License

This project is open source. See the LICENSE file for details.

---

## Acknowledgments

- **FTXUI**: Modern terminal UI framework by Arthur Sonzogni
- **cmark**: Reference implementation of CommonMark
- **Original inspiration**: Terminal-based presentation tools like `present` and `slides`
