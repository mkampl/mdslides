#include "shell_popup.hh"
#include <sstream>
#include <algorithm>
#include <cstdio>
#include <memory>
#include <array>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <thread>
#include <atomic>
#include <chrono>


ShellPopup::ShellPopup(int screen_width, int screen_height)
    : renderer(nullptr)
{
    popup_width = std::min(screen_width - 4, 120);  // Max 120 chars wide
    popup_height = std::min(screen_height - 4, 30); // Max 30 lines high
    popup_x = (screen_width - popup_width) / 2;
    popup_y = (screen_height - popup_height) / 2;
    scroll_offset = 0;
    is_running = false;
}

void ShellPopup::set_renderer(ISlideRenderer *r)
{
    renderer = r;
}

void ShellPopup::show(const std::string &cmd)
{
    command = cmd;
    show_ftxui_popup();
}

void ShellPopup::show_ftxui_popup()
{
    using namespace ftxui;

    // Reset state
    output_lines.clear();
    scroll_offset = 0;
    is_running = true;
    command_executed = false;

    auto screen = ScreenInteractive::Fullscreen();

    // Execute command in background thread
    std::atomic<bool> execution_complete{false};
    std::thread execution_thread([&]()
                                 {
        std::string output = execute_shell_command(command);
        
        // Split into lines and handle line wrapping
        std::istringstream iss(output);
        std::string line;
        output_lines.clear();
        
        while (std::getline(iss, line)) {
            // Handle long lines - split them if necessary
            if (static_cast<int>(line.length()) > popup_width - 6) {
                size_t pos = 0;
                while (pos < line.length()) {
                    output_lines.push_back(line.substr(pos, popup_width - 6));
                    pos += popup_width - 6;
                }
            } else {
                output_lines.push_back(line);
            }
        }
        
        if (output_lines.empty()) {
            output_lines.push_back("[No output]");
        }
        
        command_executed = true;
        execution_complete = true;
        
        // Force screen refresh
        screen.PostEvent(Event::Custom); });

    // Create the popup component
    auto popup_component = Renderer([&]
                                    { return render_popup_content(execution_complete.load()); });

    // Handle input events
    popup_component = CatchEvent(popup_component, [&](Event event)
                                 {
        if (event == Event::Escape || (event.is_character() && (event.character()[0] == 'q' || event.character()[0] == 'Q'))) {
            is_running = false;
            screen.ExitLoopClosure()();
            return true;
        }
        
        if (!execution_complete.load()) {
            return false; // Ignore other input while executing
        }
        
        int display_lines = popup_height - 6;
        
        if (event == Event::ArrowUp) {
            if (scroll_offset > 0) {
                scroll_offset--;
                return true;
            }
        } else if (event == Event::ArrowDown) {
            if (scroll_offset + display_lines < static_cast<int>(output_lines.size())) {
                scroll_offset++;
                return true;
            }
        } else if (event == Event::PageUp) {
            scroll_offset = std::max(0, scroll_offset - display_lines);
            return true;
        } else if (event == Event::PageDown) {
            scroll_offset = std::min(static_cast<int>(output_lines.size()) - display_lines, scroll_offset + display_lines);
            if (scroll_offset < 0) scroll_offset = 0;
            return true;
        } else if (event == Event::Home) {
            scroll_offset = 0;
            return true;
        } else if (event == Event::End) {
            scroll_offset = std::max(0, static_cast<int>(output_lines.size()) - display_lines);
            return true;
        }
        
        return false; });

    // Start the popup loop
    screen.Loop(popup_component);

    // Cleanup
    if (execution_thread.joinable())
    {
        execution_thread.join();
    }
}

void ShellPopup::set_background_provider(std::function<ftxui::Element()> provider){
    background_provider_ = provider;
}

ftxui::Element ShellPopup::render_popup_content(bool execution_complete)
{
    using namespace ftxui;

    std::vector<Element> content;

    // Title
    content.push_back(text("Shell Command Execution") | bold | center | color(Color::Cyan));
    content.push_back(separator());

    // Command display
    std::string display_cmd = "$ " + command;
    if (static_cast<int>(display_cmd.length()) > popup_width - 4)
    {
        display_cmd = display_cmd.substr(0, popup_width - 7) + "...";
    }
    content.push_back(text(display_cmd) | bold | color(Color::Yellow));
    content.push_back(separator());

    if (!execution_complete)
    {
        // Show execution in progress
        content.push_back(text(""));
        content.push_back(text("Executing...") | bold | color(Color::Green) | center);
        content.push_back(text(""));

        // Add some animated dots
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
        int dots = (elapsed.count() / 300) % 4; // Change every 300ms
        std::string dot_animation = std::string(dots, '.');
        content.push_back(text(dot_animation) | center | color(Color::Green));
    }
    else
    {
        // Show output
        std::vector<Element> output_content;

        int display_lines = popup_height - 8; // Reserve space for header, footer, borders
        int start_line = scroll_offset;
        int end_line = std::min(start_line + display_lines, static_cast<int>(output_lines.size()));

        // Add output lines
        for (int i = start_line; i < end_line; ++i)
        {
            output_content.push_back(text(output_lines[i]) | color(Color::White));
        }

        // Fill remaining space if needed
        while (static_cast<int>(output_content.size()) < display_lines)
        {
            output_content.push_back(text(""));
        }

        content.push_back(vbox(output_content) | flex);

        // Add scroll indicator if needed
        if (static_cast<int>(output_lines.size()) > display_lines)
        {
            std::string scroll_info = "Lines " + std::to_string(start_line + 1) +
                                      "-" + std::to_string(end_line) +
                                      " of " + std::to_string(output_lines.size());

            Elements scroll_indicators;
            if (start_line > 0)
            {
                scroll_indicators.push_back(text("↑") | color(Color::Yellow));
            }
            else
            {
                scroll_indicators.push_back(text(" "));
            }

            scroll_indicators.push_back(text(scroll_info) | center);

            if (end_line < static_cast<int>(output_lines.size()))
            {
                scroll_indicators.push_back(text("↓") | color(Color::Yellow));
            }
            else
            {
                scroll_indicators.push_back(text(" "));
            }

            content.push_back(separator());
            content.push_back(hbox(scroll_indicators) | center);
        }
    }

    // Footer with controls
    content.push_back(separator());
    if (execution_complete)
    {
        content.push_back(text("ESC: Close | ↑↓: Scroll | PgUp/PgDn: Page | Home/End: Jump") | center | color(Color::Cyan));
    }
    else
    {
        content.push_back(text("ESC: Close (will terminate command)") | center | color(Color::Red));
    }

    Element popup = vbox(content) | 
                   border | 
                   bgcolor(Color::Black) |
                   size(WIDTH, EQUAL, popup_width) | 
                   size(HEIGHT, EQUAL, popup_height);
    
    if (background_provider_) {
        Element background = background_provider_();
        
        // Erstelle Popup-Layer mit transparenten Bereichen
        Element popup_layer = vbox({
            filler(),
            hbox({
                filler(),
                popup,  // Popup OHNE dim
                filler()
            }),
            filler()
        });
        
        // Kombiniere: Gedimmter Background + heller Popup
        return dbox({
            background,
            popup_layer
        });
    }
    
    // Fallback
    return vbox({
        filler(),
        hbox({
            filler(),
            popup,
            filler()
        }),
        filler()
    }) | bgcolor(Color::GrayDark);
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