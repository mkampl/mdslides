#pragma once

#include "slide_element.hh"
#include <string>
#include <vector>

class MarkdownParser {
public:
    MarkdownParser();
    void load_slides(const std::string& filename, SlideCollection& slides);
    void set_utf8_support(bool enabled);
    
private:
    void parse_slide(const std::string& content, SlideCollection& slides);
    void load_char_replacements();
    bool detect_utf8_support();
    
    std::vector<std::pair<std::string, std::string>> char_replacements;
    bool utf8_supported;
};
