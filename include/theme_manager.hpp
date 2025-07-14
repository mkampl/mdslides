#pragma once

#include "slide_types.hpp"
#include <ftxui/dom/elements.hpp>
#include <vector>
#include <map>

namespace mdslides {

class ThemeManager {
public:
    ThemeManager();
    
    // Theme management
    const ThemeConfig& get_current_theme() const;
    const ThemeConfig& get_theme(Theme theme) const;
    void set_theme(Theme theme);
    Theme get_current_theme_type() const { return current_theme_; }
    
    // Color helpers for FTXUI elements
    ftxui::Element style_text(const std::string& text, ElementType type) const;
    ftxui::Element style_header1(const std::string& text) const;
    ftxui::Element style_header2(const std::string& text) const;
    ftxui::Element style_header3(const std::string& text) const;
    ftxui::Element style_bullet(const std::string& text) const;
    ftxui::Element style_code(const std::string& text) const;
    ftxui::Element style_shell_command(const std::string& text) const;
    ftxui::Element style_shell_output(const std::string& text, bool executed = true) const;
    ftxui::Element style_accent(const std::string& text) const;
    
    // Background and container styling
    ftxui::Element create_background() const;
    ftxui::Element create_slide_container(ftxui::Element content) const;
    
    // Status and UI elements
    ftxui::Element style_status_bar(const std::string& text) const;
    ftxui::Element style_progress_bar(float progress) const;
    ftxui::Element style_help_text(const std::string& text) const;
    
    // Color getters
    ftxui::Color get_background_color() const;
    ftxui::Color get_text_color() const;
    ftxui::Color get_accent_color() const;
    
private:
    Theme current_theme_;
    std::map<std::string, ThemeConfig> themes_;
    
    void initialize_themes();
    ftxui::Element apply_element_style(const std::string& text, ftxui::Color color, bool bold = false) const;
};

} // namespace mdslides
