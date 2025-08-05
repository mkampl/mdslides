#include "ftxui_renderer.hh"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <algorithm>
#include <thread>
#include <chrono>
#include <cstring>

using namespace ftxui;

FTXUIRenderer::FTXUIRenderer() 
    : current_state(AppState::MAIN_VIEW), app_running(false), 
      last_input(0), input_ready(false), screen_initialized(false)
{
    setup_theme_colors(Theme::DARK);
}

FTXUIRenderer::~FTXUIRenderer()
{
    cleanup();
}

void FTXUIRenderer::initialize()
{
    theme_manager.setup_theme(Theme::DARK);
    setup_theme_colors(Theme::DARK);
    current_state = AppState::MAIN_VIEW;
    setup_main_component();
    screen_initialized = false; // Will be initialized on first get_input() call
}

void FTXUIRenderer::cleanup()
{
    app_running = false;
    // No need to manage ScreenInteractive manually
}

void FTXUIRenderer::setup_main_component()
{
    // Create main component that handles all rendering and events
    main_component = Renderer([&] {
        switch (current_state) {
            case AppState::HELP_VIEW:
                return create_help_view();
            case AppState::GOTO_DIALOG:
                return create_goto_dialog();
            case AppState::MAIN_VIEW:
            default:
                return create_main_view();
        }
    });
    
    // Add global event handling
    main_component = CatchEvent(main_component, [&](Event event) {
        return handle_event(event);
    });
}

bool FTXUIRenderer::handle_event(Event event)
{
    switch (current_state) {
        case AppState::HELP_VIEW:
            // Any key closes help
            current_state = AppState::MAIN_VIEW;
            return true;
            
        case AppState::GOTO_DIALOG:
            if (event == Event::Escape) {
                current_state = AppState::MAIN_VIEW;
                goto_input.clear();
                return true;
            } else if (event == Event::Return) {
                current_state = AppState::MAIN_VIEW;
                // Set last_input to special value to indicate goto
                last_input = 1000 + std::atoi(goto_input.c_str()); // Special encoding
                input_ready = true;
                goto_input.clear();
                return true;
            } else if (event.is_character()) {
                char c = event.character()[0];
                if (c >= '0' && c <= '9') {
                    goto_input += c;
                } else if (c == 8 || c == 127) { // Backspace
                    if (!goto_input.empty()) {
                        goto_input.pop_back();
                    }
                }
                return true;
            }
            return false;
            
        case AppState::MAIN_VIEW:
            // Handle main view events
            if (event.is_character()) {
                char c = event.character()[0];
                if (c == 'h' || c == '?') {
                    current_state = AppState::HELP_VIEW;
                    return true;
                } else if (c == 'g') {
                    current_state = AppState::GOTO_DIALOG;
                    goto_input.clear();
                    return true;
                } else {
                    // Regular character input
                    last_input = c;
                    input_ready = true;
                    return true; // Let the caller handle screen exit
                }
            } else if (event == Event::ArrowLeft) {
                last_input = 260;
                input_ready = true;
                return true;
            } else if (event == Event::ArrowRight) {
                last_input = 261;
                input_ready = true;
                return true;
            } else if (event == Event::ArrowUp) {
                last_input = 259;
                input_ready = true;
                return true;
            } else if (event == Event::ArrowDown) {
                last_input = 258;
                input_ready = true;
                return true;
            } else if (event == Event::Return) {
                last_input = '\n';
                input_ready = true;
                return true;
            } else if (event == Event::Escape) {
                last_input = 27;
                input_ready = true;
                return true;
            } else if (event == Event::Backspace) {
                last_input = 8;
                input_ready = true;
                return true;
            }
            return false;
    }
    return false;
}

Element FTXUIRenderer::create_main_view()
{
    std::vector<Element> layout;
    
    // Header
    if (!header_text.empty()) {
        layout.push_back(
            text(header_text) | bold | color(title_color) | bgcolor(bg_color)
        );
        layout.push_back(separator());
    }
    
    // Main slide content (takes up most of the space)
    layout.push_back(create_slide_content() | flex);
    
    // Progress bar (fixed position near bottom)
    if (!progress_bar_text.empty()) {
        layout.push_back(separator());
        layout.push_back(
            text(progress_bar_text) | color(accent_color) | center
        );
    }
    
    // Message area (if any)
    if (!message_text.empty()) {
        layout.push_back(
            text(message_text) | bold | color(accent_color)
        );
    }
    
    // Footer (always at bottom)
    if (!footer_text.empty()) {
        layout.push_back(separator());
        layout.push_back(
            text(footer_text) | color(text_color)
        );
    }
    
    return vbox(layout) | bgcolor(bg_color);
}

