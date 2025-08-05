#pragma once

#include "slide_element.hh"
#include "theme_config.hh"
#include <chrono>

#include <ftxui/component/component.hpp>

enum class AppState
{
    MAIN_VIEW,
    HELP_VIEW,
    GOTO_DIALOG,
    SHELL_EXECUTION,
    SHELL_POPUP
};

struct ThemeColors
{
    ftxui::Color header_color;
    ftxui::Color text_color;
    ftxui::Color h1_color;
    ftxui::Color h2_color;
    ftxui::Color h3_color;
    ftxui::Color code_color;
    ftxui::Color shell_color;
    ftxui::Color separator_color;
    ftxui::Color footer_color;
    ftxui::Color progress_color;
    ftxui::Color background_color;
};

struct PresentationState
{
    SlideCollection slides;
    int current_slide = 0;
    bool show_timer = false;
    bool animations_enabled = true;
    bool utf8_supported = true;
    std::chrono::steady_clock::time_point start_time;
    Theme current_theme = Theme::DARK;

    // Animation state
    bool slide_changed = false;
    std::chrono::steady_clock::time_point slide_start_time;
    std::vector<bool> element_visible; // Track which elements are visible
    int animation_step = 0;

    // Shell command selection state
    bool shell_selection_active = false;
    int selected_shell_command = 0;
    std::vector<int> shell_command_indices; // Indices of shell commands on current slide

    // UI state
    AppState app_state = AppState::MAIN_VIEW;
    std::string goto_input;
    std::string status_message;
    std::string pending_shell_command;
    bool shell_confirmation_visible = false;

    // Helper methods
    const std::vector<SlideElement> &get_current_slide() const;

    int get_elapsed_seconds() const;
    void next_slide();
    void prev_slide();
    void goto_slide(int slide_num);
    void start_slide_animation();
    void update_shell_commands();
    void start_shell_selection();
    void exit_shell_selection();
    bool navigate_shell_up();
    bool navigate_shell_down();
    std::string get_selected_shell_command() const;
    int get_selected_shell_index() const;
    void update_animation();
    bool is_element_visible(int index) const;
    void cycle_theme();
};