#include "presentation_app.hh"
#include "markdown_parser.hh"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <memory>
#include <array>
#include <cstdlib>
#include <thread>
#include <atomic>

using namespace ftxui;

PresentationApp::PresentationApp() {
    state_.start_time = std::chrono::steady_clock::now();
    state_.start_slide_animation(); // Initialize animation state
    shell_popup_ = std::make_unique<ShellPopup>(80, 24);
    setup_components();
    update_theme();
    update_theme_colors();
}

PresentationApp::~PresentationApp() = default;

void PresentationApp::load_slides(const std::string& filename) {
    MarkdownParser parser;
    parser.set_utf8_support(state_.utf8_supported);
    parser.load_slides(filename, state_.slides);
}

void PresentationApp::run() {
    if (state_.slides.is_empty()) {
        printf("No slides loaded!\n");
        return;
    }
    
    auto screen = ScreenInteractive::Fullscreen();
    
    // Start a background thread for animation updates
    std::atomic<bool> keep_running{true};
    std::thread animation_thread([&]() {
        while (keep_running) {
            if (state_.slide_changed && state_.animations_enabled) {
                // Force a screen refresh during animations
                screen.PostEvent(Event::Custom);
                std::this_thread::sleep_for(std::chrono::milliseconds(30)); // Faster refresh for smooth animation
            } else {
                // Longer sleep when no animation
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        }
    });
    
    // Create main component with custom event handling
    auto app_component = CatchEvent(main_component_, [&](Event event) {
        if (event == Event::Custom) {
            // Custom event for animation refresh - just trigger re-render
            return true;
        }
        
        switch (state_.app_state) {
            case AppState::HELP_VIEW:
                return handle_help_view_event(event);
            case AppState::GOTO_DIALOG:
                return handle_goto_dialog_event(event);
            case AppState::SHELL_EXECUTION:
                return handle_shell_confirmation_event(event);
            case AppState::SHELL_POPUP:
                return handle_shell_popup_event(event);
            case AppState::MAIN_VIEW:
            default:
                return handle_main_view_event(event);
        }
    });
    
    try {
        screen.Loop(app_component);
    } catch (...) {
        // Ensure cleanup happens even if there's an exception
    }
    
    // Stop animation thread and cleanup
    keep_running = false;
    if (animation_thread.joinable()) {
        animation_thread.join();
    }
    
    // Force terminal cleanup
    cleanup_terminal();
}

bool PresentationApp::handle_shell_popup_event(Event  /* event */) {
    // The shell popup handles its own events, but you might want to 
    // check if it's still running and transition back to main view
    if (!shell_popup_ || !shell_popup_->get_is_running()) {
        state_.app_state = AppState::MAIN_VIEW;
        return true;
    }
    return false;
}

void PresentationApp::setup_components() {
    main_component_ = Renderer([&] {
        // Update animations on every render
        state_.update_animation();
        
        switch (state_.app_state) {
            case AppState::HELP_VIEW:
                return render_help_view();
            case AppState::GOTO_DIALOG:
                return render_goto_dialog();
            case AppState::SHELL_EXECUTION:
                return render_shell_confirmation();
            case AppState::MAIN_VIEW:
            default:
                return render_main_view();
        }
    });
}

bool PresentationApp::handle_main_view_event(Event event) {
    // Handle shell selection first if active
    if (state_.shell_selection_active) {
        if (event == Event::ArrowUp) {
            state_.navigate_shell_up();
            return true;
        } else if (event == Event::ArrowDown) {
            state_.navigate_shell_down();
            return true;
        } else if (event == Event::Escape) {
            state_.exit_shell_selection();
            state_.status_message = "";
            return true;
        } else if (event == Event::Return) {
                std::string command = state_.get_selected_shell_command();
                if (!command.empty())
                {
                    state_.pending_shell_command = command;
                    state_.app_state = AppState::SHELL_EXECUTION;
                    state_.exit_shell_selection();
                }
            return true;
        }
        return false; // Let other events through during selection
    }
    
    if (event.is_character()) {
        char c = event.character()[0];
        
        switch (c) {
            case 'q':
            case 'Q':
                cleanup_terminal();
                exit(0);
                return true;
                
            case 'h':
            case '?':
                state_.app_state = AppState::HELP_VIEW;
                return true;
                
            case 'g':
            case 'G':
                state_.app_state = AppState::GOTO_DIALOG;
                state_.goto_input.clear();
                return true;
                
            case 't':
                state_.cycle_theme();
                theme_manager_.cycle_theme();
                update_theme();
                state_.status_message = "Theme changed";
                return true;
                
            case 'T':
                state_.show_timer = !state_.show_timer;
                state_.status_message = state_.show_timer ? "Timer enabled" : "Timer disabled";
                return true;
                
            case 'a':
            case 'A':
                state_.animations_enabled = !state_.animations_enabled;
                state_.status_message = state_.animations_enabled ? "Animations enabled" : "Animations disabled";
                return true;
                
            case 'r':
            case 'R':
                state_.status_message = "Screen refreshed";
                return true;
                
            case 'l':
            case 'L':
            case ' ':
                state_.next_slide();
                return true;
                
            case '0':
                state_.current_slide = 0;
                state_.start_slide_animation();
                return true;
                
            case '$':
                state_.current_slide = state_.slides.get_slide_count() - 1;
                state_.start_slide_animation();
                return true;
                
                
            case 'u':
            case 'U':
                // Scroll shell output up (placeholder for now)
                state_.status_message = "Scroll up";
                return true;
                
            case 'd':
            case 'D':
                // Scroll shell output down (placeholder for now)  
                state_.status_message = "Scroll down";
                return true;
        }
    } else if (event == Event::ArrowRight || event == Event::ArrowDown) {
        state_.next_slide();
        return true;
    } else if (event == Event::ArrowLeft || event == Event::ArrowUp || event == Event::Backspace) {
        state_.prev_slide();
        return true;
    } else if (event == Event::Escape) {
        cleanup_terminal();
        exit(0);
        return true;
    }
    else if (event == Event::Return) {
        // Check for shell commands and start selection
        if (!state_.shell_command_indices.empty()) {
           state_.start_shell_selection();
           state_.status_message = "Shell command selection active. Use ↑↓ to select, Enter to execute, Escape to cancel.";
       } else {
           state_.status_message = "No shell commands on this slide";
       }
       return true;
   }
    
    return false;
}

bool PresentationApp::handle_help_view_event(Event /* event */) {
    state_.app_state = AppState::MAIN_VIEW;
    return true;
}

bool PresentationApp::handle_goto_dialog_event(Event event) {
    if (event == Event::Escape) {
        state_.app_state = AppState::MAIN_VIEW;
        state_.goto_input.clear();
        return true;
    } else if (event == Event::Return) {
        int slide_num = std::atoi(state_.goto_input.c_str());
        state_.goto_slide(slide_num);
        state_.app_state = AppState::MAIN_VIEW;
        state_.goto_input.clear();
        return true;
    } else if (event.is_character()) {
        char c = event.character()[0];
        if (c >= '0' && c <= '9') {
            state_.goto_input += c;
        } else if (c == 8 || c == 127) {
            if (!state_.goto_input.empty()) {
                state_.goto_input.pop_back();
            }
        }
        return true;
    }
    return false;
}

bool PresentationApp::handle_shell_confirmation_event(Event event) {
    if (event.is_character()) {
        char c = event.character()[0];
        if (c == 'y' || c == 'Y') {
            // Execute the shell command (this will show the popup and block until closed)
            execute_shell_command(state_.pending_shell_command);
            
            // After popup is closed, return to main view
            state_.app_state = AppState::MAIN_VIEW;
            return true;
        } else if (c == 'n' || c == 'N') {
            state_.app_state = AppState::MAIN_VIEW;
            return true;
        }
    } else if (event == Event::Escape) {
        state_.app_state = AppState::MAIN_VIEW;
        return true;
    }
    return false;
}

Element PresentationApp::render_main_view() {
    std::vector<Element> layout = {
        render_header(),
        separator() | color(current_colors_.separator_color),  // Use theme color
        render_slide_content() | flex,
        separator() | color(current_colors_.separator_color),  // Use theme color
        render_progress_bar(),
        render_footer()
    };
    
    if (!state_.status_message.empty()) {
        layout.insert(layout.end() - 1, text(state_.status_message) | color(Color::Yellow) | center);
    }
    
    return vbox(layout) | bgcolor(current_colors_.background_color);  // Use theme background
}

Element PresentationApp::render_help_view() {
    std::vector<Element> help_content = {
        text(""),
        text("MARKDOWN SLIDE PRESENTER - HELP") | bold | center | color(current_colors_.header_color),
        text(""),
        separator(),
        text(""),
        text("Navigation:") | bold | color(current_colors_.h1_color),
        text("  → / Space / l    Next slide")| color(current_colors_.text_color),
        text("  ← / Backspace    Previous slide") | color(current_colors_.text_color),
        text("  g                Go to specific slide") | color(current_colors_.text_color),
        text("  0                First slide") | color(current_colors_.text_color),
        text("  $                Last slide") | color(current_colors_.text_color),
        text("  ENTER            Execute shell commands") | color(current_colors_.text_color),
        text(""),
        text("Display:") | bold | color(current_colors_.h1_color),
        text("  t                Cycle themes") | color(current_colors_.text_color),
        text("  a                Toggle animations") | color(current_colors_.text_color),
        text("  T                Toggle timer") | color(current_colors_.text_color),
        text("  r                Refresh/redraw") | color(current_colors_.text_color),
        text(""),
        text("Other:") | bold | color(current_colors_.h1_color),
        text("  h / ?            Show this help") | color(current_colors_.text_color),
        text("  q / Escape       Quit") | color(current_colors_.text_color),
        text(""),
        separator(),
        text(""),
        text("Press any key to continue...") | bold | center | color(current_colors_.footer_color)
    };
    
    // Erstelle Popup-Fenster
    Element popup = vbox(help_content) | 
                   border | 
                   bgcolor(current_colors_.background_color) |  // Fester Hintergrund
                   size(WIDTH, LESS_THAN, 80) | 
                   size(HEIGHT, LESS_THAN, 25);
    
    // Kombiniere mit aktuellem Slide im Hintergrund
    Element background = render_main_view();  // Aktuelle Präsentation

    Element popup_layer = vbox({
        filler(),
        hbox({
            filler(),
            popup,
            filler()
        }),
        filler()
    });
    
    // Overlay-Effekt: Background + zentriertes Popup
    return dbox({
        background, 
        popup_layer
    });
}

Element PresentationApp::render_goto_dialog() {
    Element popup = vbox({
        text("Go to slide") | bold | center,
        text(""),
        hbox({
            text("Enter slide number (1-" + std::to_string(state_.slides.get_slide_count()) + "): "),
            text(state_.goto_input + "_") | bold | color(Color::Cyan)
        }) | center,
        text(""),
        text("Press Enter to confirm, Escape to cancel") | center
    }) | border | bgcolor(Color::Black) | center | 
         size(WIDTH, EQUAL, 50) | size(HEIGHT, EQUAL, 8);
    
    // Background mit aktuellem Slide
    Element background = render_main_view();
    
    return dbox({
        background,
        vbox({
            filler(),
            popup,
            filler()
        })
    });
}

Element PresentationApp::render_shell_confirmation() {
    Element popup = vbox({
        text("Execute Shell Command?") | bold | center | color(Color::Yellow),
        text(""),
        hbox({
            text("Command: "),
            text(state_.pending_shell_command) | color(Color::Cyan) | bold
        }) | center,
        text(""),
        hbox({
            text("Press "),
            text("Y") | bold | color(Color::Green),
            text(" to execute or "),
            text("N") | bold | color(Color::Red),
            text(" to cancel")
        }) | center,
        text(""),
        text("ESC also cancels") | center
    }) | border | bgcolor(Color::Black) | center | 
         size(WIDTH, EQUAL, 60) | size(HEIGHT, EQUAL, 10);
    
    // Background mit aktuellem Slide
    Element background = render_main_view();
    
    return dbox({
        background,
        vbox({
            filler(),
            popup,
            filler()
        })
    });
}

Element PresentationApp::render_slide_content() {
    std::vector<Element> slide_content;
    
    const auto& current_slide_elements = state_.get_current_slide();
    for (int i = 0; i < static_cast<int>(current_slide_elements.size()); ++i) {
        const auto& element = current_slide_elements[i];
        
        if (element.type != ElementType::SHELL_OUTPUT) {
            // Only show element if it's visible (for animations)
            if (state_.is_element_visible(i)) {
                Element rendered;
                
                // Special handling for shell commands to show selection
                if (element.type == ElementType::SHELL_COMMAND) {
                    rendered = render_shell_element_with_selection(element, i);
                } else {
                    rendered = render_slide_element(element);
                }
                
                // Apply animation effects
                if (state_.animations_enabled && state_.slide_changed) {
                    // For shell commands, only apply animation if not in selection mode
                    if (element.type != ElementType::SHELL_COMMAND || !state_.shell_selection_active) {
                        rendered = apply_animation_effect(rendered, element, i);
                    }
                }
                
                int indent = std::max(0, element.x - 2);
                if (indent > 0) {
                    std::string spaces(indent, ' ');
                    rendered = hbox({text(spaces), rendered});
                }
                
                slide_content.push_back(rendered);
                
                if (element.type == ElementType::HEADER1 || 
                    element.type == ElementType::HEADER2 || 
                    element.type == ElementType::HEADER3) {
                    slide_content.push_back(text(""));
                }
            }
        }
    }
    
    if (slide_content.empty()) {
        slide_content.push_back(text("Empty slide") | center);
    }
    
    return vbox(slide_content) | flex;
}

Element PresentationApp::render_slide_element(const SlideElement& element) {
    Element content = text(element.content);
    
    switch (element.type) {
        case ElementType::HEADER1:
            content = content | bold | color(current_colors_.h1_color) | center;
            break;
        case ElementType::HEADER2:
            content = content | bold | color(current_colors_.h2_color) | underlined;
            break;
        case ElementType::HEADER3:
            content = content | bold | color(current_colors_.h3_color) | italic;
            break;
        case ElementType::CODE_BLOCK:
            content = content | color(current_colors_.code_color);
            break;
        case ElementType::SHELL_COMMAND:
            content = content | color(current_colors_.shell_color) | bold;
            break;
        case ElementType::BULLET:
        case ElementType::NUMBERED:
        case ElementType::TEXT:
            content = content | color(current_colors_.text_color);
            if (element.is_bold) {
                content = content | bold;
            }
            break;
        default:
            content = content | color(current_colors_.text_color);
            break;
    }
    
    return content;
}

Element PresentationApp::render_header() {
    std::string slide_info = "Slide " + std::to_string(state_.current_slide + 1) + 
                            "/" + std::to_string(state_.slides.get_slide_count());
    
    std::string mode_info = "Mode: " + std::string(state_.utf8_supported ? "UTF-8" : "ASCII");
    
    const char* theme_names[] = {"Dark", "Light", "Matrix", "Retro"};
    std::string theme_info = "Theme: " + std::string(theme_names[static_cast<int>(state_.current_theme)]);
    
    Elements header_elements = {
        text(slide_info) | bold,
        text(mode_info),
        text(theme_info)
    };
    
    if (state_.show_timer) {
        std::string timer_info = "Time: " + format_timer();
        header_elements.insert(header_elements.begin() + 1, text(timer_info));
    }
    
    return hbox({
        header_elements[0],
        filler(),
        hbox(Elements(header_elements.begin() + 1, header_elements.end())) | center
    }) | color(current_colors_.header_color);  // Use theme color
}

Element PresentationApp::render_footer() {
    std::string controls = "Controls: ←/→ Navigate | ENTER Execute | t Theme | h Help | g Goto | q Quit";
    return text(controls) | center | color(current_colors_.footer_color);  // Use theme color
}

Element PresentationApp::render_progress_bar() {
    if (state_.slides.get_slide_count() == 0) {
        return text("");
    }
    
    int progress_width = 50;
    int filled = (state_.current_slide * progress_width) / state_.slides.get_slide_count();
    
    std::string progress_text = "[";
    for (int i = 0; i < progress_width; ++i) {
        progress_text += (i < filled) ? "#" : " ";
    }
    progress_text += "]";
    
    return text(progress_text) | center | color(current_colors_.progress_color);  // Use theme color
}

Element PresentationApp::render_shell_element_with_selection(const SlideElement& element, int element_index) {
    Element content = text(element.content);
    content = content | color(Color::Magenta) | bold;
    
    // Check if this shell command is selected
    if (state_.shell_selection_active && state_.get_selected_shell_index() == element_index) {
        // Highlight selected command
        content = content | bgcolor(Color::Blue) | color(Color::White);
        
        // Add selection indicators
        Element selection_indicator = hbox({
            text("→ ") | color(Color::Yellow) | bold,
            content,
            text(" ←") | color(Color::Yellow) | bold
        });
        return selection_indicator;
    }
    
    return content;
}


void PresentationApp::update_theme_colors() {
    using namespace ftxui;

    auto orange = Color::RGB(255, 165, 0);  // truecolor
  // auto orange = Color::Indexed(208);  // uncomment this for 256-color fallback

//   auto theme = theme_manager_.get_current_theme();
//   const auto &theme_config = theme_manager_.get_current_theme_config();
//   current_colors_ = {
//     .header_color = theme_config.title_color,
//     .text_color = theme_config.text_color,
//     .h1_color = theme_config.title_color,
//     .h2_color = theme_config.title_color,
//     .h3_color = theme_config.title_color,
//     .code_color = theme_config.code_color,
//     .shell_color = theme_config.code_color,
//     .separator_color = theme_config.accent_color,
//     .footer_color = theme_config.accent_color,
//     .progress_color = theme_config.accent_color,
//     .background_color = theme_config.bg_color
// };
    
    switch (state_.current_theme) {
        case Theme::DARK:
            current_colors_ = {
                .header_color = Color::Cyan,
                .text_color = Color::White,
                .h1_color = Color::Cyan,
                .h2_color = Color::Green,
                .h3_color = Color::White,
                .code_color = Color::Magenta,
                .shell_color = Color::Magenta,
                .separator_color = Color::GrayDark,
                .footer_color = Color::White,
                .progress_color = Color::Green,
                .background_color = Color::Black
            };
            break;
            
        case Theme::LIGHT:
            current_colors_ = {
                .header_color = Color::Blue,
                .text_color = Color::Black,
                .h1_color = Color::Blue,
                .h2_color = Color::DarkGreen,
                .h3_color = Color::GrayDark,
                .code_color = Color::Purple,
                .shell_color = Color::Purple,
                .separator_color = Color::GrayLight,
                .footer_color = Color::GrayDark,
                .progress_color = Color::DarkGreen,
                .background_color = Color::White
            };
            break;
            
        case Theme::MATRIX:
            current_colors_ = {
                .header_color = Color::GreenLight,
                .text_color = Color::Green,
                .h1_color = Color::GreenLight,
                .h2_color = Color::Green,
                .h3_color = Color::GreenLight,
                .code_color = Color::GreenLight,
                .shell_color = Color::GreenLight,
                .separator_color = Color::Green,
                .footer_color = Color::Green,
                .progress_color = Color::GreenLight,
                .background_color = Color::Black
            };
            break;
            
        case Theme::RETRO:
            current_colors_ = {
                .header_color = Color::Yellow,
                .text_color = orange,
                .h1_color = Color::Yellow,
                .h2_color = orange,
                .h3_color = Color::Red,
                .code_color = Color::Cyan,
                .shell_color = Color::Cyan,
                .separator_color = Color::Red,
                .footer_color = orange,
                .progress_color = Color::Yellow,
                .background_color = Color::Black
            };
            break;
    }
}

void PresentationApp::execute_shell_command(const std::string& command) {
    // Get current terminal size for proper popup sizing
    auto screen = ScreenInteractive::Fullscreen();
    int width = 80;  // Default fallback
    int height = 24; // Default fallback
    
    // Try to get actual terminal size (FTXUI doesn't expose this directly)
    // You might need to add a method to get screen dimensions
    
    // Create a new popup with current screen size
    shell_popup_ = std::make_unique<ShellPopup>(width, height);
    // Background-Provider setzen
    shell_popup_->set_background_provider([this]() {
        return render_main_view();  // Aktuelle Präsentation als Background
    });
    
    // Show the popup (this will block until user closes it)
    shell_popup_->show(command);
    
    // Update status message
    state_.status_message = "Command executed: " + command;
    
    // The popup handles its own execution and display, so we don't need to 
    // store output in slide elements for the FTXUI version
    // (unless you want to implement that feature as well)
}

std::string PresentationApp::run_shell_command(const std::string& command) {
    std::array<char, 128> buffer;
    std::string result;
    
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return "Error: Could not execute command";
    }
    
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    
    pclose(pipe);
    
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    
    return result;
}

