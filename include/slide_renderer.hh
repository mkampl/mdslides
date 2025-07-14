#pragma once

#include "slide_element.hh"
#include "theme_config.hh"
#include "markdown_parser.hh"
#include <chrono>
#include <string>
#include <vector>

class MarkdownSlideRenderer {
public:
    MarkdownSlideRenderer();
    void load_slides(const std::string& filename);
    void run();
    
private:
    // Core rendering methods
    void render_slide_animated();
    void render_slide_instant();
    void render_element_animated(const SlideElement& element);
    
    // Shell command handling
    void execute_shell_commands_on_slide();
    void execute_single_shell_command(SlideElement& shell_element);
    void update_shell_output_display(const SlideElement& shell_element);
    void scroll_shell_output(int direction);
    std::string execute_shell_command(const std::string& command);
    
    // UI drawing methods
    void draw_header();
    void draw_footer();
    void draw_progress_bar();
    void clear_with_background();
    void show_help();
    void goto_slide();
    
    // Utility methods
    void safe_mvprintw(int y, int x, const std::string& text);
    void setup_unicode_support();
    
    // Member variables
    SlideCollection slides;
    MarkdownParser parser;
    ThemeManager theme_manager;
    int current_slide;
    bool show_timer;
    std::chrono::steady_clock::time_point start_time;
    std::vector<std::pair<std::string, std::string>> char_replacements;
    bool utf8_supported;
};
