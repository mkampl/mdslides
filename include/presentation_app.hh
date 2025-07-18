// include/presentation_app.hh
#pragma once

#include "slide_element.hh"
#include "theme_config.hh"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <vector>
#include <string>
#include <chrono>

class PresentationApp {
public:
    PresentationApp();
    ~PresentationApp();
    
    void load_slides(const std::string& filename);
    void run(); // Main event loop - replaces the old approach
    
private:
    // Application state
    enum class AppState {
        MAIN_VIEW,
        HELP_VIEW,
        GOTO_DIALOG,
        SHELL_EXECUTION
    };
    
    struct PresentationState {
        SlideCollection slides;
        int current_slide = 0;
        bool show_timer = false;
        bool animations_enabled = true;
        bool utf8_supported = true;
        std::chrono::steady_clock::time_point start_time;
        Theme current_theme = Theme::DARK;
        
        // UI state
        AppState app_state = AppState::MAIN_VIEW;
        std::string goto_input;
        std::string status_message;
        std::string pending_shell_command;
        bool shell_confirmation_visible = false;
        
        // Helper methods
        const std::vector<SlideElement>& get_current_slide() const {
            if (current_slide >= 0 && current_slide < slides.get_slide_count()) {
                return slides.get_slide(current_slide);
            }
            static std::vector<SlideElement> empty;
            return empty;
        }
        
        int get_elapsed_seconds() const {
            auto now = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - start_time);
            return duration.count();
        }
        
        void next_slide() {
            if (current_slide < slides.get_slide_count() - 1) {
                current_slide++;
            }
        }
        
        void prev_slide() {
            if (current_slide > 0) {
                current_slide--;
            }
        }
        
        void goto_slide(int slide_num) {
            if (slide_num >= 1 && slide_num <= slides.get_slide_count()) {
                current_slide = slide_num - 1;
            }
        }
        
        void cycle_theme() {
            current_theme = static_cast<Theme>((static_cast<int>(current_theme) + 1) % 4);
        }
    };
    
    PresentationState state_;
    ThemeManager theme_manager_;
    
    // FTXUI components
    ftxui::Component main_component_;
    
    // Event handlers
    bool handle_main_view_event(ftxui::Event event);
    bool handle_help_view_event(ftxui::Event event);
    bool handle_goto_dialog_event(ftxui::Event event);
    bool handle_shell_confirmation_event(ftxui::Event event);
    
    // UI rendering
    ftxui::Element render_main_view();
    ftxui::Element render_help_view();
    ftxui::Element render_goto_dialog();
    ftxui::Element render_shell_confirmation();
    
    ftxui::Element render_slide_content();
    ftxui::Element render_slide_element(const SlideElement& element);
    ftxui::Element render_header();
    ftxui::Element render_footer();
    ftxui::Element render_progress_bar();
    
    // Shell command handling
    void execute_shell_command(const std::string& command);
    std::string run_shell_command(const std::string& command);
    
    // Helper methods
    void setup_components();
    void update_theme();
    std::string format_timer() const;
};