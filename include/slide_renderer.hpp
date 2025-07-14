#pragma once

#include "slide_types.hpp"
#include "theme_manager.hpp"
#include "shell_executor.hpp"
#include <ftxui/dom/elements.hpp>
#include <memory>

namespace mdslides {

class SlideRenderer {
public:
    SlideRenderer(std::shared_ptr<ThemeManager> theme_manager, 
                  std::shared_ptr<ShellExecutor> shell_executor);
    ~SlideRenderer();
    
    // Main rendering functions
    ftxui::Element render_slide(const Slide& slide, bool with_animations = true);
    ftxui::Element render_presentation_ui(const PresentationState& state);
    
    // Individual element rendering
    ftxui::Element render_element(const SlideElement& element);
    ftxui::Element render_shell_output(const SlideElement& element);
    
    // UI components
    ftxui::Element render_header(const PresentationState& state);
    ftxui::Element render_footer();
    ftxui::Element render_progress_bar(const PresentationState& state);
    ftxui::Element render_help_screen();
    ftxui::Element render_goto_dialog(const PresentationState& state);
    ftxui::Element render_shell_confirmation_dialog(const std::string& command);
    
    // Layout helpers
    ftxui::Element create_slide_container(ftxui::Element content);
    ftxui::Element create_centered_content(ftxui::Element content);
    
    // Animation support (for future implementation)
    void set_animation_enabled(bool enabled) { animations_enabled_ = enabled; }
    bool is_animation_enabled() const { return animations_enabled_; }
    
    // Scrolling and viewport management
    ftxui::Element create_scrollable_output(const SlideElement& element);
    std::string format_scroll_indicator(const SlideElement& element);
    
private:
    std::shared_ptr<ThemeManager> theme_manager_;
    std::shared_ptr<ShellExecutor> shell_executor_;
    bool animations_enabled_;
    
    // Helper methods
    ftxui::Element render_slide_content(const Slide& slide);
    ftxui::Element render_text_element(const SlideElement& element);
    ftxui::Element render_header_element(const SlideElement& element);
    ftxui::Element render_code_element(const SlideElement& element);
    ftxui::Element render_shell_element(const SlideElement& element);
    
    // Layout utilities
    int calculate_header_indent(ElementType type) const;
    ftxui::Element add_indentation(ftxui::Element content, int indent) const;
    ftxui::Element create_bullet_point(const std::string& content) const;
    
    // Status and timing
    std::string format_timer(int elapsed_seconds) const;
    std::string format_slide_info(const PresentationState& state) const;
    std::string format_mode_info(bool utf8_supported) const;
};

} // namespace mdslides