Element FTXUIRenderer::create_help_view()
{
    return create_help_element(true);
}

Element FTXUIRenderer::create_goto_dialog()
{
    return vbox({
        filler(),
        vbox({
            text("Go to slide"),
            text(""),
            hbox({
                text("Enter slide number: "),
                text(goto_input + "_") | bold | color(accent_color)
            }),
            text(""),
            text("Press Enter to confirm, Escape to cancel") | dim
        }) | border | center | bgcolor(bg_color) | 
          size(WIDTH, EQUAL, 40) | size(HEIGHT, EQUAL, 8),
        filler()
    });
}

int FTXUIRenderer::get_input()
{
    input_ready = false;
    last_input = 0;
    
    // Very simple approach - one screen, one loop, exit immediately on input
    auto component = Renderer([&] {
        switch (current_state) {
            case AppState::HELP_VIEW:
                return create_help_view();
            case AppState::GOTO_DIALOG:
                return create_goto_dialog();
            case AppState::MAIN_VIEW:
            default:
                return create_main_view();
        }
    });
    
    component = CatchEvent(component, [&](Event event) {
        bool result = handle_event(event);
        // Exit immediately when any event is handled
        return result;
    });
    
    auto screen = ScreenInteractive::Fullscreen();
    screen.Loop(component);
    
    // Handle special goto encoding
    if (last_input > 1000) {
        int slide_num = last_input - 1000;
        if (slide_num > 0) {
            goto_input = std::to_string(slide_num);
            return 'g';
        }
        return 0;
    }
    
    return last_input.load();
}

void FTXUIRenderer::render_slide(const std::vector<SlideElement> &elements, bool animated)
{
    current_elements = elements;
    current_state = AppState::MAIN_VIEW;
    
    if (animated) {
        sleep_ms(100);
    }
}

void FTXUIRenderer::clear_screen()
{
    current_elements.clear();
}

void FTXUIRenderer::draw_header(int current_slide, int total_slides, 
                                const std::string &theme_name, bool show_timer, 
                                int minutes, int seconds, bool utf8_mode)
{
    header_text = "Slide " + std::to_string(current_slide + 1) + "/" + 
                  std::to_string(total_slides);
    
    if (show_timer) {
        header_text += " | Time: " + std::to_string(minutes) + ":" + 
                       (seconds < 10 ? "0" : "") + std::to_string(seconds);
    }
    
    header_text += " | Mode: " + std::string(utf8_mode ? "UTF-8" : "ASCII") +
                   " | Theme: " + theme_name;
}

void FTXUIRenderer::draw_footer()
{
    footer_text = "Controls: ←/→ Navigate | ENTER Execute | u/d Scroll | 't' Theme | 'h' Help | 'g' Goto | 'q' Quit";
}

void FTXUIRenderer::draw_progress_bar(int current_slide, int total_slides)
{
    if (total_slides == 0) return;
    
    int progress_width = 50;
    int filled = (current_slide * progress_width) / total_slides;
    
    progress_bar_text = "[";
    for (int i = 0; i < progress_width; ++i) {
        progress_bar_text += (i < filled) ? "#" : " ";
    }
    progress_bar_text += "]";
}

void FTXUIRenderer::show_help(bool /* utf8_supported */)
{
    current_state = AppState::HELP_VIEW;
}

void FTXUIRenderer::show_message(const std::string &message, int /* y */)
{
    message_text = message;
}

void FTXUIRenderer::clear_message_area()
{
    message_text.clear();
}

void FTXUIRenderer::get_string(char *buffer, int max_length)
{
    current_state = AppState::GOTO_DIALOG;
    goto_input.clear();
    
    // The event handler will fill goto_input and exit when Enter is pressed
    // For now, return the goto_input after state change
    strncpy(buffer, goto_input.c_str(), max_length - 1);
    buffer[max_length - 1] = '\0';
}

int FTXUIRenderer::get_screen_width() const
{
    return 80;
}

int FTXUIRenderer::get_screen_height() const
{
    return 24;
}

void FTXUIRenderer::enable_echo()
{
    // Not needed for FTXUI
}

void FTXUIRenderer::disable_echo()
{
    // Not needed for FTXUI
}

void FTXUIRenderer::apply_theme(Theme theme)
{
    theme_manager.setup_theme(theme);
    setup_theme_colors(theme);
}

void FTXUIRenderer::refresh_display()
{
    // FTXUI refreshes automatically
}

