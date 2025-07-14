#pragma once

#include "slide_types.hpp"
#include <string>
#include <vector>
#include <memory>

namespace mdslides {

struct ShellResult {
    std::string output;
    int exit_code;
    bool success;
    std::string error_message;
    
    ShellResult() : exit_code(-1), success(false) {}
    ShellResult(const std::string& out, int code) 
        : output(out), exit_code(code), success(code == 0) {}
};

class ShellExecutor {
public:
    ShellExecutor();
    ~ShellExecutor();
    
    // Main execution methods
    ShellResult execute_command(const std::string& command);
    
    // Slide element processing
    void execute_shell_commands_in_slide(Slide& slide);
    bool has_shell_commands(const Slide& slide) const;
    
    // Individual element processing
    void execute_shell_element(SlideElement& element);
    
    // Output management
    std::vector<std::string> split_output_lines(const std::string& output) const;
    void update_element_output(SlideElement& element, const ShellResult& result);
    
    // Scrolling support
    void scroll_output_up(SlideElement& element);
    void scroll_output_down(SlideElement& element);
    bool can_scroll_up(const SlideElement& element) const;
    bool can_scroll_down(const SlideElement& element) const;
    
    // Settings
    void set_max_output_lines(int lines) { max_output_lines_ = lines; }
    int get_max_output_lines() const { return max_output_lines_; }
    
    // Security and validation
    bool is_command_safe(const std::string& command) const;
    std::string sanitize_command(const std::string& command) const;
    
private:
    int max_output_lines_;
    std::vector<std::string> dangerous_commands_;
    
    void initialize_dangerous_commands();
    std::string execute_system_command(const std::string& command);
    void trim_string(std::string& str) const;
};

} // namespace mdslides
