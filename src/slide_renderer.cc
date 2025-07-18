// Replace the entire slide_renderer.cc content with this cleaned version:

#include "slide_renderer.hh"
#include "ncurses_renderer.hh"
#include "shell_popup.hh"
#include <ncurses.h>
#include <algorithm>
#include <thread>
#include <chrono>
#include <memory>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <locale.h>
#include <sstream>

MarkdownSlideRenderer::MarkdownSlideRenderer()
    : current_slide(0), show_timer(false), utf8_supported(false), current_theme(Theme::DARK)
{

    // Create ncurses renderer
    renderer = std::make_unique<NCursesRenderer>();

    // Set up shell selector
    shell_selector.set_renderer(renderer.get());

    // Set up locale and detect UTF-8 support
    setlocale(LC_ALL, "");

    // Check environment variables for UTF-8 support
    const char *lang = getenv("LANG");
    const char *lc_all = getenv("LC_ALL");
    const char *lc_ctype = getenv("LC_CTYPE");

    if ((lang && (strstr(lang, "UTF-8") || strstr(lang, "utf8"))) ||
        (lc_all && (strstr(lc_all, "UTF-8") || strstr(lc_all, "utf8"))) ||
        (lc_ctype && (strstr(lc_ctype, "UTF-8") || strstr(lc_ctype, "utf8"))))
    {
        utf8_supported = true;
    }

    // Also check if the current locale supports UTF-8
    char *current_locale = setlocale(LC_CTYPE, nullptr);
    if (current_locale && (strstr(current_locale, "UTF-8") || strstr(current_locale, "utf8")))
    {
        utf8_supported = true;
    }

    // Set UTF-8 support in both parser and renderer
    parser.set_utf8_support(utf8_supported);
    if (auto ncurses_renderer = dynamic_cast<NCursesRenderer *>(renderer.get()))
    {
        ncurses_renderer->set_utf8_support(utf8_supported);
    }
}

void MarkdownSlideRenderer::goto_slide()
{
    renderer->clear_screen();
    renderer->show_message("Go to slide (1-" + std::to_string(slides.get_slide_count()) + "): ",
                           renderer->get_screen_height() / 2);
    renderer->refresh_display();

    renderer->enable_echo();
    char input[10];
    renderer->get_string(input, sizeof(input));
    renderer->disable_echo();

    int slide_num = std::atoi(input);
    if (slide_num >= 1 && slide_num <= slides.get_slide_count())
    {
        current_slide = slide_num - 1;
    }

    render_current_slide(false);
}

void MarkdownSlideRenderer::get_timer_values(int &minutes, int &seconds)
{
    if (show_timer)
    {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - start_time);
        minutes = duration.count() / 60;
        seconds = duration.count() % 60;
    }
    else
    {
        minutes = seconds = 0;
    }
}

std::string MarkdownSlideRenderer::get_current_theme_name()
{
    const char *theme_names[] = {"Dark", "Light", "Matrix", "Retro"};
    return theme_names[static_cast<int>(current_theme)];
}

void MarkdownSlideRenderer::render_current_slide(bool animated)
{
    int minutes, seconds;
    get_timer_values(minutes, seconds);

    renderer->draw_header(current_slide, slides.get_slide_count(), get_current_theme_name(),
                          show_timer, minutes, seconds, utf8_supported);
    renderer->draw_footer();
    renderer->draw_progress_bar(current_slide, slides.get_slide_count());
    renderer->refresh_display();

    renderer->render_slide(slides.get_slide(current_slide), animated);
    renderer->refresh_display();
}

void MarkdownSlideRenderer::load_slides(const std::string &filename)
{
    parser.load_slides(filename, slides);
}

