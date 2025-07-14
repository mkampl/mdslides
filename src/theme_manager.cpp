#include "theme_manager.hpp"
#include <ftxui/dom/elements.hpp>

using namespace ftxui;
// using namespace mdslides;

namespace mdslides {

ThemeManager::ThemeManager() : current_theme_(Theme::DARK) {
    initialize_themes();
}

void ThemeManager::initialize_themes() {
    themes_["Dark"] = ThemeConfig{
        Color::Black,        // bg_color
        Color::Cyan,         // title_color
        Color::Yellow,       // subtitle_color
        Color::White,        // text_color
        Color::Green,        // accent_color
        Color::Magenta,      // code_color
        "Dark"              // name
    };

    themes_["Light"] = ThemeConfig{
        Color::White,        // bg_color
        Color::Blue,         // title_color
        Color::Red,          // subtitle_color
        Color::Black,        // text_color
        Color::Green,        // accent_color
        Color::Magenta,      // code_color
        "Light"             // name
    };

    themes_["Matrix"] = ThemeConfig{
        Color::Black,        // bg_color
        Color::Green,        // title_color
        Color::Green,        // subtitle_color
        Color::Green,        // text_color
        Color::White,        // accent_color
        Color::Green,        // code_color
        "Matrix"            // name
    };

    themes_["Retro"] = ThemeConfig{
        Color::Black,        // bg_color
        Color::Yellow,       // title_color
        Color::Cyan,         // subtitle_color
        Color::White,        // text_color
        Color::Magenta,      // accent_color
        Color::Red,          // code_color
        "Retro"             // name
    };
}

const ThemeConfig& ThemeManager::get_current_theme() const {
    std::string theme_name;
    
    // Convert enum to string (adjust enum values to match your actual enum)
    switch(current_theme_) {
        case Theme::DARK:
            theme_name = "Dark";
            break;
        case Theme::LIGHT:  
            theme_name = "Light";
            break;
        case Theme::MATRIX:
            theme_name = "Matrix";
            break;
        case Theme::RETRO:
            theme_name = "Retro";
            break;
        default:
            theme_name = "Dark"; // fallback
            break;
    }
    
    auto it = themes_.find(theme_name);
    if (it != themes_.end()) {
        return it->second;
    }
    
    // Fallback to first theme if not found
    return themes_.begin()->second;
}


const ThemeConfig& ThemeManager::get_theme(Theme theme) const {
   std::string theme_name;
    switch(theme) {
        case Theme::DARK: theme_name = "Dark"; break;
        case Theme::LIGHT: theme_name = "Light"; break;
        case Theme::MATRIX: theme_name = "Matrix"; break;
        case Theme::RETRO: theme_name = "Retro"; break;
        default: theme_name = "Dark"; break;
    }
    
    return themes_.at(theme_name);
}

void ThemeManager::set_theme(Theme theme) {
    current_theme_ = theme;
}

Element ThemeManager::apply_element_style(const std::string& text, Color color, bool bold) const {
    Element elem = ftxui::text(text);
    elem = elem | ftxui::color(color);
    if (bold) {
        elem = elem | ftxui::bold;
    }
    return elem;
}

Element ThemeManager::style_text(const std::string& text, ElementType type) const {
    const auto& theme = get_current_theme();
    
    switch (type) {
        case ElementType::HEADER1:
            return style_header1(text);
        case ElementType::HEADER2:
            return style_header2(text);
        case ElementType::HEADER3:
            return style_header3(text);
        case ElementType::BULLET:
        case ElementType::NUMBERED:
            return style_bullet(text);
        case ElementType::CODE_BLOCK:
            return style_code(text);
        case ElementType::SHELL_COMMAND:
            return style_shell_command(text);
        case ElementType::SHELL_OUTPUT:
            return style_shell_output(text);
        default:
            return apply_element_style(text, theme.text_color);
    }
}

Element ThemeManager::style_header1(const std::string& text) const {
    const auto& theme = get_current_theme();
    return apply_element_style(text, theme.title_color, true);
}

Element ThemeManager::style_header2(const std::string& text) const {
    const auto& theme = get_current_theme();
    return apply_element_style(text, theme.subtitle_color, true);
}

Element ThemeManager::style_header3(const std::string& text) const {
    const auto& theme = get_current_theme();
    return apply_element_style(text, theme.accent_color, true);
}

Element ThemeManager::style_bullet(const std::string& text) const {
    const auto& theme = get_current_theme();
    return apply_element_style(text, theme.text_color);
}

Element ThemeManager::style_code(const std::string& text) const {
    const auto& theme = get_current_theme();
    return apply_element_style(text, theme.code_color);
}

Element ThemeManager::style_shell_command(const std::string& text) const {
    // const auto& theme = get_current_theme();
    return apply_element_style(text, Color::Green, true);
}

Element ThemeManager::style_shell_output(const std::string& text, bool executed) const {
    // const auto& theme = get_current_theme();
    Color color = executed ? Color::Yellow : Color::GrayDark;
    return apply_element_style(text, color);
}

Element ThemeManager::style_accent(const std::string& text) const {
    const auto& theme = get_current_theme();
    return apply_element_style(text, theme.accent_color, true);
}

Element ThemeManager::create_background() const {
    const auto& theme = get_current_theme();
    // return ftxui::bgcolor(theme.bg_color);
    return text("") | ftxui::bgcolor(theme.bg_color);
}

Element ThemeManager::create_slide_container(Element content) const {
    const auto& theme = get_current_theme();
    return content | ftxui::bgcolor(theme.bg_color);
}

Element ThemeManager::style_status_bar(const std::string& text) const {
    const auto& theme = get_current_theme();
    return apply_element_style(text, theme.accent_color, true);
}

Element ThemeManager::style_progress_bar(float progress) const {
    const auto& theme = get_current_theme();
    int width = 40;
    int filled = static_cast<int>(progress * width);
    
    std::string bar = "[";
    for (int i = 0; i < width; ++i) {
        bar += (i < filled) ? "#" : " ";
    }
    bar += "]";
    
    return apply_element_style(bar, theme.accent_color);
}

Element ThemeManager::style_help_text(const std::string& text) const {
    const auto& theme = get_current_theme();
    return apply_element_style(text, theme.text_color);
}

Color ThemeManager::get_background_color() const {
    return get_current_theme().bg_color;
}

Color ThemeManager::get_text_color() const {
    return get_current_theme().text_color;
}

Color ThemeManager::get_accent_color() const {
    return get_current_theme().accent_color;
}

} // namespace mdslides
