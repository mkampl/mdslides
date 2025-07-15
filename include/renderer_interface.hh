#pragma once

#include "slide_element.hh"
#include "theme_config.hh"
#include <string>
#include <vector>

class ISlideRenderer {
public:
    virtual ~ISlideRenderer() = default;
    
    // Core rendering
    virtual void initialize() = 0;
    virtual void cleanup() = 0;
    virtual void render_slide(const std::vector<SlideElement>& elements, bool animated = false) = 0;
    virtual void clear_screen() = 0;
    
    // UI elements
    virtual void draw_header(int current_slide, int total_slides, const std::string& theme_name, 
                           bool show_timer, int minutes, int seconds, bool utf8_mode) = 0;
    virtual void draw_footer() = 0;
    virtual void draw_progress_bar(int current_slide, int total_slides) = 0;
    virtual void show_help(bool utf8_supported) = 0;
    virtual void show_message(const std::string& message, int y = -1) = 0;
    virtual void clear_message_area() = 0;
    
    // Input handling
    virtual int get_input() = 0;
    virtual int get_screen_width() const = 0;
    virtual int get_screen_height() const = 0;
    virtual void enable_echo() = 0;
    virtual void disable_echo() = 0;
    virtual void get_string(char* buffer, int max_length) = 0;
    
    // Theme management
    virtual void apply_theme(Theme theme) = 0;
    
    // Shell output display
    virtual void update_shell_output(const SlideElement& shell_element) = 0;
    
    // Utility methods
    virtual void refresh_display() = 0;
    virtual void sleep_ms(int milliseconds) = 0;
};