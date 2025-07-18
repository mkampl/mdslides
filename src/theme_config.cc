#include "theme_config.hh"
#include <ncurses.h>

ThemeManager::ThemeManager() : current_theme(Theme::DARK)
{
    themes = {
        {COLOR_BLACK, COLOR_CYAN, COLOR_YELLOW, COLOR_WHITE, COLOR_GREEN, COLOR_MAGENTA, "Dark"},
        {COLOR_WHITE, COLOR_BLUE, COLOR_RED, COLOR_BLACK, COLOR_GREEN, COLOR_MAGENTA, "Light"},
        {COLOR_BLACK, COLOR_GREEN, COLOR_GREEN, COLOR_GREEN, COLOR_WHITE, COLOR_GREEN, "Matrix"},
        {COLOR_BLACK, COLOR_YELLOW, COLOR_CYAN, COLOR_WHITE, COLOR_MAGENTA, COLOR_RED, "Retro"}};
}

void ThemeManager::setup_theme(Theme theme)
{
    current_theme = theme;
    const auto &theme_config = themes[static_cast<int>(theme)];

    init_pair(1, theme_config.title_color, theme_config.bg_color);
    init_pair(2, theme_config.subtitle_color, theme_config.bg_color);
    init_pair(3, theme_config.text_color, theme_config.bg_color);
    init_pair(4, theme_config.accent_color, theme_config.bg_color);
    init_pair(5, theme_config.bg_color, theme_config.text_color);
    init_pair(6, theme_config.code_color, theme_config.bg_color);
    init_pair(7, COLOR_GREEN, theme_config.bg_color);
    init_pair(8, COLOR_YELLOW, theme_config.bg_color);
    init_pair(9, COLOR_RED, theme_config.bg_color);
    init_pair(0, theme_config.text_color, theme_config.bg_color);

    refresh();
    // Set window background
    wbkgd(stdscr, ' ' | COLOR_PAIR(9));
    refresh();
}

void ThemeManager::cycle_theme()
{
    current_theme = static_cast<Theme>((static_cast<int>(current_theme) + 1) % themes.size());
    setup_theme(current_theme);
}

Theme ThemeManager::get_current_theme() const
{
    return current_theme;
}

const char *ThemeManager::get_current_theme_name() const
{
    return themes[static_cast<int>(current_theme)].name;
}
