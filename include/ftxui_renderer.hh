#pragma once

#include "renderer_interface.hh"
#include "theme_config.hh"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <vector>
#include <string>
#include <memory>
#include <atomic>

class FTXUIRenderer : public ISlideRenderer
{
public:
    FTXUIRenderer();
    ~FTXUIRenderer() override;

    // ISlideRenderer interface implementation
    void initialize() override;
    void cleanup() override;
    void render_slide(const std::vector<SlideElement> &elements, bool animated = false) override;
    void clear_screen() override;

    void draw_header(int current_slide, int total_slides, const std::string &theme_name,
                     bool show_timer, int minutes, int seconds, bool utf8_mode) override;
    void draw_footer() override;
    void draw_progress_bar(int current_slide, int total_slides) override;
    void show_help(bool utf8_supported) override;
    void show_message(const std::string &message, int y = -1) override;
    void clear_message_area() override;

    int get_input() override;
    int get_screen_width() const override;
    int get_screen_height() const override;
    void enable_echo() override;
    void disable_echo() override;
    void get_string(char *buffer, int max_length) override;

    void apply_theme(Theme theme) override;
    void refresh_display() override;
    void sleep_ms(int milliseconds) override;

    // UTF-8 support (FTXUI has native UTF-8 support)
    void set_utf8_support(bool enabled);

private:
    // Application state
    enum class AppState {
        MAIN_VIEW,
        HELP_VIEW,
        GOTO_DIALOG
    };
    
    AppState current_state;
    bool app_running;
    bool screen_initialized;
    
    // Rendering state
    std::vector<SlideElement> current_elements;
    std::string header_text;
    std::string footer_text;
    std::string message_text;
    std::string progress_bar_text;
    std::string goto_input;
    
    // Theme management
    ThemeManager theme_manager;
    ftxui::Color bg_color, title_color, text_color, accent_color, code_color;
    
    // Input handling
    std::atomic<int> last_input;
    std::atomic<bool> input_ready;
    
    // FTXUI components - NOT using unique_ptr since ScreenInteractive is not moveable
    ftxui::Component main_component;
    
    // Helper methods
    ftxui::Element create_main_view();
    ftxui::Element create_help_view();
    ftxui::Element create_goto_dialog();
    ftxui::Element create_slide_content();
    ftxui::Element create_help_element(bool utf8_supported);
    
    ftxui::Element render_slide_element(const SlideElement &element);
    
    void setup_theme_colors(Theme theme);
    bool handle_event(ftxui::Event event);
    void setup_main_component();
};