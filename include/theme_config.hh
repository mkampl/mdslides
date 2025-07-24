#pragma once

#include <vector>
#include <string>

// Define color constants when not using ncurses
#include <ftxui/screen/color.hpp>
// FTXUI color mapping
#define COLOR_BLACK   ftxui::Color::Black
#define COLOR_RED     ftxui::Color::Red
#define COLOR_GREEN   ftxui::Color::Green
#define COLOR_YELLOW  ftxui::Color::Yellow
#define COLOR_BLUE    ftxui::Color::Blue
#define COLOR_MAGENTA ftxui::Color::Magenta
#define COLOR_CYAN    ftxui::Color::Cyan
#define COLOR_WHITE   ftxui::Color::White

enum class Theme
{
    DARK,
    LIGHT,
    MATRIX,
    RETRO
};

struct ThemeConfig
{
    ftxui::Color bg_color, title_color, subtitle_color, text_color, accent_color, code_color;
    const char *name;
};

class ThemeManager
{
public:
    ThemeManager();
    void setup_theme(Theme theme);
    void cycle_theme();
    Theme get_current_theme() const;
    const char *get_current_theme_name() const;

private:
    std::vector<ThemeConfig> themes;
    Theme current_theme;
};