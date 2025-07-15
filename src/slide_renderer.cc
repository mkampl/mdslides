#include "slide_renderer.hh"
#include "ncurses_renderer.hh"
#include "ncurses.h"
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
    : current_slide(0), show_timer(false), utf8_supported(false),current_theme(Theme::DARK) {
    
    // Create ncurses renderer (later we can add conditional FTXUI creation here)
    renderer = std::make_unique<NCursesRenderer>();
    
    // Set up locale and detect UTF-8 support
    setlocale(LC_ALL, "");
    
    // Check environment variables for UTF-8 support
    const char* lang = getenv("LANG");
    const char* lc_all = getenv("LC_ALL");
    const char* lc_ctype = getenv("LC_CTYPE");
    
    if ((lang && (strstr(lang, "UTF-8") || strstr(lang, "utf8"))) ||
        (lc_all && (strstr(lc_all, "UTF-8") || strstr(lc_all, "utf8"))) ||
        (lc_ctype && (strstr(lc_ctype, "UTF-8") || strstr(lc_ctype, "utf8")))) {
        utf8_supported = true;
    }
    
    // Also check if the current locale supports UTF-8
    char* current_locale = setlocale(LC_CTYPE, nullptr);
    if (current_locale && (strstr(current_locale, "UTF-8") || strstr(current_locale, "utf8"))) {
        utf8_supported = true;
    }
    
    // Set UTF-8 support in both parser and renderer
    parser.set_utf8_support(utf8_supported);
    if (auto ncurses_renderer = dynamic_cast<NCursesRenderer*>(renderer.get())) {
        ncurses_renderer->set_utf8_support(utf8_supported);
    }
}

std::string MarkdownSlideRenderer::execute_shell_command(const std::string& command) {
    std::array<char, 128> buffer;
    std::string result;
    
    FILE* pipe_raw = popen(command.c_str(), "r");
    if (!pipe_raw) return "Error: Could not execute command";
    
    std::unique_ptr<FILE, int(*)(FILE*)> pipe(pipe_raw, pclose);
    
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    return result;
}

void MarkdownSlideRenderer::execute_shell_commands_on_slide() {
    bool has_shell_commands = false;
    for (const auto& e : slides.get_slide(current_slide)) {
        if (e.type == ElementType::SHELL_COMMAND) {
            has_shell_commands = true;
            break;
        }
    }
    
    if (!has_shell_commands) return;
    
    renderer->show_message("Shell commands detected! Press ENTER to execute, ESC to skip");
    
    int ch = renderer->get_input();
    renderer->clear_message_area();
    
    if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
        for (auto& element : slides.get_slide(current_slide)) {
            if (element.type == ElementType::SHELL_COMMAND && !element.executed) {
                execute_single_shell_command(element);
            }
        }
    }
    renderer->refresh_display();
}

void MarkdownSlideRenderer::execute_single_shell_command(SlideElement& shell_element) {
    if (shell_element.shell_command.empty()) return;
    
    renderer->show_message("Executing...", shell_element.y + 1);
    renderer->refresh_display();
    
    std::string output = execute_shell_command(shell_element.shell_command);
    shell_element.executed = true;
    
    // Split output into lines
    std::istringstream iss(output);
    std::string line;
    shell_element.shell_output_lines.clear();
    
    while (std::getline(iss, line)) {
        shell_element.shell_output_lines.push_back(line);
    }
    
    if (shell_element.shell_output_lines.empty()) {
        shell_element.shell_output_lines.push_back("[No output]");
    }
    
    renderer->update_shell_output(shell_element);
    renderer->sleep_ms(500);
}

void MarkdownSlideRenderer::scroll_shell_output(int direction) {
    for (auto& element : slides.get_slide(current_slide)) {
        if (element.type == ElementType::SHELL_COMMAND && element.executed) {
            int max_scroll = std::max(0, (int)element.shell_output_lines.size() - element.max_output_lines);
            
            if (direction > 0) {
                element.output_scroll_offset = std::min(element.output_scroll_offset + 1, max_scroll);
            } else {
                element.output_scroll_offset = std::max(element.output_scroll_offset - 1, 0);
            }
            
            renderer->update_shell_output(element);
        }
    }
}

