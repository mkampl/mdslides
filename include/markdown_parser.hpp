#pragma once

#include "slide_types.hpp"
#include <string>
#include <vector>

namespace mdslides {

class MarkdownParser {
public:
    MarkdownParser();
    ~MarkdownParser();
    
    // Main parsing functions
    std::vector<Slide> parse_file(const std::string& filename);
    std::vector<Slide> parse_string(const std::string& content);
    
    // UTF-8 support
    void set_utf8_support(bool enabled) { utf8_supported_ = enabled; }
    bool is_utf8_supported() const { return utf8_supported_; }
    
    // Character replacement for ASCII fallback
    std::string apply_character_replacements(const std::string& text) const;
    
private:
    bool utf8_supported_ = false;
    std::vector<std::pair<std::string, std::string>> char_replacements_;
    
    // Private parsing methods
    void load_character_replacements();
    bool detect_utf8_support();
    
    // Slide parsing
    Slide parse_slide_content(const std::string& content);
    SlideElement parse_line(const std::string& line, int line_number);
    
    // Element type detection
    ElementType detect_element_type(const std::string& line);
    std::string extract_content(const std::string& line, ElementType type);
    
    // Shell command handling
    bool is_shell_command_block(const std::string& line);
    std::string extract_shell_command(const std::string& line);
    
    // Markdown processing
    std::string process_bold_text(const std::string& text);
    std::string clean_markdown_syntax(const std::string& text);
    
    // Animation assignment
    AnimationType get_default_animation(ElementType type);
    int get_default_delay(ElementType type, int line_number);
};

} // namespace mdslides
