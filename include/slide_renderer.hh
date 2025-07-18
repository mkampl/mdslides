#pragma once

#include "slide_element.hh"
#include "theme_config.hh"
#include "markdown_parser.hh"
#include "renderer_interface.hh"
#include "shell_command_selector.hh"
#include <chrono>
#include <string>
#include <memory>

class MarkdownSlideRenderer
{
public:
    MarkdownSlideRenderer();
    void load_slides(const std::string &filename);
    void run();

private:
    ShellCommandSelector shell_selector;

    //methods for shell command handling
    void check_for_shell_commands();
    void show_shell_command_hint();
    void start_shell_command_selection();
    bool handle_shell_selection_input(int ch);
    void execute_selected_shell_command();

    std::string execute_shell_command(const std::string &command);

    // Navigation and UI
    void goto_slide();
    void render_current_slide(bool animated);
    void get_timer_values(int &minutes, int &seconds);
    std::string get_current_theme_name();

    // Member variables
    SlideCollection slides;
    MarkdownParser parser;
    std::unique_ptr<ISlideRenderer> renderer;
    int current_slide;
    bool show_timer;
    std::chrono::steady_clock::time_point start_time;
    bool utf8_supported;
    Theme current_theme;
};