void MarkdownSlideRenderer::run()
{
    if (slides.is_empty())
    {
        printf("No slides loaded!\n");
        return;
    }

    renderer->initialize();
    renderer->apply_theme(current_theme);
    start_time = std::chrono::steady_clock::now();

    bool use_animations = true;

    // Initial render
    render_current_slide(use_animations);
    check_for_shell_commands();

    int ch;
    while ((ch = renderer->get_input()) != 'q')
    {
        // Handle shell command selection first
        if (shell_selector.is_active())
        {
            if (handle_shell_selection_input(ch))
            {
                continue; // Input was handled by selection system
            }
        }

        switch (ch)
        {
        case KEY_RIGHT:
        case ' ':
        case 'l':
            shell_selector.exit_selection_mode();
            renderer->clear_message_area();
            if (current_slide < slides.get_slide_count() - 1)
            {
                current_slide++;
                render_current_slide(use_animations);
                check_for_shell_commands();
            }
            break;

        case KEY_LEFT:
        case KEY_BACKSPACE:
            shell_selector.exit_selection_mode();
            renderer->clear_message_area();
            if (current_slide > 0)
            {
                current_slide--;
                render_current_slide(use_animations);
                check_for_shell_commands();
            }
            break;

        case '\n':
        case '\r':
        case KEY_ENTER:
            if (shell_selector.is_active())
            {
                execute_selected_shell_command();
            }
            else
            {
                start_shell_command_selection();
            }
            break;

        case KEY_HOME:
        case '0':
            shell_selector.exit_selection_mode();
            renderer->clear_message_area();
            current_slide = 0;
            render_current_slide(false);
            check_for_shell_commands();
            break;

        case KEY_END:
        case '$':
            shell_selector.exit_selection_mode();
            renderer->clear_message_area();
            current_slide = slides.get_slide_count() - 1;
            render_current_slide(false);
            check_for_shell_commands();
            break;

        case 'g':
            shell_selector.exit_selection_mode();
            renderer->clear_message_area();
            goto_slide();
            check_for_shell_commands();
            break;

        case 't':
            current_theme = static_cast<Theme>((static_cast<int>(current_theme) + 1) % 4);
            renderer->apply_theme(current_theme);
            render_current_slide(false);
            if (!shell_selector.is_active())
            {
                check_for_shell_commands();
            }
            break;

        case 'a':
            use_animations = !use_animations;
            render_current_slide(false);
            if (!shell_selector.is_active())
            {
                check_for_shell_commands();
            }
            break;

        case 'T':
            show_timer = !show_timer;
            render_current_slide(false);
            if (!shell_selector.is_active())
            {
                check_for_shell_commands();
            }
            break;

        case 'r':
            render_current_slide(false);
            if (!shell_selector.is_active())
            {
                check_for_shell_commands();
            }
            break;

        case 'h':
        case '?':
            shell_selector.exit_selection_mode();
            renderer->clear_message_area();
            renderer->show_help(utf8_supported);
            renderer->get_input(); // Wait for key press
            render_current_slide(false);
            check_for_shell_commands();
            break;
        }
    }

    renderer->cleanup();
}

void MarkdownSlideRenderer::check_for_shell_commands()
{
    // Check if current slide has shell commands
    for (const auto &element : slides.get_slide(current_slide))
    {
        if (element.type == ElementType::SHELL_COMMAND)
        {
            show_shell_command_hint();
            return;
        }
    }
}

void MarkdownSlideRenderer::show_shell_command_hint()
{
    renderer->show_message("Shell commands detected! Press ENTER to select command", LINES - 5);
}

void MarkdownSlideRenderer::start_shell_command_selection()
{
    auto &current_slide_elements = slides.get_slide(current_slide);
    shell_selector.enter_selection_mode(current_slide_elements);

    if (shell_selector.is_active())
    {
        std::string msg = "Use ↑↓ to select command (" +
                          std::to_string(shell_selector.get_command_count()) +
                          " available), ENTER to execute, ESC to cancel";
        renderer->show_message(msg, LINES - 5);
    }
    else
    {
        renderer->show_message("No shell commands found on this slide", LINES - 5);
    }
}

bool MarkdownSlideRenderer::handle_shell_selection_input(int ch)
{
    switch (ch)
    {
    case KEY_UP:
        shell_selector.navigate_up();
        // Update status message with current selection
        {
            std::string msg = "Command " + std::to_string(shell_selector.get_selected_index() + 1) +
                              " of " + std::to_string(shell_selector.get_command_count()) +
                              " selected. ENTER to execute, ESC to cancel";
            renderer->show_message(msg, LINES - 5);
        }
        return true;

    case KEY_DOWN:
        shell_selector.navigate_down();
        // Update status message with current selection
        {
            std::string msg = "Command " + std::to_string(shell_selector.get_selected_index() + 1) +
                              " of " + std::to_string(shell_selector.get_command_count()) +
                              " selected. ENTER to execute, ESC to cancel";
            renderer->show_message(msg, LINES - 5);
        }
        return true;

    case 27: // ESC
        shell_selector.exit_selection_mode();
        renderer->clear_message_area();
        check_for_shell_commands(); // Show hint again
        return true;

    case '\n':
    case '\r':
    case KEY_ENTER:
        execute_selected_shell_command();
        return true;

    default:
        return false; // Let main input handler process this
    }
}

void MarkdownSlideRenderer::execute_selected_shell_command()
{
    SlideElement *selected = shell_selector.get_selected_command();
    if (selected)
    {
        shell_selector.exit_selection_mode();
        renderer->clear_message_area();

        // Create and show popup
        ShellPopup popup(renderer->get_screen_width(), renderer->get_screen_height());
        popup.show(selected->shell_command);

        // Refresh slide after popup closes
        render_current_slide(false);
        check_for_shell_commands();
    }
}