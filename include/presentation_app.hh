// include/presentation_app.hh
#pragma once

#include "slide_element.hh"
#include "theme_config.hh"
#include "shell_popup.hh"
#include "presentation_state.hh"
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
    
    
    ThemeColors current_colors_;
    
    void update_theme_colors();
    

    
    PresentationState state_;
    ThemeManager theme_manager_;
    std::unique_ptr<ShellPopup> shell_popup_;
    
    // FTXUI components
    ftxui::Component main_component_;
    
    // Event handlers
    bool handle_main_view_event(ftxui::Event event);
    bool handle_help_view_event(ftxui::Event event);
    bool handle_goto_dialog_event(ftxui::Event event);
    bool handle_shell_confirmation_event(ftxui::Event event);
    bool handle_shell_popup_event(ftxui::Event event);
    
    // UI rendering
    ftxui::Element render_main_view();
    ftxui::Element render_help_view();
    ftxui::Element render_goto_dialog();
    ftxui::Element render_shell_confirmation();
    
    ftxui::Element render_slide_content();
    ftxui::Element render_slide_element(const SlideElement& element);
    ftxui::Element render_shell_element_with_selection(const SlideElement& element, int element_index);
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