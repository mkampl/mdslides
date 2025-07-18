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
#include <thread>
#include <mutex>

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
    // FTXUI components
    std::unique_ptr<ftxui::ScreenInteractive> screen;
    ftxui::Component main_component;
    
    // Rendering state
    std::vector<SlideElement> current_elements;
    std::string header_text;
    std::string footer_text;
    std::string message_text;
    std::string progress_bar_text;
    bool show_help_screen;
    
    // Theme management
    ThemeManager theme_manager;
    ftxui::Color bg_color, title_color, text_color, accent_color, code_color;
    
    // Input handling
    std::atomic<int> last_input;
    std::atomic<bool> input_ready;
    std::atomic<bool> echo_enabled;
    std::string input_buffer;
    std::mutex input_mutex;
    
    // Animation state
    std::atomic<bool> animation_active;
    std::thread animation_thread;
    
    // Helper methods
    ftxui::Element create_slide_content();
    ftxui::Element create_header_element(int current_slide, int total_slides, 
                                       const std::string &theme_name, bool show_timer, 
                                       int minutes, int seconds, bool utf8_mode);
    ftxui::Element create_footer_element();
    ftxui::Element create_progress_element(int current_slide, int total_slides);
    ftxui::Element create_help_element(bool utf8_supported);
    ftxui::Element create_message_element();
    
    ftxui::Element render_slide_element(const SlideElement &element);
    ftxui::Color get_element_color(int color_pair);
    ftxui::Element create_animated_element(const SlideElement &element, int animation_frame);
    
    void setup_theme_colors(Theme theme);
    void start_animation_loop();
    void stop_animation();
    
    // Input handling helpers
    void setup_input_handler();
    void handle_key_event(ftxui::Event &event);
};