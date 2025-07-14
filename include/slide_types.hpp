#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <ftxui/dom/elements.hpp>

namespace mdslides {

enum class Theme {
    DARK,
    LIGHT, 
    MATRIX,
    RETRO
};

enum class AnimationType {
    NONE,
    FADE_IN,
    SLIDE_IN,
    TYPEWRITER
};

enum class ElementType {
    TEXT,
    HEADER1,
    HEADER2, 
    HEADER3,
    BULLET,
    NUMBERED,
    CODE_BLOCK,
    SHELL_COMMAND,
    SHELL_OUTPUT
};

struct ThemeConfig {
    ftxui::Color bg_color;
    ftxui::Color title_color;
    ftxui::Color subtitle_color;
    ftxui::Color text_color;
    ftxui::Color accent_color;
    ftxui::Color code_color;
    std::string name;
};

struct SlideElement {
    std::string content;
    ElementType type = ElementType::TEXT;
    AnimationType animation = AnimationType::FADE_IN;
    int delay_ms = 0;
    bool is_bold = false;
    
    // Shell command specific
    std::string shell_command;
    bool executed = false;
    std::vector<std::string> shell_output_lines;
    int output_scroll_offset = 0;
    int max_output_lines = 5;
    
    // Constructor
    SlideElement() = default;
    SlideElement(const std::string& text, ElementType elem_type) 
        : content(text), type(elem_type) {}
};

using Slide = std::vector<SlideElement>;

struct PresentationState {
    std::vector<Slide> slides;
    int current_slide = 0;
    Theme current_theme = Theme::DARK;
    bool show_timer = false;
    bool use_animations = true;
    std::chrono::steady_clock::time_point start_time;
    bool utf8_supported = false;
    
    // Navigation state
    bool help_visible = false;
    bool goto_dialog_visible = false;
    std::string goto_input;

    bool shell_confirmation_visible = false;
    std::string pending_shell_command;
    
    // Methods
    bool has_next_slide() const {
        return current_slide < static_cast<int>(slides.size()) - 1;
    }
    
    bool has_previous_slide() const {
        return current_slide > 0;
    }
    
    void next_slide() {
        if (has_next_slide()) {
            current_slide++;
        }
    }
    
    void previous_slide() {
        if (has_previous_slide()) {
            current_slide--;
        }
    }
    
    void first_slide() {
        current_slide = 0;
    }
    
    void last_slide() {
        current_slide = static_cast<int>(slides.size()) - 1;
    }
    
    bool goto_slide(int slide_number) {
        if (slide_number >= 1 && slide_number <= static_cast<int>(slides.size())) {
            current_slide = slide_number - 1;
            return true;
        }
        return false;
    }
    
    Slide& get_current_slide() {
        return slides[current_slide];
    }
    
    const Slide& get_current_slide() const {
        return slides[current_slide];
    }
    
    int get_elapsed_seconds() const {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - start_time);
        return static_cast<int>(duration.count());
    }
    
    void cycle_theme() {
        int theme_count = 4; // DARK, LIGHT, MATRIX, RETRO
        current_theme = static_cast<Theme>((static_cast<int>(current_theme) + 1) % theme_count);
    }
    
    void toggle_animations() {
        use_animations = !use_animations;
    }
    
    void toggle_timer() {
        show_timer = !show_timer;
    }
    
    void toggle_help() {
        help_visible = !help_visible;
    }
    
    void show_goto_dialog() {
        goto_dialog_visible = true;
        goto_input.clear();
    }
    
    void hide_goto_dialog() {
        goto_dialog_visible = false;
        goto_input.clear();
    }
};

} // namespace mdslides
