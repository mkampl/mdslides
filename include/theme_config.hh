#pragma once

#include <vector>
#include <string>

// Define color constants when not using ncurses
#ifdef USE_FTXUI_RENDERER
// FTXUI color mapping - these are just indexes for our theme system
#define COLOR_BLACK   0
#define COLOR_RED     1
#define COLOR_GREEN   2
#define COLOR_YELLOW  3
#define COLOR_BLUE    4
#define COLOR_MAGENTA 5
#define COLOR_CYAN    6
#define COLOR_WHITE   7
#endif

enum class Theme
{
    DARK,
    LIGHT,
    MATRIX,
    RETRO
};

struct ThemeConfig
{
    int bg_color, title_color, subtitle_color, text_color, accent_color, code_color;
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