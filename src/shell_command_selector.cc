#include "shell_command_selector.hh"
#include "ncurses_renderer.hh"
#include <ncurses.h>
#include <algorithm>

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

    if (selected_index < (int)shell_commands.size() - 1)
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
        selected_index < 0 || selected_index >= (int)shell_commands.size())
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
    if (selected_index >= 0 && selected_index < (int)shell_commands.size())
    {
        highlight_command(selected_index, true);
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
    int y = cmd->y;
    int x = cmd->x;

    if (highlight)
    {
        // Draw selection indicator
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(y, x - 2, "→"); // Arrow indicator
        attroff(COLOR_PAIR(1) | A_BOLD);

        // Highlight the command text with reverse video
        attron(COLOR_PAIR(6) | A_BOLD | A_REVERSE);
        mvprintw(y, x, "%s", cmd->content.c_str());
        attroff(COLOR_PAIR(6) | A_BOLD | A_REVERSE);

        // Draw end indicator
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(y, x + (int)cmd->content.length(), "←");
        attroff(COLOR_PAIR(1) | A_BOLD);
    }
    else
    {
        // Clear selection indicators
        mvprintw(y, x - 2, " ");
        mvprintw(y, x + (int)cmd->content.length(), " ");

        // Restore normal command appearance
        attron(COLOR_PAIR(6));
        mvprintw(y, x, "%s", cmd->content.c_str());
        attroff(COLOR_PAIR(6));
    }

    refresh();
}