#pragma once

#include <string>
#include <vector>

#include <ftxui/dom/elements.hpp>
#include <atomic>

// Forward declaration
class ISlideRenderer;

class ShellPopup
{
public:
    ShellPopup(int screen_width, int screen_height);
    
    void set_renderer(ISlideRenderer* renderer);
    void show(const std::string& command);
    bool get_is_running(){return is_running;};
    void set_background_provider(std::function<ftxui::Element()> provider);

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
    
    bool command_executed = false;
    std::function<ftxui::Element()> background_provider_;
    
    void show_ftxui_popup();
    ftxui::Element render_popup_content(bool execution_complete);

    std::string execute_shell_command(const std::string& command);
};