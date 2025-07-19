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
        
        // Animation state
        bool slide_changed = false;
        std::chrono::steady_clock::time_point slide_start_time;
        std::vector<bool> element_visible;  // Track which elements are visible
        int animation_step = 0;
        
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
                start_slide_animation();
            }
        }
        
        void prev_slide() {
            if (current_slide > 0) {
                current_slide--;
                start_slide_animation();
            }
        }
        
        void goto_slide(int slide_num) {
            if (slide_num >= 1 && slide_num <= slides.get_slide_count()) {
                current_slide = slide_num - 1;
                start_slide_animation();
            }
        }
        
        void start_slide_animation() {
            slide_changed = true;
            slide_start_time = std::chrono::steady_clock::now();
            animation_step = 0;
            
            // Reset element visibility
            const auto& current_slide_elements = get_current_slide();
            element_visible.clear();
            element_visible.resize(current_slide_elements.size(), false);
        }
        
        void update_animation() {
            if (!animations_enabled) {
                // No animation - show all elements immediately
                element_visible.assign(get_current_slide().size(), true);
                slide_changed = false;
                return;
            }
            
            if (!slide_changed) {
                // Animation already complete - show all elements
                element_visible.assign(get_current_slide().size(), true);
                return;
            }
            
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - slide_start_time);
            
            const auto& current_slide_elements = get_current_slide();
            
            // Ensure element_visible has correct size
            if (element_visible.size() != current_slide_elements.size()) {
                element_visible.resize(current_slide_elements.size(), false);
            }
            
            // Calculate when each element should start based on when previous finishes
            int current_time = 0;
            for (int i = 0; i < static_cast<int>(current_slide_elements.size()); ++i) {
                // Element becomes visible when its start time is reached
                element_visible[i] = (elapsed.count() >= current_time);
                
                // Calculate duration for this element type
                int element_duration;
                switch (current_slide_elements[i].type) {
                    case ElementType::CODE_BLOCK:
                    case ElementType::SHELL_COMMAND:
                        element_duration = 1200; // Typewriter duration
                        break;
                    case ElementType::HEADER1:
                    case ElementType::HEADER2:
                    case ElementType::HEADER3:
                        element_duration = 600; // Slide duration
                        break;
                    default:
                        element_duration = 400; // Fade duration
                        break;
                }
                
                // Next element starts after this one finishes + small gap
                current_time += element_duration + 200; // 200ms gap between animations
            }
            
            // Animation is done when all elements are complete
            if (elapsed.count() >= current_time) {
                slide_changed = false;
                element_visible.assign(current_slide_elements.size(), true);
            }
        }
        
        bool is_element_visible(int index) const {
            if (!animations_enabled || !slide_changed) {
                return true;
            }
            return index < static_cast<int>(element_visible.size()) && element_visible[index];
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
    void cleanup_terminal();
    
    // Animation methods
    ftxui::Element apply_animation_effect(ftxui::Element element, const SlideElement& slide_element, int index);
};