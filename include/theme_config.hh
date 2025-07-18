#pragma once

#include <vector>
#include <string>

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
