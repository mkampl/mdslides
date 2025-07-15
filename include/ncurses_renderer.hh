#pragma once

#include "renderer_interface.hh"
#include "theme_config.hh"
#include <vector>
#include <string>

class NCursesRenderer : public ISlideRenderer {
public:
    NCursesRenderer();
    ~NCursesRenderer() override;
    
    // ISlideRenderer interface implementation
    void initialize() override;
    void cleanup() override;
    void render_slide(const std::vector<SlideElement>& elements, bool animated = false) override;
    void clear_screen() override;
    
    void draw_header(int current_slide, int total_slides, const std::string& theme_name, 
                    bool show_timer, int minutes, int seconds, bool utf8_mode) override;
    void draw_footer() override;
    void draw_progress_bar(int current_slide, int total_slides) override;
    void show_help(bool utf8_supported) override;
    void show_message(const std::string& message, int y = -1) override;
    void clear_message_area() override;
    
    int get_input() override;
    int get_screen_width() const override;
    int get_screen_height() const override;
    void enable_echo() override;
    void disable_echo() override;
    void get_string(char* buffer, int max_length) override;
    
    void apply_theme(Theme theme) override;
    void update_shell_output(const SlideElement& shell_element) override;
    void refresh_display() override;
    void sleep_ms(int milliseconds) override;
    
    // UTF-8 support
    void set_utf8_support(bool enabled);
    
private:
    // Private rendering methods
    void safe_mvprintw(int y, int x, const std::string& text);
    void render_element_animated(const SlideElement& element);
    void render_element_instant(const SlideElement& element);
    void clear_with_background();
    void clear_with_background(int start_line,int end_line);
    bool check_for_input_during_animation();
    
    // Theme management
    ThemeManager theme_manager;
    
    // UTF-8 and character handling
    bool utf8_supported;
    std::vector<std::pair<std::string, std::string>> char_replacements;
    void load_char_replacements();
    bool detect_utf8_support();
};