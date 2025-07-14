#pragma once

#include "slide_types.hpp"
#include "markdown_parser.hpp"
#include "slide_renderer.hpp"
#include "theme_manager.hpp"
#include "shell_executor.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <memory>
#include <string>

namespace mdslides {

class PresentationController {
public:
    PresentationController();
    ~PresentationController();
    
    // Main control methods
    bool load_presentation(const std::string& filename);
    void run();
    void quit();
    
    // Navigation
    void next_slide();
    void previous_slide();
    void first_slide();
    void last_slide();
    void goto_slide(int slide_number);
    void show_goto_dialog();
    
    // Display control
    void cycle_theme();
    void toggle_animations();
    void toggle_timer();
    void toggle_help();
    void refresh_display();
    
    // Shell command execution
    void execute_shell_commands();
    void scroll_shell_output_up();
    void scroll_shell_output_down();
    
    // State access
    const PresentationState& get_state() const { return state_; }
    
private:
    PresentationState state_;
    std::shared_ptr<MarkdownParser> parser_;
    std::shared_ptr<ThemeManager> theme_manager_;
    std::shared_ptr<ShellExecutor> shell_executor_;
    std::shared_ptr<SlideRenderer> renderer_;
    
    ftxui::ScreenInteractive screen_;
    ftxui::Component main_component_;
    bool should_quit_;
    
    // Component creation
    void setup_components();
    ftxui::Component create_main_component();
    ftxui::Component create_slide_component();
    ftxui::Component create_goto_component();
    
    // Event handling
    bool handle_key_event(ftxui::Event event);
    bool handle_navigation_key(ftxui::Event event);
    bool handle_display_key(ftxui::Event event);
    bool handle_shell_key(ftxui::Event event);
    bool handle_goto_key(ftxui::Event event);
    
    // UI updates
    void update_display();
    void execute_shell_commands_on_current_slide();
    
    // Input validation
    bool is_valid_slide_number(const std::string& input) const;
    int parse_slide_number(const std::string& input) const;
    
    // Initialization
    void initialize_state();
    void detect_utf8_support();
};

} // namespace mdslides