void FTXUIRenderer::sleep_ms(int milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void FTXUIRenderer::set_utf8_support(bool /* enabled */)
{
    // FTXUI has native UTF-8 support
}

// Helper methods implementation

Element FTXUIRenderer::create_slide_content()
{
    std::vector<Element> slide_content;
    
    if (!current_elements.empty()) {
        for (const auto &element : current_elements) {
            if (element.type != ElementType::SHELL_OUTPUT) {
                Element rendered_element = render_slide_element(element);
                
                // Apply proper indentation
                int indent = std::max(0, element.x - 2);
                if (indent > 0) {
                    std::string indent_str(indent, ' ');
                    rendered_element = hbox({
                        text(indent_str),
                        rendered_element
                    });
                }
                
                slide_content.push_back(rendered_element);
                
                // Add spacing after headers
                if (element.type == ElementType::HEADER1 || 
                    element.type == ElementType::HEADER2 || 
                    element.type == ElementType::HEADER3) {
                    slide_content.push_back(text(""));
                }
            }
        }
    } else {
        slide_content.push_back(
            text("No slide content loaded") | color(text_color) | center
        );
    }
    
    return vbox(slide_content) | flex;
}

Element FTXUIRenderer::render_slide_element(const SlideElement &element)
{
    Element content = text(element.content);
    
    switch (element.type) {
        case ElementType::HEADER1:
            content = content | bold | color(title_color) | center;
            break;
        case ElementType::HEADER2:
            content = content | bold | color(accent_color) | underlined;
            break;
        case ElementType::HEADER3:
            content = content | bold | color(text_color) | italic;
            break;
        case ElementType::CODE_BLOCK:
            content = content | color(code_color);
            break;
        case ElementType::SHELL_COMMAND:
            content = content | color(code_color) | bold;
            break;
        case ElementType::BULLET:
        case ElementType::NUMBERED:
        case ElementType::TEXT:
            content = content | color(text_color);
            if (element.is_bold) {
                content = content | bold;
            }
            break;
        default:
            content = content | color(text_color);
            break;
    }
    
    return content;
}

Element FTXUIRenderer::create_help_element(bool /* utf8_supported */)
{
    std::vector<Element> help_content = {
        text(""),
        text("MARKDOWN SLIDE PRESENTER - HELP") | bold | color(title_color) | center,
        text(""),
        separator(),
        text(""),
        text("Navigation:") | bold | color(accent_color),
        text("  → / Space / l    Next slide") | color(text_color),
        text("  ← / Backspace    Previous slide") | color(text_color),
        text("  g                Go to specific slide") | color(text_color),
        text("  Home / 0         First slide") | color(text_color),
        text("  End / $          Last slide") | color(text_color),
        text("  ENTER            Execute shell commands") | color(text_color),
        text("  u / d            Scroll shell output up/down") | color(text_color),
        text(""),
        text("Display:") | bold | color(accent_color),
        text("  t                Cycle themes") | color(text_color),
        text("  a                Toggle animations") | color(text_color),
        text("  T                Toggle timer") | color(text_color),
        text("  r                Refresh/redraw") | color(text_color),
        text(""),
        text("Other:") | bold | color(accent_color),
        text("  h                Show this help") | color(text_color),
        text("  q / Escape       Quit") | color(text_color),
        text(""),
        separator(),
        text(""),
        text("Press any key to continue...") | bold | color(title_color) | center
    };
    
    return vbox(help_content) | bgcolor(bg_color) | border | center | 
           size(WIDTH, LESS_THAN, 80) | size(HEIGHT, LESS_THAN, 25);
}

void FTXUIRenderer::setup_theme_colors(Theme theme)
{
    switch (theme) {
        case Theme::DARK:
            bg_color = Color::Black;
            title_color = Color::Cyan;
            text_color = Color::White;
            accent_color = Color::Green;
            code_color = Color::Magenta;
            break;
        case Theme::LIGHT:
            bg_color = Color::White;
            title_color = Color::Blue;
            text_color = Color::Black;
            accent_color = Color::Green;
            code_color = Color::Magenta;
            break;
        case Theme::MATRIX:
            bg_color = Color::Black;
            title_color = Color::Green;
            text_color = Color::Green;
            accent_color = Color::White;
            code_color = Color::Green;
            break;
        case Theme::RETRO:
            bg_color = Color::Black;
            title_color = Color::Yellow;
            text_color = Color::White;
            accent_color = Color::Magenta;
            code_color = Color::Red;
            break;
    }
}