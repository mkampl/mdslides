#include "presentation_controller.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <locale.h>
#include <cstdlib>
#include <cstring>

using namespace ftxui;

namespace mdslides {

PresentationController::PresentationController() 
    : screen_(ScreenInteractive::Fullscreen())
    , should_quit_(false) {
    
    // Initialize components
    parser_ = std::make_shared<MarkdownParser>();
    theme_manager_ = std::make_shared<ThemeManager>();
    shell_executor_ = std::make_shared<ShellExecutor>();
    renderer_ = std::make_shared<SlideRenderer>(theme_manager_, shell_executor_);
    
    initialize_state();
    setup_components();
}

PresentationController::~PresentationController() = default;

void PresentationController::initialize_state() {
    detect_utf8_support();
    parser_->set_utf8_support(state_.utf8_supported);
    state_.start_time = std::chrono::steady_clock::now();
}

void PresentationController::detect_utf8_support() {
    setlocale(LC_ALL, "");
    
    const char* lang = getenv("LANG");
    const char* lc_all = getenv("LC_ALL");
    const char* lc_ctype = getenv("LC_CTYPE");
    
    state_.utf8_supported = 
        (lang && (strstr(lang, "UTF-8") || strstr(lang, "utf8"))) ||
        (lc_all && (strstr(lc_all, "UTF-8") || strstr(lc_all, "utf8"))) ||
        (lc_ctype && (strstr(lc_ctype, "UTF-8") || strstr(lc_ctype, "utf8")));
    
    if (!state_.utf8_supported) {
        char* current_locale = setlocale(LC_CTYPE, nullptr);
        if (current_locale && (strstr(current_locale, "UTF-8") || strstr(current_locale, "utf8"))) {
            state_.utf8_supported = true;
        }
    }
}

bool PresentationController::load_presentation(const std::string& filename) {
    try {
        state_.slides = parser_->parse_file(filename);
        if (state_.slides.empty()) {
            return false;
        }
        state_.current_slide = 0;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

void PresentationController::setup_components() {
    main_component_ = create_main_component();
}

Component PresentationController::create_main_component() {
    return Renderer([this] {
        return renderer_->render_presentation_ui(state_);
    }) | CatchEvent([this](Event event) {
        return handle_key_event(event);
    });
}

bool PresentationController::handle_key_event(Event event) {
    if (state_.goto_dialog_visible) {
        return handle_goto_key(event);
    }
    
    if (state_.help_visible) {
        // Any key closes help
        state_.help_visible = false;
        return true;
    }
    
    // Handle different types of keys
    if (handle_navigation_key(event)) return true;
    if (handle_display_key(event)) return true;
    if (handle_shell_key(event)) return true;
    
    // Global keys
    if (event == Event::Character('q') || event == Event::Escape) {
        should_quit_ = true;
        return true;
    }
    
    if (event == Event::Character('h') || event == Event::Character('?')) {
        toggle_help();
        return true;
    }
    
    if (event == Event::Character('g')) {
        show_goto_dialog();
        return true;
    }
    
    return false;
}

bool PresentationController::handle_navigation_key(Event event) {
    if (event == Event::ArrowRight || event == Event::Character(' ') || event == Event::Character('l')) {
        next_slide();
        return true;
    }
    
    if (event == Event::ArrowLeft || event == Event::Backspace || event == Event::Character('h')) {
        previous_slide();
        return true;
    }
    
    if (event == Event::Home || event == Event::Character('0')) {
        first_slide();
        return true;
    }
    
    if (event == Event::End || event == Event::Character('$')) {
        last_slide();
        return true;
    }
    
    return false;
}

bool PresentationController::handle_display_key(Event event) {
    if (event == Event::Character('t')) {
        cycle_theme();
        return true;
    }
    
    if (event == Event::Character('a')) {
        toggle_animations();
        return true;
    }
    
    if (event == Event::Character('T')) {
        toggle_timer();
        return true;
    }
    
    if (event == Event::Character('r')) {
        refresh_display();
        return true;
    }
    
    return false;
}

bool PresentationController::handle_shell_key(Event event) {
    if (event == Event::Return) {
        execute_shell_commands();
        return true;
    }
    
    if (event == Event::Character('u')) {
        scroll_shell_output_up();
        return true;
    }
    
    if (event == Event::Character('d')) {
        scroll_shell_output_down();
        return true;
    }
    
    return false;
}

bool PresentationController::handle_goto_key(Event event) {
    if (event == Event::Return) {
        if (is_valid_slide_number(state_.goto_input)) {
            int slide_num = parse_slide_number(state_.goto_input);
            goto_slide(slide_num);
        }
        state_.hide_goto_dialog();
        return true;
    }
    
    if (event == Event::Escape) {
        state_.hide_goto_dialog();
        return true;
    }
    
    if (event == Event::Backspace) {
        if (!state_.goto_input.empty()) {
            state_.goto_input.pop_back();
        }
        return true;
    }
    
    // Handle numeric input
    if (event.is_character() && std::isdigit(event.character()[0])) {
        if (state_.goto_input.length() < 5) { // Reasonable limit
            state_.goto_input += event.character();
        }
        return true;
    }
    
    return false;
}

void PresentationController::run() {
    if (state_.slides.empty()) {
        return;
    }
    
    // Execute shell commands on first slide if any
    execute_shell_commands_on_current_slide();
    
    screen_.Loop(main_component_);
}

void PresentationController::quit() {
    should_quit_ = true;
    screen_.Exit();
}

void PresentationController::next_slide() {
    if (state_.has_next_slide()) {
        state_.next_slide();
        execute_shell_commands_on_current_slide();
    }
}

void PresentationController::previous_slide() {
    if (state_.has_previous_slide()) {
        state_.previous_slide();
        execute_shell_commands_on_current_slide();
    }
}

void PresentationController::first_slide() {
    state_.first_slide();
    execute_shell_commands_on_current_slide();
}

void PresentationController::last_slide() {
    state_.last_slide();
    execute_shell_commands_on_current_slide();
}

void PresentationController::goto_slide(int slide_number) {
    if (state_.goto_slide(slide_number)) {
        execute_shell_commands_on_current_slide();
    }
}

void PresentationController::show_goto_dialog() {
    state_.show_goto_dialog();
}

void PresentationController::cycle_theme() {
    state_.cycle_theme();
    theme_manager_->set_theme(state_.current_theme);
}

void PresentationController::toggle_animations() {
    state_.toggle_animations();
    renderer_->set_animation_enabled(state_.use_animations);
}

void PresentationController::toggle_timer() {
    state_.toggle_timer();
}

void PresentationController::toggle_help() {
    state_.toggle_help();
}

void PresentationController::refresh_display() {
    // Force screen refresh - FTXUI handles this automatically
}

void PresentationController::execute_shell_commands() {
    execute_shell_commands_on_current_slide();
}

void PresentationController::execute_shell_commands_on_current_slide() {
    if (!state_.slides.empty()) {
        Slide& current_slide = state_.get_current_slide();
        if (shell_executor_->has_shell_commands(current_slide)) {
            shell_executor_->execute_shell_commands_in_slide(current_slide);
        }
    }
}

void PresentationController::scroll_shell_output_up() {
    if (!state_.slides.empty()) {
        Slide& current_slide = state_.get_current_slide();
        for (auto& element : current_slide) {
            if (element.type == ElementType::SHELL_COMMAND && element.executed) {
                shell_executor_->scroll_output_up(element);
            }
        }
    }
}

void PresentationController::scroll_shell_output_down() {
    if (!state_.slides.empty()) {
        Slide& current_slide = state_.get_current_slide();
        for (auto& element : current_slide) {
            if (element.type == ElementType::SHELL_COMMAND && element.executed) {
                shell_executor_->scroll_output_down(element);
            }
        }
    }
}

bool PresentationController::is_valid_slide_number(const std::string& input) const {
    if (input.empty()) return false;
    
    try {
        int num = std::stoi(input);
        return num >= 1 && num <= static_cast<int>(state_.slides.size());
    } catch (const std::exception&) {
        return false;
    }
}

int PresentationController::parse_slide_number(const std::string& input) const {
    try {
        return std::stoi(input);
    } catch (const std::exception&) {
        return 1;
    }
}

} // namespace mdslides
