#include "slide_renderer.hpp"
#include <ftxui/dom/elements.hpp>
#include <sstream>
#include <iomanip>

using namespace ftxui;

namespace mdslides {

SlideRenderer::SlideRenderer(std::shared_ptr<ThemeManager> theme_manager,
                           std::shared_ptr<ShellExecutor> shell_executor)
    : theme_manager_(theme_manager)
    , shell_executor_(shell_executor)
    , animations_enabled_(true) {
}

SlideRenderer::~SlideRenderer() = default;

Element SlideRenderer::render_presentation_ui(const PresentationState& state) {
    Element content;
    
    if (state.help_visible) {
        content = render_help_screen();
    } else if (state.goto_dialog_visible) {
        content = render_goto_dialog(state);
    } else {
        // Main slide view
        const auto& current_slide = state.get_current_slide();
        Element slide_content = render_slide(current_slide, state.use_animations);
        content = vbox({
            render_header(state),
            separator(),
            slide_content | flex,
            separator(),
            render_progress_bar(state),
            render_footer()
        });
        
        // Shell confirmation overlay (if needed)
        if (state.shell_confirmation_visible) {
            auto confirmation_dialog = render_shell_confirmation_dialog(state.pending_shell_command);
            content = dbox({
                content,                    // Main content in background
                confirmation_dialog | center | clear_under | border | 
                bgcolor(Color::Black) | color(Color::White)
            });
        }
    }
    
    return theme_manager_->create_slide_container(content);
}

Element SlideRenderer::render_shell_confirmation_dialog(const std::string& command) {
    using namespace ftxui;  // Stelle sicher dass ftxui namespace verwendet wird
    
    return vbox({
        text("Execute Shell Command?") | bold | center | color(Color::Yellow),
        text(""),
        hbox({
            text("Command: "),
            text(command) | color(Color::Cyan) | bold
        }) | center,
        text(""),
        hbox({
            text("Press ") | color(Color::White),
            text("Y") | bold | color(Color::Green),
            text(" to execute or ") | color(Color::White),
            text("N") | bold | color(Color::Red),
            text(" to cancel") | color(Color::White)
        }) | center,
        text(""),
        text("ESC also cancels") | center | color(Color::Cyan)  // Gray -> DarkGray
    }) | border | size(WIDTH, EQUAL, 50) | size(HEIGHT, EQUAL, 8) | bgcolor(Color::Black);
}

Element SlideRenderer::render_slide(const Slide& slide, [[maybe_unused]] bool with_animations) {
    return render_slide_content(slide);
}

Element SlideRenderer::render_slide_content(const Slide& slide) {
    Elements content_elements;
    
    for (const auto& element : slide) {
        Element rendered = render_element(element);
        if (rendered) {
            content_elements.push_back(rendered);
        }
        
        // Add shell output if the element is an executed shell command
        if (element.type == ElementType::SHELL_COMMAND && element.executed) {
            Element output = render_shell_output(element);
            if (output) {
                content_elements.push_back(output);
            }
        }
    }
    
    if (content_elements.empty()) {
        content_elements.push_back(text("Empty slide"));
    }
    
    return vbox(content_elements) | xflex;
}

Element SlideRenderer::render_element(const SlideElement& element) {
    switch (element.type) {
        case ElementType::HEADER1:
        case ElementType::HEADER2:
        case ElementType::HEADER3:
            return render_header_element(element);
            
        case ElementType::CODE_BLOCK:
            return render_code_element(element);
            
        case ElementType::SHELL_COMMAND:
            return render_shell_element(element);
            
        case ElementType::SHELL_OUTPUT:
            // Shell output is handled separately
            return nullptr;
            
        case ElementType::BULLET:
        case ElementType::NUMBERED:
            return add_indentation(render_text_element(element), 2);
            
        default:
            return render_text_element(element);
    }
}

Element SlideRenderer::render_text_element(const SlideElement& element) {
    return theme_manager_->style_text(element.content, element.type);
}

Element SlideRenderer::render_header_element(const SlideElement& element) {
    Element header = theme_manager_->style_text(element.content, element.type);
    
    if (element.type == ElementType::HEADER1) {
        // Center H1 headers
        header = header | center;
    }
    
    return header;
}

Element SlideRenderer::render_code_element(const SlideElement& element) {
    return add_indentation(theme_manager_->style_code(element.content), 4);
}

Element SlideRenderer::render_shell_element(const SlideElement& element) {
    return add_indentation(theme_manager_->style_shell_command(element.content), 2);
}

Element SlideRenderer::render_shell_output(const SlideElement& element) {
    if (!element.executed || element.shell_output_lines.empty()) {
        return add_indentation(theme_manager_->style_shell_output("[Press ENTER to execute]", false), 4);
    }
    
    return create_scrollable_output(element);
}

Element SlideRenderer::create_scrollable_output(const SlideElement& element) {
    Elements output_lines;
    
    int lines_to_show = std::min(static_cast<int>(element.shell_output_lines.size()), 
                                element.max_output_lines);
    
    for (int i = 0; i < lines_to_show; ++i) {
        int line_idx = element.output_scroll_offset + i;
        if (line_idx < static_cast<int>(element.shell_output_lines.size())) {
            output_lines.push_back(
                theme_manager_->style_shell_output(element.shell_output_lines[line_idx], true)
            );
        }
    }
    
    Element output_box = vbox(output_lines);
    
    // Add scroll indicator if needed
    if (static_cast<int>(element.shell_output_lines.size()) > element.max_output_lines) {
        std::string scroll_info = format_scroll_indicator(element);
        Element scroll_indicator = theme_manager_->style_accent(scroll_info);
        output_box = vbox({output_box, scroll_indicator});
    }
    
    return add_indentation(output_box, 4);
}

std::string SlideRenderer::format_scroll_indicator(const SlideElement& element) {
    int lines_shown = std::min(static_cast<int>(element.shell_output_lines.size()), 
                              element.max_output_lines);
    int start_line = element.output_scroll_offset + 1;
    int end_line = element.output_scroll_offset + lines_shown;
    int total_lines = static_cast<int>(element.shell_output_lines.size());
    
    std::ostringstream oss;
    oss << "(" << start_line << "-" << end_line << "/" << total_lines << " lines)";
    
    if (shell_executor_->can_scroll_up(element)) {
        oss << " ↑u";
    }
    if (shell_executor_->can_scroll_down(element)) {
        oss << " ↓d";
    }
    
    return oss.str();
}

Element SlideRenderer::render_header(const PresentationState& state) {
    std::string slide_info = format_slide_info(state);
    std::string mode_info = format_mode_info(state.utf8_supported);
    std::string theme_info = "Theme: " + theme_manager_->get_current_theme().name;
    
    Elements header_elements = {
        theme_manager_->style_status_bar(slide_info),
        theme_manager_->style_status_bar(mode_info),
        theme_manager_->style_status_bar(theme_info)
    };
    
    if (state.show_timer) {
        std::string timer_info = "Time: " + format_timer(state.get_elapsed_seconds());
        header_elements.insert(header_elements.begin() + 1, 
                              theme_manager_->style_status_bar(timer_info));
    }
    
    return hbox({
        header_elements[0],
        filler(),
        hbox(Elements(header_elements.begin() + 1, header_elements.end())) | hcenter
    });
}

Element SlideRenderer::render_footer() {
    std::string controls = "Controls: ←/→ Navigate | ENTER Execute | u/d Scroll | t Theme | h Help | q Quit";
    return theme_manager_->style_help_text(controls) | hcenter;
}

Element SlideRenderer::render_progress_bar(const PresentationState& state) {
    if (state.slides.empty()) {
        return text("");
    }
    
    float progress = static_cast<float>(state.current_slide) / static_cast<float>(state.slides.size());
    return theme_manager_->style_progress_bar(progress) | hcenter;
}

Element SlideRenderer::render_help_screen() {
    Elements help_content = {
        theme_manager_->style_header1("MARKDOWN SLIDE PRESENTER - HELP"),
        text(""),
        theme_manager_->style_header2("Navigation:"),
        theme_manager_->style_help_text("  → / Space / l    Next slide"),
        theme_manager_->style_help_text("  ← / Backspace / h Previous slide"),
        theme_manager_->style_help_text("  g                Go to specific slide"),
        theme_manager_->style_help_text("  Home / 0         First slide"),
        theme_manager_->style_help_text("  End / $          Last slide"),
        theme_manager_->style_help_text("  ENTER            Execute shell commands"),
        theme_manager_->style_help_text("  u / d            Scroll shell output up/down"),
        text(""),
        theme_manager_->style_header2("Display:"),
        theme_manager_->style_help_text("  t                Cycle themes"),
        theme_manager_->style_help_text("  a                Toggle animations"),
        theme_manager_->style_help_text("  T                Toggle timer"),
        theme_manager_->style_help_text("  r                Refresh/redraw"),
        text(""),
        theme_manager_->style_header2("Other:"),
        theme_manager_->style_help_text("  h / ?            Show this help"),
        theme_manager_->style_help_text("  q / Escape       Quit"),
        text(""),
        theme_manager_->style_header2("Supported Markdown:"),
        theme_manager_->style_help_text("  # H1 Headers     ## H2 Headers    ### H3 Headers"),
        theme_manager_->style_help_text("  - Bullet points  1. Numbered lists **Bold text**"),
        theme_manager_->style_help_text("  ```code blocks```  ```$shell command```"),
        text(""),
        theme_manager_->style_accent("Press any key to continue...")
    };
    
    return vbox(help_content) | center;
}

Element SlideRenderer::render_goto_dialog(const PresentationState& state) {
    int max_slide = static_cast<int>(state.slides.size());
    std::string prompt = "Go to slide (1-" + std::to_string(max_slide) + "): " + state.goto_input;
    
    return vbox({
        filler(),
        theme_manager_->style_header2(prompt) | hcenter,
        filler()
    });
}

Element SlideRenderer::add_indentation(ftxui::Element content, int indent) const {
    std::string spaces(indent, ' ');
    return hbox({text(spaces), content});
}

std::string SlideRenderer::format_timer(int elapsed_seconds) const {
    int minutes = elapsed_seconds / 60;
    int seconds = elapsed_seconds % 60;
    
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << minutes 
        << ":" << std::setw(2) << seconds;
    return oss.str();
}

std::string SlideRenderer::format_slide_info(const PresentationState& state) const {
    return "Slide " + std::to_string(state.current_slide + 1) + "/" + 
           std::to_string(state.slides.size());
}

std::string SlideRenderer::format_mode_info(bool utf8_supported) const {
    return "Mode: " + std::string(utf8_supported ? "UTF-8" : "ASCII");
}

} // namespace mdslides