void MarkdownSlideRenderer::goto_slide() {
    renderer->clear_screen();
    renderer->show_message("Go to slide (1-" + std::to_string(slides.get_slide_count()) + "): ", 
                          renderer->get_screen_height() / 2);
    renderer->refresh_display();
    
    renderer->enable_echo();
    char input[10];
    renderer->get_string(input, sizeof(input));
    renderer->disable_echo();
    
    int slide_num = std::atoi(input);
    if (slide_num >= 1 && slide_num <= slides.get_slide_count()) {
        current_slide = slide_num - 1;
    }
    
    render_current_slide(false);
}

void MarkdownSlideRenderer::render_current_slide(bool animated) {
    // Calculate timer values first
    int minutes = 0, seconds = 0;
    if (show_timer) {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - start_time);
        minutes = duration.count() / 60;
        seconds = duration.count() % 60;
    }
    
    // Draw header and footer BEFORE animations
    renderer->draw_header(current_slide, slides.get_slide_count(), "Dark", 
                         show_timer, minutes, seconds, utf8_supported);
    renderer->draw_footer();
    renderer->draw_progress_bar(current_slide, slides.get_slide_count());
    renderer->refresh_display();
    
    // THEN do the slide content with animations
    renderer->render_slide(slides.get_slide(current_slide), animated);
    renderer->refresh_display();
}

void MarkdownSlideRenderer::load_slides(const std::string& filename) {
    parser.load_slides(filename, slides);
}

void MarkdownSlideRenderer::run() {
    if (slides.is_empty()) {
        printf("No slides loaded!\n");
        return;
    }
    
    renderer->initialize();
    renderer->apply_theme(Theme::DARK);
    start_time = std::chrono::steady_clock::now();
    
    bool use_animations = true;
    
    // Initial render
    render_current_slide(use_animations);
    execute_shell_commands_on_slide();
    
    int ch;
    while ((ch = renderer->get_input()) != 'q' && ch != 27) {
        switch(ch) {
            case KEY_RIGHT: case ' ': case 'l':
                if (current_slide < slides.get_slide_count() - 1) {
                    current_slide++;
                    render_current_slide(use_animations);
                    execute_shell_commands_on_slide();
                }
                break;
                
            case KEY_LEFT: case KEY_BACKSPACE:
                if (current_slide > 0) {
                    current_slide--;
                    render_current_slide(use_animations);
                    execute_shell_commands_on_slide();
                }
                break;
                
            case '\n': case '\r': case KEY_ENTER:
                execute_shell_commands_on_slide();
                break;
                
            case KEY_HOME: case '0':
                current_slide = 0;
                render_current_slide(false);
                break;
                
            case KEY_END: case '$':
                current_slide = slides.get_slide_count() - 1;
                render_current_slide(false);
                break;
                
            case 'g':
                goto_slide();
                execute_shell_commands_on_slide();
                break;
                
            case 't':
                current_theme = static_cast<Theme>((static_cast<int>(current_theme) + 1) % 4);
                renderer->apply_theme(current_theme);
                render_current_slide(false);
                break;
                
            case 'a':
                use_animations = !use_animations;
                render_current_slide(false);
                break;
                
            case 'T':
                show_timer = !show_timer;
                render_current_slide(false);
                break;
                
            case 'r':
                render_current_slide(false);
                break;
                
            case 'u':
                scroll_shell_output(-1);
                break;
                
            case 'd':
                scroll_shell_output(1);
                break;
                
            case 'h': case '?':
                renderer->show_help(utf8_supported);
                renderer->get_input(); // Wait for key press
                render_current_slide(false);
                break;
        }
    }
    
    renderer->cleanup();
}