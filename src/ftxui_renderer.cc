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
    : last_input(0), input_ready(false), echo_enabled(false), animation_active(false)
{
    setup_theme_colors(Theme::DARK);
}

FTXUIRenderer::~FTXUIRenderer()
{
    cleanup();
}

void FTXUIRenderer::initialize()
{
    // Initialize with basic setup
    theme_manager.setup_theme(Theme::DARK);
    setup_theme_colors(Theme::DARK);
}

void FTXUIRenderer::cleanup()
{
    stop_animation();
}

void FTXUIRenderer::render_slide(const std::vector<SlideElement> &elements, bool animated)
{
    current_elements = elements;
    
    if (animated) {
        // For now, just add a small delay for animation effect
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
    footer_text = "Controls: ←/→ Navigate | ENTER Execute | u/d Scroll | 't' Theme | 'h' Help | 'q' Quit";
}

void FTXUIRenderer::draw_progress_bar(int current_slide, int total_slides)
{
    if (total_slides == 0) return;
    
    int progress_width = 50; // Fixed width for progress bar
    int filled = (current_slide * progress_width) / total_slides;
    
    progress_bar_text = "[";
    for (int i = 0; i < progress_width; ++i) {
        progress_bar_text += (i < filled) ? "#" : " ";
    }
    progress_bar_text += "]";
}

void FTXUIRenderer::show_help(bool /* utf8_supported */)
{
    // Simple help screen without custom event loops
    auto help_component = Renderer([&] {
        return create_help_element(true);
    });
    
    auto screen = ScreenInteractive::Fullscreen();
    screen.Loop(help_component);
}

void FTXUIRenderer::show_message(const std::string &message, int /* y */)
{
    message_text = message;
}

void FTXUIRenderer::clear_message_area()
{
    message_text.clear();
}

int FTXUIRenderer::get_input()
{
    // Create a screen for this input session
    auto screen = ScreenInteractive::Fullscreen();
    
    input_ready = false;
    last_input = 0;
    
    // Create a component that renders the current content and handles input
    auto input_component = Renderer([&] {
        return create_slide_content();
    });
    
    // Wrap with event handling
    input_component = CatchEvent(input_component, [&](Event event) {
        if (event.is_character()) {
            last_input = event.character()[0];
            input_ready = true;
            screen.Exit();
            return true;
        } else if (event == Event::ArrowLeft) {
            last_input = 260; // KEY_LEFT equivalent
            input_ready = true;
            screen.Exit();
            return true;
        } else if (event == Event::ArrowRight) {
            last_input = 261; // KEY_RIGHT equivalent
            input_ready = true;
            screen.Exit();
            return true;
        } else if (event == Event::ArrowUp) {
            last_input = 259; // KEY_UP equivalent
            input_ready = true;
            screen.Exit();
            return true;
        } else if (event == Event::ArrowDown) {
            last_input = 258; // KEY_DOWN equivalent
            input_ready = true;
            screen.Exit();
            return true;
        } else if (event == Event::Return) {
            last_input = '\n';
            input_ready = true;
            screen.Exit();
            return true;
        } else if (event == Event::Escape) {
            last_input = 27;
            input_ready = true;
            screen.Exit();
            return true;
        } else if (event == Event::Backspace) {
            last_input = 8; // Backspace
            input_ready = true;
            screen.Exit();
            return true;
        }
        
        return false;
    });
    
    // Run the event loop until we get input
    screen.Loop(input_component);
    
    return last_input.load();
}

int FTXUIRenderer::get_screen_width() const
{
    return 80; // Default fallback
}

int FTXUIRenderer::get_screen_height() const
{
    return 24; // Default fallback
}

void FTXUIRenderer::enable_echo()
{
    echo_enabled = true;
}

void FTXUIRenderer::disable_echo()
{
    echo_enabled = false;
}

void FTXUIRenderer::get_string(char *buffer, int max_length)
{
    // Simplified implementation - show message and get single character input
    show_message("Enter slide number (1-9) and press key:", -1);
    
    int input = get_input();
    
    // Convert single digit input to string
    if (input >= '1' && input <= '9') {
        buffer[0] = input;
        buffer[1] = '\0';
    } else {
        buffer[0] = '\0'; // Invalid input
    }
    
    clear_message_area();
}

void FTXUIRenderer::apply_theme(Theme theme)
{
    theme_manager.setup_theme(theme);
    setup_theme_colors(theme);
}

void FTXUIRenderer::refresh_display()
{
    // With FTXUI, the display refreshes automatically when components change
}

void FTXUIRenderer::sleep_ms(int milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void FTXUIRenderer::set_utf8_support(bool /* enabled */)
{
    // FTXUI has native UTF-8 support, so this is mainly for compatibility
}

// Helper methods implementation

Element FTXUIRenderer::create_slide_content()
{
    std::vector<Element> slide_content;
    
    // Add slide elements with proper spacing and indentation
    if (!current_elements.empty()) {
        for (const auto &element : current_elements) {
            // Skip shell output elements for now
            if (element.type != ElementType::SHELL_OUTPUT) {
                Element rendered_element = render_slide_element(element);
                
                // Apply proper indentation based on element type and original x position
                int indent = std::max(0, element.x - 2); // Convert absolute x to relative indent
                if (indent > 0) {
                    std::string indent_str(indent, ' ');
                    rendered_element = hbox({
                        text(indent_str),
                        rendered_element
                    });
                }
                
                slide_content.push_back(rendered_element);
                
                // Add spacing after certain elements
                if (element.type == ElementType::HEADER1 || 
                    element.type == ElementType::HEADER2 || 
                    element.type == ElementType::HEADER3) {
                    slide_content.push_back(text(""));
                }
            }
        }
    } else {
        // Show placeholder if no content
        slide_content.push_back(
            text("No slide content loaded") | color(text_color) | center
        );
    }
    
    // Create the main slide area
    Element main_slide = vbox(slide_content) | flex;
    
    // Create the complete layout with header, content, progress, footer
    std::vector<Element> layout;
    
    // Header
    if (!header_text.empty()) {
        layout.push_back(
            text(header_text) | bold | color(title_color) | bgcolor(bg_color)
        );
        layout.push_back(separator());
    }
    
    // Main slide content (takes up most of the space)
    layout.push_back(main_slide | flex);
    
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

Element FTXUIRenderer::render_slide_element(const SlideElement &element)
{
    Element content = text(element.content);
    
    // Apply styling based on element type
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
            content = content | color(text_color);
            break;
        case ElementType::NUMBERED:
            content = content | color(text_color);
            break;
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
        text("  g                Go to specific slide (Ctrl+Enter to confirm)") | color(text_color),
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

void FTXUIRenderer::start_animation_loop()
{
    // Placeholder for animation implementation
    animation_active = true;
}

void FTXUIRenderer::stop_animation()
{
    animation_active = false;
    if (animation_thread.joinable()) {
        animation_thread.join();
    }
}