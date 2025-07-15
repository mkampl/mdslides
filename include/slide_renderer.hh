#pragma once

#include "slide_element.hh"
#include "theme_config.hh"
#include "markdown_parser.hh"
#include "renderer_interface.hh"
#include <chrono>
#include <string>
#include <memory>

class MarkdownSlideRenderer {
public:
    MarkdownSlideRenderer();
    void load_slides(const std::string& filename);
    void run();
    
private:
    // Shell command handling
    void execute_shell_commands_on_slide();
    void execute_single_shell_command(SlideElement& shell_element);
    void scroll_shell_output(int direction);
    std::string execute_shell_command(const std::string& command);
    
    // Navigation and UI
    void goto_slide();
    void render_current_slide(bool animated);
    
    // Member variables
    Theme current_theme;
    SlideCollection slides;
    MarkdownParser parser;
    std::unique_ptr<ISlideRenderer> renderer;
    int current_slide;
    bool show_timer;
    std::chrono::steady_clock::time_point start_time;
    bool utf8_supported;
};