void PresentationApp::update_theme() {
    theme_manager_.setup_theme(state_.current_theme);
    update_theme_colors();
}

void PresentationApp::cleanup_terminal() {
    // Reset terminal to normal state
    printf("\033[?25h");     // Show cursor
    printf("\033[0m");       // Reset all attributes
    printf("\033[2J");       // Clear screen
    printf("\033[H");        // Move cursor to home
    printf("\033[?1049l");   // Exit alternate screen buffer
    printf("\033[?1000l");   // Disable mouse reporting
    printf("\033[?1002l");   // Disable mouse tracking
    printf("\033[?1003l");   // Disable all mouse events
    fflush(stdout);
}

std::string PresentationApp::format_timer() const {
    int elapsed = state_.get_elapsed_seconds();
    int minutes = elapsed / 60;
    int seconds = elapsed % 60;
    
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << minutes 
        << ":" << std::setw(2) << seconds;
    return oss.str();
}

Element PresentationApp::apply_animation_effect(Element element, const SlideElement& slide_element, int index) {
    if (!state_.animations_enabled || !state_.slide_changed) {
        return element; // No animation
    }
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - state_.slide_start_time);
    
    // TIMING CONFIGURATION
    // const int TYPEWRITER_DURATION = 1200;   // ms for typewriter animation
    const int SLIDE_DURATION = 600;         // ms for slide-in animation  
    const int FADE_DURATION = 400;          // ms for fade animation
    const int TYPEWRITER_CHAR_DELAY = 40;   // ms per character in typewriter
    const int GAP_BETWEEN_ELEMENTS = 200;   // ms gap between animations
    
    // Calculate when THIS element should start (cumulative timing)
    int element_start_time = 0;
    const auto& current_slide_elements = state_.get_current_slide();
    
    for (int i = 0; i < index && i < static_cast<int>(current_slide_elements.size()); ++i) {
        // Check if this element and the next are both code elements (same code block)
        bool is_code_block = (current_slide_elements[i].type == ElementType::CODE_BLOCK || 
                             current_slide_elements[i].type == ElementType::SHELL_COMMAND);
        bool next_is_code = (i + 1 < static_cast<int>(current_slide_elements.size()) && 
                           (current_slide_elements[i + 1].type == ElementType::CODE_BLOCK || 
                            current_slide_elements[i + 1].type == ElementType::SHELL_COMMAND));
        
        // Add duration of previous element
        switch (current_slide_elements[i].type) {
            case ElementType::CODE_BLOCK:
            case ElementType::SHELL_COMMAND:
                // For code: only add character typing time, not full duration
                element_start_time += static_cast<int>(current_slide_elements[i].content.length()) * TYPEWRITER_CHAR_DELAY;
                break;
            case ElementType::HEADER1:
            case ElementType::HEADER2:
            case ElementType::HEADER3:
                element_start_time += SLIDE_DURATION;
                break;
            default:
                element_start_time += FADE_DURATION;
                break;
        }
        
        // Add gap only if this is NOT followed by another code element
        if (!(is_code_block && next_is_code)) {
            element_start_time += GAP_BETWEEN_ELEMENTS;
        } else {
            // Between code lines: only add a line break time (much shorter)
            element_start_time += 100; // 100ms for line break
        }
    }
    
    if (elapsed.count() < element_start_time) {
        return text(""); // Not yet time for this element
    }
    
    int element_elapsed = elapsed.count() - element_start_time;
    
    // Different animation effects based on element type
    switch (slide_element.type) {
        case ElementType::HEADER1:
        case ElementType::HEADER2:
        case ElementType::HEADER3:
            // Slide-in effect for headers
            if (element_elapsed < SLIDE_DURATION) {
                float progress = static_cast<float>(element_elapsed) / SLIDE_DURATION;
                // Ease-out function for smoother animation
                progress = 1.0f - (1.0f - progress) * (1.0f - progress);
                
                int offset = static_cast<int>((1.0f - progress) * 20); // Slide from right
                std::string spaces(offset, ' ');
                
                // Add dim effect in first half
                if (element_elapsed < SLIDE_DURATION / 2) {
                    return hbox({text(spaces), element | dim});
                } else {
                    return hbox({text(spaces), element});
                }
            }
            return element;
            
        case ElementType::CODE_BLOCK:
        case ElementType::SHELL_COMMAND:
            // Typewriter effect for code - duration based on actual content length
            {
                int actual_duration = static_cast<int>(slide_element.content.length()) * TYPEWRITER_CHAR_DELAY;
                if (element_elapsed < actual_duration) {
                    int chars_to_show = element_elapsed / TYPEWRITER_CHAR_DELAY;
                    
                    if (chars_to_show >= static_cast<int>(slide_element.content.length())) {
                        return element; // Fully typed
                    }
                    
                    std::string partial = slide_element.content.substr(0, chars_to_show);
                    
                    // Blinking cursor effect - only show on the last line of a code block
                    bool is_last_code_line = (index + 1 >= static_cast<int>(current_slide_elements.size()) ||
                                            (current_slide_elements[index + 1].type != ElementType::CODE_BLOCK &&
                                             current_slide_elements[index + 1].type != ElementType::SHELL_COMMAND));
                    
                    bool show_cursor = is_last_code_line && (elapsed.count() / 400) % 2; // Blink every 400ms
                    if (show_cursor && chars_to_show < static_cast<int>(slide_element.content.length())) {
                        partial += "_";
                    }
                    
                    Element result = text(partial);
                    // Apply same styling as original
                    if (slide_element.type == ElementType::SHELL_COMMAND) {
                        result = result | color(Color::Magenta) | bold;
                    } else {
                        result = result | color(Color::Magenta);
                    }
                    return result;
                }
                return element;
            }
            
        default:
            // Fade-in effect for text and bullets
            if (element_elapsed < FADE_DURATION) {
                if (element_elapsed < FADE_DURATION / 4) {
                    return text(""); // Invisible
                } else if (element_elapsed < FADE_DURATION / 2) {
                    return element | dim; // Dim appearance
                } else if (element_elapsed < FADE_DURATION * 3 / 4) {
                    return element; // Normal brightness
                } else {
                    return element | dim; // Brief dim again
                }
            }
            return element; // Fully visible
    }
}