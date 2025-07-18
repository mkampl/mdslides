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

using namespace ftxui;

PresentationApp::PresentationApp() {
    state_.start_time = std::chrono::steady_clock::now();
    setup_components();
    update_theme();
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
    screen.Loop(main_component_);
}

void PresentationApp::setup_components() {
    main_component_ = Renderer([&] {
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
    
    main_component_ = CatchEvent(main_component_, [&](Event event) {
        switch (state_.app_state) {
            case AppState::HELP_VIEW:
                return handle_help_view_event(event);
            case AppState::GOTO_DIALOG:
                return handle_goto_dialog_event(event);
            case AppState::SHELL_EXECUTION:
                return handle_shell_confirmation_event(event);
            case AppState::MAIN_VIEW:
            default:
                return handle_main_view_event(event);
        }
    });
}

bool PresentationApp::handle_main_view_event(Event event) {
    if (event.is_character()) {
        char c = event.character()[0];
        
        switch (c) {
            case 'q':
            case 'Q':
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
                return true;
                
            case '$':
                state_.current_slide = state_.slides.get_slide_count() - 1;
                return true;
                
            case '\n':
            case '\r':
                for (const auto& element : state_.get_current_slide()) {
                    if (element.type == ElementType::SHELL_COMMAND) {
                        state_.pending_shell_command = element.shell_command;
                        state_.app_state = AppState::SHELL_EXECUTION;
                        return true;
                    }
                }
                return true;
        }
    } else if (event == Event::ArrowRight || event == Event::ArrowDown) {
        state_.next_slide();
        return true;
    } else if (event == Event::ArrowLeft || event == Event::ArrowUp || event == Event::Backspace) {
        state_.prev_slide();
        return true;
    } else if (event == Event::Escape) {
        exit(0);
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
            execute_shell_command(state_.pending_shell_command);
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
        separator(),
        render_slide_content() | flex,
        separator(),
        render_progress_bar(),
        render_footer()
    };
    
    if (!state_.status_message.empty()) {
        layout.insert(layout.end() - 1, text(state_.status_message) | color(Color::Yellow) | center);
    }
    
    return vbox(layout);
}

Element PresentationApp::render_help_view() {
    std::vector<Element> help_content = {
        text(""),
        text("MARKDOWN SLIDE PRESENTER - HELP") | bold | center | color(Color::Cyan),
        text(""),
        separator(),
        text(""),
        text("Navigation:") | bold | color(Color::Green),
        text("  → / Space / l    Next slide"),
        text("  ← / Backspace    Previous slide"),
        text("  g                Go to specific slide"),
        text("  0                First slide"),
        text("  $                Last slide"),
        text("  ENTER            Execute shell commands"),
        text(""),
        text("Display:") | bold | color(Color::Green),
        text("  t                Cycle themes"),
        text("  a                Toggle animations"),
        text("  T                Toggle timer"),
        text("  r                Refresh/redraw"),
        text(""),
        text("Other:") | bold | color(Color::Green),
        text("  h / ?            Show this help"),
        text("  q / Escape       Quit"),
        text(""),
        separator(),
        text(""),
        text("Press any key to continue...") | bold | center | color(Color::Yellow)
    };
    
    return vbox(help_content) | border | center | 
           size(WIDTH, LESS_THAN, 80) | size(HEIGHT, LESS_THAN, 25);
}

Element PresentationApp::render_goto_dialog() {
    return vbox({
        filler(),
        vbox({
            text("Go to slide") | bold | center,
            text(""),
            hbox({
                text("Enter slide number (1-" + std::to_string(state_.slides.get_slide_count()) + "): "),
                text(state_.goto_input + "_") | bold | color(Color::Cyan)
            }) | center,
            text(""),
            text("Press Enter to confirm, Escape to cancel") | center
        }) | border | center | size(WIDTH, EQUAL, 50) | size(HEIGHT, EQUAL, 8),
        filler()
    });
}

Element PresentationApp::render_shell_confirmation() {
    return vbox({
        filler(),
        vbox({
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
        }) | border | center | size(WIDTH, EQUAL, 60) | size(HEIGHT, EQUAL, 10),
        filler()
    });
}

Element PresentationApp::render_slide_content() {
    std::vector<Element> slide_content;
    
    for (const auto& element : state_.get_current_slide()) {
        if (element.type != ElementType::SHELL_OUTPUT) {
            Element rendered = render_slide_element(element);
            
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
    
    if (slide_content.empty()) {
        slide_content.push_back(text("Empty slide") | center);
    }
    
    return vbox(slide_content) | flex;
}

Element PresentationApp::render_slide_element(const SlideElement& element) {
    Element content = text(element.content);
    
    switch (element.type) {
        case ElementType::HEADER1:
            content = content | bold | color(Color::Cyan) | center;
            break;
        case ElementType::HEADER2:
            content = content | bold | color(Color::Green) | underlined;
            break;
        case ElementType::HEADER3:
            content = content | bold | color(Color::White) | italic;
            break;
        case ElementType::CODE_BLOCK:
            content = content | color(Color::Magenta);
            break;
        case ElementType::SHELL_COMMAND:
            content = content | color(Color::Magenta) | bold;
            break;
        case ElementType::BULLET:
        case ElementType::NUMBERED:
        case ElementType::TEXT:
            content = content | color(Color::White);
            if (element.is_bold) {
                content = content | bold;
            }
            break;
        default:
            content = content | color(Color::White);
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
    }) | color(Color::Cyan);
}

Element PresentationApp::render_footer() {
    std::string controls = "Controls: ←/→ Navigate | ENTER Execute | t Theme | h Help | g Goto | q Quit";
    return text(controls) | center | color(Color::White);
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
    
    return text(progress_text) | center | color(Color::Green);
}

void PresentationApp::execute_shell_command(const std::string& command) {
    std::string output = run_shell_command(command);
    state_.status_message = "Command executed: " + command;
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