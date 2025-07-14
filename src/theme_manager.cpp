#include "theme_manager.hpp"
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

namespace mdslides {

ThemeManager::ThemeManager() : current_theme_(Theme::DARK) {
    initialize_themes();
}

void ThemeManager::initialize_themes() {
    themes_ = {
        // DARK theme
        {
            .bg_color = Color::Black,
            .title_color = Color::Cyan,
            .subtitle_color = Color::Yellow,
            .text_color = Color::White,
            .accent_color = Color::Green,
            .code_color = Color::Magenta,
            .name = "Dark"
        },
        // LIGHT theme
        {
            .bg_color = Color::White,
            .title_color = Color::Blue,
            .subtitle_color = Color::Red,
            .text_color = Color::Black,
            .accent_color = Color::Green,
            .code_color = Color::Magenta,
            .name = "Light"
        },
        // MATRIX theme
        {
            .bg_color = Color::Black,
            .title_color = Color::Green,
            .subtitle_color = Color::Green,
            .text_color = Color::Green,
            .accent_color = Color::White,
            .code_color = Color::Green,
            .name = "Matrix"
        },
        // RETRO theme
        {
            .bg_color = Color::Black,
            .title_color = Color::Yellow,
            .subtitle_color = Color::Cyan,
            .text_color = Color::White,
            .accent_color = Color::Magenta,
            .code_color = Color::Red,
            .name = "Retro"
        }
    };
}

const ThemeConfig& ThemeManager::get_current_theme() const {
    return themes_[static_cast<size_t>(current_theme_)];
}

const ThemeConfig& ThemeManager::get_theme(Theme theme) const {
    return themes_[static_cast<size_t>(theme)];
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
    const auto& theme = get_current_theme();
    return apply_element_style(text, Color::Green, true);
}

Element ThemeManager::style_shell_output(const std::string& text, bool executed) const {
    const auto& theme = get_current_theme();
    Color color = executed ? Color::Yellow : Color::GrayDark;
    return apply_element_style(text, color);
}

Element ThemeManager::style_accent(const std::string& text) const {
    const auto& theme = get_current_theme();
    return apply_element_style(text, theme.accent_color, true);
}

Element ThemeManager::create_background() const {
    const auto& theme = get_current_theme();
    return ftxui::bgcolor(theme.bg_color);
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
