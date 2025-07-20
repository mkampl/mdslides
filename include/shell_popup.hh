#pragma once

#include <string>
#include <vector>

#ifdef USE_FTXUI_RENDERER
#include <ftxui/dom/elements.hpp>
#include <atomic>
#endif

// Forward declaration
class ISlideRenderer;

class ShellPopup
{
public:
    ShellPopup(int screen_width, int screen_height);
    
    void set_renderer(ISlideRenderer* renderer);
    void show(const std::string& command);
    bool get_is_running(){return is_running;};

private:
    ISlideRenderer* renderer;
    std::string command;
    std::vector<std::string> output_lines;
    
    int popup_width;
    int popup_height;
    int popup_x;
    int popup_y;
    int scroll_offset;
    bool is_running;
    
#ifdef USE_FTXUI_RENDERER
    bool command_executed = false;
    
    void show_ftxui_popup();
    ftxui::Element render_popup_content(bool execution_complete);
#else
    // NCurses-specific methods
    void draw_popup_frame();
    void execute_command();
    void display_output();
    void handle_input();
    void clear_popup_area();
#endif

    std::string execute_shell_command(const std::string& command);
};