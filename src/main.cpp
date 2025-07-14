#include "presentation_controller.hpp"
#include <iostream>
#include <string>
#include <locale.h>

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " <markdown_file>\n\n";
    std::cout << "Example markdown format:\n";
    std::cout << "# Title Slide\n";
    std::cout << "This is the content\n";
    std::cout << "---\n";
    std::cout << "## Second Slide\n";
    std::cout << "- Bullet point 1\n";
    std::cout << "- Bullet point 2\n";
    std::cout << "---\n";
    std::cout << "### Code Example\n";
    std::cout << "```cpp\n";
    std::cout << "int main() {\n";
    std::cout << "    return 0;\n";
    std::cout << "}\n";
    std::cout << "```\n";
    std::cout << "---\n";
    std::cout << "### Shell Command Demo\n";
    std::cout << "```$ls -la\n";
    std::cout << "```\n";
    std::cout << "```$date\n";
    std::cout << "```\n\n";
    
    std::cout << "Controls:\n";
    std::cout << "  →/← or Space/Backspace  Navigate slides\n";
    std::cout << "  ENTER                   Execute shell commands\n";
    std::cout << "  u/d                     Scroll shell output\n";
    std::cout << "  t                       Cycle themes\n";
    std::cout << "  g                       Go to specific slide\n";
    std::cout << "  h or ?                  Show help\n";
    std::cout << "  q or ESC                Quit\n";
}

void print_version() {
    std::cout << "Markdown Slide Presenter v1.0.0\n";
    std::cout << "Modern terminal-based presentation tool\n";
    std::cout << "Built with FTXUI and cmark\n";
}

int main(int argc, char* argv[]) {
    // Set up locale for UTF-8 support
    setlocale(LC_ALL, "");
    
    // Parse command line arguments
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    std::string arg = argv[1];
    if (arg == "--help" || arg == "-h") {
        print_usage(argv[0]);
        return 0;
    }
    
    if (arg == "--version" || arg == "-v") {
        print_version();
        return 0;
    }
    
    if (argc != 2) {
        std::cerr << "Error: Too many arguments\n";
        print_usage(argv[0]);
        return 1;
    }
    
    std::string filename = argv[1];
    
    try {
        // Create and initialize the presentation controller
        mdslides::PresentationController controller;
        
        // Load the presentation
        if (!controller.load_presentation(filename)) {
            std::cerr << "Error: Could not load presentation file: " << filename << "\n";
            std::cerr << "Please check that the file exists and is readable.\n";
            return 1;
        }
        
        // Check if slides were loaded
        if (controller.get_state().slides.empty()) {
            std::cerr << "Error: No slides found in the presentation file.\n";
            std::cerr << "Please check the markdown format (slides should be separated by '---').\n";
            return 1;
        }
        
        // Run the presentation
        controller.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred.\n";
        return 1;
    }
    
    return 0;
}
