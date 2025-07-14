#include "shell_executor.hpp"
#include <cstdio>
#include <memory>
#include <array>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace mdslides {

ShellExecutor::ShellExecutor() : max_output_lines_(5) {
    initialize_dangerous_commands();
}

ShellExecutor::~ShellExecutor() = default;

void ShellExecutor::initialize_dangerous_commands() {
    dangerous_commands_ = {
        "rm", "rmdir", "del", "format", "fdisk",
        "dd", "mkfs", "sudo", "su", "passwd",
        "shutdown", "reboot", "halt", "init",
        "kill", "killall", "pkill", "fuser"
    };
}

bool ShellExecutor::is_command_safe(const std::string& command) const {
    std::string cmd_lower = command;
    std::transform(cmd_lower.begin(), cmd_lower.end(), cmd_lower.begin(), ::tolower);
    
    // Extract the first word (actual command)
    std::istringstream iss(cmd_lower);
    std::string first_word;
    iss >> first_word;
    
    // Check against dangerous commands
    for (const auto& dangerous : dangerous_commands_) {
        if (first_word == dangerous || first_word.find(dangerous + " ") == 0) {
            return false;
        }
    }
    
    // Check for potentially dangerous patterns
    if (cmd_lower.find(">/") != std::string::npos ||
        cmd_lower.find(">>") != std::string::npos ||
        cmd_lower.find("&&") != std::string::npos ||
        cmd_lower.find("||") != std::string::npos ||
        cmd_lower.find(";") != std::string::npos ||
        cmd_lower.find("|") != std::string::npos) {
        return false;
    }
    
    return true;
}

std::string ShellExecutor::sanitize_command(const std::string& command) const {
    std::string sanitized = command;
    
    // Remove any null bytes or control characters
    sanitized.erase(std::remove_if(sanitized.begin(), sanitized.end(),
        [](char c) { return c == '\0' || (c > 0 && c < 32 && c != '\t' && c != '\n'); }),
        sanitized.end());
    
    // Trim whitespace
    trim_string(sanitized);
    
    return sanitized;
}

void ShellExecutor::trim_string(std::string& str) const {
    // Trim leading whitespace
    str.erase(str.begin(), std::find_if(str.begin(), str.end(),
        [](int ch) { return !std::isspace(ch); }));
    
    // Trim trailing whitespace
    str.erase(std::find_if(str.rbegin(), str.rend(),
        [](int ch) { return !std::isspace(ch); }).base(), str.end());
}

std::string ShellExecutor::execute_system_command(const std::string& command) {
    std::array<char, 128> buffer;
    std::string result;
    
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    
    if (!pipe) {
        return "Error: Could not execute command";
    }
    
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    
    return result;
}

ShellResult ShellExecutor::execute_command(const std::string& command) {
    std::string sanitized = sanitize_command(command);
    
    if (sanitized.empty()) {
        return ShellResult("Error: Empty command", -1);
    }
    
    if (!is_command_safe(sanitized)) {
        ShellResult result;
        result.output = "Error: Command blocked for security reasons";
        result.exit_code = -1;
        result.success = false;
        result.error_message = "Potentially dangerous command";
        return result;
    }
    
    try {
        std::string output = execute_system_command(sanitized);
        
        // Remove trailing newline if present
        if (!output.empty() && output.back() == '\n') {
            output.pop_back();
        }
        
        if (output.empty()) {
            output = "[No output]";
        }
        
        return ShellResult(output, 0);
    } catch (const std::exception& e) {
        ShellResult result;
        result.output = "Error: " + std::string(e.what());
        result.exit_code = -1;
        result.success = false;
        result.error_message = e.what();
        return result;
    }
}

std::vector<std::string> ShellExecutor::split_output_lines(const std::string& output) const {
    std::vector<std::string> lines;
    std::istringstream iss(output);
    std::string line;
    
    while (std::getline(iss, line)) {
        lines.push_back(line);
    }
    
    if (lines.empty()) {
        lines.push_back("[No output]");
    }
    
    return lines;
}

void ShellExecutor::update_element_output(SlideElement& element, const ShellResult& result) {
    element.executed = true;
    element.shell_output_lines = split_output_lines(result.output);
    element.output_scroll_offset = 0;
    
    // Store error information if needed
    if (!result.success) {
        if (element.shell_output_lines.empty()) {
            element.shell_output_lines.push_back("Command failed: " + result.error_message);
        } else {
            element.shell_output_lines.insert(element.shell_output_lines.begin(),
                "Command failed (exit code " + std::to_string(result.exit_code) + "):");
        }
    }
}

void ShellExecutor::execute_shell_element(SlideElement& element) {
    if (element.type != ElementType::SHELL_COMMAND || element.executed || element.shell_command.empty()) {
        return;
    }
    
    ShellResult result = execute_command(element.shell_command);
    update_element_output(element, result);
}

bool ShellExecutor::has_shell_commands(const Slide& slide) const {
    return std::any_of(slide.begin(), slide.end(),
        [](const SlideElement& element) {
            return element.type == ElementType::SHELL_COMMAND;
        });
}

void ShellExecutor::execute_shell_commands_in_slide(Slide& slide) {
    for (auto& element : slide) {
        if (element.type == ElementType::SHELL_COMMAND && !element.executed) {
            execute_shell_element(element);
        }
    }
}

bool ShellExecutor::can_scroll_up(const SlideElement& element) const {
    return element.output_scroll_offset > 0;
}

bool ShellExecutor::can_scroll_down(const SlideElement& element) const {
    if (!element.executed || element.shell_output_lines.empty()) {
        return false;
    }
    
    int max_scroll = static_cast<int>(element.shell_output_lines.size()) - element.max_output_lines;
    return element.output_scroll_offset < max_scroll;
}

void ShellExecutor::scroll_output_up(SlideElement& element) {
    if (can_scroll_up(element)) {
        element.output_scroll_offset--;
    }
}

void ShellExecutor::scroll_output_down(SlideElement& element) {
    if (can_scroll_down(element)) {
        element.output_scroll_offset++;
    }
}

} // namespace mdslides
