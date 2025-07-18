#include "shell_popup.hh"
#include <ncurses.h>
#include <sstream>
#include <algorithm>
#include <cstdio>
#include <memory>
#include <array>

ShellPopup::ShellPopup(int screen_width, int screen_height)
{
    popup_width = std::min(screen_width - 4, 120);  // Max 120 chars wide
    popup_height = std::min(screen_height - 4, 30); // Max 30 lines high
    popup_x = (screen_width - popup_width) / 2;
    popup_y = (screen_height - popup_height) / 2;
    scroll_offset = 0;
    is_running = false;
}

void ShellPopup::show(const std::string &cmd)
{
    command = cmd;
    draw_popup_frame();
    execute_command();
    handle_input();
    clear_popup_area();
}

void ShellPopup::draw_popup_frame()
{
    // Draw popup background
    attron(COLOR_PAIR(0));
    for (int i = 0; i < popup_height; ++i)
    {
        for (int j = 0; j < popup_width; ++j)
        {
            mvaddch(popup_y + i, popup_x + j, ' ');
        }
    }
    attroff(COLOR_PAIR(0));

    // Draw popup border
    attron(COLOR_PAIR(1) | A_BOLD);

    // Top border
    mvhline(popup_y, popup_x, '-', popup_width);
    mvprintw(popup_y, popup_x + 2, "[ Shell Command Execution ]");

    // Side borders
    for (int i = 1; i < popup_height - 1; ++i)
    {
        mvprintw(popup_y + i, popup_x, "|");
        mvprintw(popup_y + i, popup_x + popup_width - 1, "|");
    }

    // Bottom border
    mvhline(popup_y + popup_height - 1, popup_x, '-', popup_width);
    mvprintw(popup_y + popup_height - 1, popup_x + 2, "[ ESC: Close | ↑↓: Scroll | PgUp/PgDn: Page ]");

    attroff(COLOR_PAIR(1) | A_BOLD);

    // Show command
    attron(COLOR_PAIR(7) | A_BOLD);
    std::string display_cmd = "$ " + command;
    if ((int)display_cmd.length() > popup_width - 4)
    {
        display_cmd = display_cmd.substr(0, popup_width - 7) + "...";
    }
    mvprintw(popup_y + 2, popup_x + 2, "%s", display_cmd.c_str());
    attroff(COLOR_PAIR(7) | A_BOLD);

    // Draw separator line
    attron(COLOR_PAIR(4));
    mvhline(popup_y + 3, popup_x + 1, '-', popup_width - 2);
    attroff(COLOR_PAIR(4));

    refresh();
}

void ShellPopup::execute_command()
{
    // Show "Executing..." message
    attron(COLOR_PAIR(4) | A_BOLD);
    mvprintw(popup_y + 5, popup_x + 2, "Executing...");
    attroff(COLOR_PAIR(4) | A_BOLD);
    refresh();

    // Execute command
    std::string output = execute_shell_command(command);

    // Split into lines
    std::istringstream iss(output);
    std::string line;
    output_lines.clear();
    while (std::getline(iss, line))
    {
        // Handle long lines - split them if necessary
        if ((int)line.length() > popup_width - 6)
        {
            size_t pos = 0;
            while (pos < line.length())
            {
                output_lines.push_back(line.substr(pos, popup_width - 6));
                pos += popup_width - 6;
            }
        }
        else
        {
            output_lines.push_back(line);
        }
    }

    if (output_lines.empty())
    {
        output_lines.push_back("[No output]");
    }

    scroll_offset = 0;
    display_output();
}

void ShellPopup::display_output()
{
    // Clear output area
    attron(COLOR_PAIR(0));
    for (int i = 4; i < popup_height - 2; ++i)
    {
        for (int j = popup_x + 1; j < popup_x + popup_width - 1; ++j)
        {
            mvaddch(popup_y + i, j, ' ');
        }
    }
    attroff(COLOR_PAIR(0));

    // Calculate display area
    int display_lines = popup_height - 6; // Available lines for output
    int start_line = scroll_offset;
    int end_line = std::min(start_line + display_lines, (int)output_lines.size());

    // Display output lines
    attron(COLOR_PAIR(8));
    for (int i = start_line; i < end_line; ++i)
    {
        int display_row = popup_y + 4 + (i - start_line);
        mvprintw(display_row, popup_x + 2, "%s", output_lines[i].c_str());
    }
    attroff(COLOR_PAIR(8));

    // Show scroll indicator if needed
    if ((int)output_lines.size() > display_lines)
    {
        attron(COLOR_PAIR(4) | A_BOLD);

        // Calculate scroll info
        int displayed_end = std::min(start_line + display_lines, (int)output_lines.size());
        std::string scroll_info = "Lines " + std::to_string(start_line + 1) +
                                  "-" + std::to_string(displayed_end) +
                                  " of " + std::to_string(output_lines.size());

        // Show scroll info in top right of popup
        mvprintw(popup_y + 1, popup_x + popup_width - scroll_info.length() - 3,
                 "%s", scroll_info.c_str());

        // Show scroll arrows if applicable
        if (start_line > 0)
        {
            mvprintw(popup_y + 4, popup_x + popup_width - 3, "↑");
        }
        if (displayed_end < (int)output_lines.size())
        {
            mvprintw(popup_y + popup_height - 3, popup_x + popup_width - 3, "↓");
        }

        attroff(COLOR_PAIR(4) | A_BOLD);
    }

    refresh();
}

void ShellPopup::handle_input()
{
    int display_lines = popup_height - 6;
    int ch;

    while ((ch = getch()) != 27)
    { // ESC to close
        switch (ch)
        {
        case KEY_UP:
            if (scroll_offset > 0)
            {
                scroll_offset--;
                display_output();
            }
            break;

        case KEY_DOWN:
            if (scroll_offset + display_lines < (int)output_lines.size())
            {
                scroll_offset++;
                display_output();
            }
            break;

        case KEY_PPAGE: // Page Up
            scroll_offset = std::max(0, scroll_offset - display_lines);
            display_output();
            break;

        case KEY_NPAGE: // Page Down
            scroll_offset = std::min((int)output_lines.size() - display_lines,
                                     scroll_offset + display_lines);
            if (scroll_offset < 0)
                scroll_offset = 0;
            display_output();
            break;

        case KEY_HOME:
            scroll_offset = 0;
            display_output();
            break;

        case KEY_END:
            scroll_offset = std::max(0, (int)output_lines.size() - display_lines);
            display_output();
            break;

        case 'q':
        case 'Q':
            return; // Also allow 'q' to quit
        }
    }
}

void ShellPopup::clear_popup_area()
{
    // Clear the entire popup area
    attron(COLOR_PAIR(0));
    for (int i = 0; i < popup_height; ++i)
    {
        for (int j = 0; j < popup_width; ++j)
        {
            mvaddch(popup_y + i, popup_x + j, ' ');
        }
    }
    attroff(COLOR_PAIR(0));
    refresh();
}

std::string ShellPopup::execute_shell_command(const std::string &command)
{
    std::array<char, 128> buffer;
    std::string result;

    FILE *pipe_raw = popen(command.c_str(), "r");
    if (!pipe_raw)
        return "Error: Could not execute command";

    std::unique_ptr<FILE, int (*)(FILE *)> pipe(pipe_raw, pclose);

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }

    // Remove trailing newline if present
    if (!result.empty() && result.back() == '\n')
    {
        result.pop_back();
    }

    return result;
}