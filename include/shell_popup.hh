#pragma once

#include "slide_element.hh"
#include <vector>
#include <string>

class ShellPopup
{
private:
    int popup_width, popup_height;
    int popup_x, popup_y;
    int scroll_offset;
    std::vector<std::string> output_lines;
    std::string command;
    bool is_running;

public:
    ShellPopup(int screen_width, int screen_height);

    void show(const std::string &cmd);

private:
    void draw_popup_frame();
    void execute_command();
    void display_output();
    void handle_input();
    void clear_popup_area();
    std::string execute_shell_command(const std::string &command);
};