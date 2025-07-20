// Fix in shell_command_selector.cc

#include "shell_command_selector.hh"
#include "renderer_interface.hh"
#include <algorithm>

#ifndef USE_FTXUI_RENDERER
#include <ncurses.h>
#endif

ShellCommandSelector::ShellCommandSelector()
    : selected_index(0), selection_mode(false), renderer(nullptr)
{
}

void ShellCommandSelector::set_renderer(ISlideRenderer *r)
{
    renderer = r;
}

void ShellCommandSelector::enter_selection_mode(std::vector<SlideElement> &slide_elements)
{
    shell_commands.clear();
    selected_index = 0;
    selection_mode = true;

    // Find all shell commands on current slide
    for (auto &element : slide_elements)
    {
        if (element.type == ElementType::SHELL_COMMAND)
        {
            shell_commands.push_back(&element);
        }
    }

    if (shell_commands.empty())
    {
        selection_mode = false;
        return;
    }

    // Highlight first command
    update_selection_display();
}

void ShellCommandSelector::exit_selection_mode()
{
    if (!selection_mode)
        return;

    selection_mode = false;
    clear_all_highlights();
    
    // Force a complete redraw
    if (renderer) {
        renderer->refresh_display();
    }
}

bool ShellCommandSelector::is_active() const
{
    return selection_mode;
}

bool ShellCommandSelector::navigate_up()
{
    if (!selection_mode || shell_commands.empty())
        return false;

    if (selected_index > 0)
    {
        selected_index--;
        update_selection_display();
        return true;
    }
    return false;
}

bool ShellCommandSelector::navigate_down()
{
    if (!selection_mode || shell_commands.empty())
        return false;

    if (selected_index < shell_commands.size() - 1)
    {
        selected_index++;
        update_selection_display();
        return true;
    }
    return false;
}

SlideElement *ShellCommandSelector::get_selected_command()
{
    if (!selection_mode || shell_commands.empty() ||
        selected_index >= shell_commands.size())
    {
        return nullptr;
    }
    return shell_commands[selected_index];
}

int ShellCommandSelector::get_command_count() const
{
    return shell_commands.size();
}

int ShellCommandSelector::get_selected_index() const
{
    return selected_index;
}

void ShellCommandSelector::update_selection_display()
{
    clear_all_highlights();
    if (selected_index < shell_commands.size())
    {
        highlight_command(selected_index, true);
    }
    
#ifndef USE_FTXUI_RENDERER
    // For ncurses: Force immediate redraw of the affected lines
    redraw_shell_commands();
#endif
    
    if (renderer) {
        renderer->refresh_display();
    }
}

void ShellCommandSelector::clear_all_highlights()
{
    for (size_t i = 0; i < shell_commands.size(); ++i)
    {
        highlight_command(i, false);
    }
}

void ShellCommandSelector::highlight_command(int index, bool highlight)
{
    if (index < 0 || index >= (int)shell_commands.size())
        return;

    SlideElement *cmd = shell_commands[index];
    
    if (highlight)
    {
        // Add selection indicators if not already present
        if (cmd->content.find("→ ") != 0) {
            cmd->original_content = cmd->content; // Store original
            cmd->content = "→ " + cmd->content + " ←";
        }
    }
    else
    {
        // Restore original content if we have it
        if (!cmd->original_content.empty()) {
            cmd->content = cmd->original_content;
            cmd->original_content.clear();
        } else {
            // Fallback: remove indicators manually
            if (cmd->content.find("→ ") == 0 && cmd->content.find(" ←") == cmd->content.length() - 2) {
                cmd->content = cmd->content.substr(2, cmd->content.length() - 4);
            }
        }
    }
}

#ifndef USE_FTXUI_RENDERER
void ShellCommandSelector::redraw_shell_commands()
{
    // Redraw only the shell command lines to avoid flicker
    for (size_t i = 0; i < shell_commands.size(); ++i)
    {
        SlideElement *cmd = shell_commands[i];
        
        // Clear the line first
        int line_width = 120; // Assume reasonable max width
        mvprintw(cmd->y, 0, "%*s", line_width, "");
        
        // Determine colors and attributes
        int attrs = COLOR_PAIR(cmd->color_pair);
        if (cmd->is_bold)
            attrs |= A_BOLD;
        
        // Add highlight background for selected command
        if (i == selected_index && selection_mode) {
            attrs |= A_REVERSE; // Add reverse video for selection
        }
        
        // Draw the command with proper attributes
        attron(attrs);
        mvprintw(cmd->y, cmd->x, "%s", cmd->content.c_str());
        attroff(attrs);
    }
    
    // Force screen update
    refresh();
}
#endif