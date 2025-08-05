#include "theme_config.hh"

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