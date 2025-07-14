#include "markdown_parser.hpp"
#include <fstream>
#include <sstream>
#include <regex>
#include <cstdlib>
#include <cstring>
#include <locale.h>
#include <cmark.h>

namespace mdslides {

MarkdownParser::MarkdownParser() {
    utf8_supported_ = detect_utf8_support();
    load_character_replacements();
}

MarkdownParser::~MarkdownParser() = default;

std::vector<Slide> MarkdownParser::parse_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    
    return parse_string(content);
}

std::vector<Slide> MarkdownParser::parse_string(const std::string& content) {
    std::vector<Slide> slides;
    std::stringstream ss(content);
    std::string line;
    std::string slide_content;
    
    while (std::getline(ss, line)) {
        if (line == "---") {
            if (!slide_content.empty()) {
                slides.push_back(parse_slide_content(slide_content));
                slide_content.clear();
            }
        } else {
            slide_content += line + "\n";
        }
    }
    
    // Don't forget the last slide
    if (!slide_content.empty()) {
        slides.push_back(parse_slide_content(slide_content));
    }
    
    return slides;
}

bool MarkdownParser::detect_utf8_support() {
    const char* lang = getenv("LANG");
    const char* lc_all = getenv("LC_ALL");
    const char* lc_ctype = getenv("LC_CTYPE");
    
    if ((lang && (strstr(lang, "UTF-8") || strstr(lang, "utf8"))) ||
        (lc_all && (strstr(lc_all, "UTF-8") || strstr(lc_all, "utf8"))) ||
        (lc_ctype && (strstr(lc_ctype, "UTF-8") || strstr(lc_ctype, "utf8")))) {
        return true;
    }
    
    char* current_locale = setlocale(LC_CTYPE, nullptr);
    if (current_locale && (strstr(current_locale, "UTF-8") || strstr(current_locale, "utf8"))) {
        return true;
    }
    
    return false;
}

void MarkdownParser::load_character_replacements() {
    char_replacements_ = {
        // German umlauts
        {"ä", "ae"}, {"ö", "oe"}, {"ü", "ue"}, 
        {"Ä", "Ae"}, {"Ö", "Oe"}, {"Ü", "Ue"}, 
        {"ß", "ss"},
        
        // Arrows
        {"→", "->"}, {"←", "<-"}, {"↑", "^"}, 
        {"↓", "v"}, {"⇒", "=>"}, {"⇐", "<="},
        
        // Bullets and symbols
        {"•", "*"}, {"◦", "o"}, {"▪", "*"}, 
        {"▫", "o"}, {"★", "*"}, {"☆", "*"},
        {"✓", "v"}, {"✗", "x"}, {"✔", "+"}, 
        {"✘", "x"}, {"⚠", "!"}, {"⚡", "!"},
        
        // French accents
        {"é", "e"}, {"è", "e"}, {"ê", "e"}, 
        {"ë", "e"}, {"à", "a"}, {"â", "a"},
        {"ç", "c"}, {"î", "i"}, {"ï", "i"}, 
        {"ô", "o"}, {"ù", "u"}, {"û", "u"},
        {"É", "E"}, {"È", "E"}, {"Ê", "E"}, 
        {"À", "A"}, {"Ç", "C"},
        
        // Spanish characters
        {"ñ", "n"}, {"Ñ", "N"}, {"í", "i"}, 
        {"ó", "o"}, {"ú", "u"}, {"á", "a"},
        {"Í", "I"}, {"Ó", "O"}, {"Ú", "U"}, 
        {"Á", "A"},
        
        // Mathematical symbols
        {"≈", "~="}, {"≠", "!="}, {"≤", "<="}, 
        {"≥", ">="}, {"∞", "inf"},
        {"π", "pi"}, {"α", "alpha"}, {"β", "beta"}, 
        {"γ", "gamma"}, {"δ", "delta"},
        
        // Quotation marks
        {""", "\""}, {""", "\""}, {"'", "'"}, 
        {"'", "'"}, {"«", "\""}, {"»", "\""},
        
        // Dashes and ellipsis
        {"—", "--"}, {"–", "-"}, {"…", "..."},
        
        // Currency and other symbols
        {"£", "GBP"}, {"€", "EUR"}, {"¥", "YEN"}, 
        {"©", "(c)"}, {"®", "(R)"},
        {"™", "(TM)"}, {"°", "deg"}, {"±", "+/-"}, 
        {"×", "x"}, {"÷", "/"}
    };
}

std::string MarkdownParser::apply_character_replacements(const std::string& text) const {
    if (utf8_supported_) {
        return text;
    }
    
    std::string result = text;
    for (const auto& [search, replace] : char_replacements_) {
        size_t pos = 0;
        while ((pos = result.find(search, pos)) != std::string::npos) {
            result.replace(pos, search.length(), replace);
            pos += replace.length();
        }
    }
    
    // Additional fallback for any remaining non-ASCII characters
    std::string ascii_safe;
    for (size_t i = 0; i < result.length(); ++i) {
        unsigned char c = static_cast<unsigned char>(result[i]);
        if (c < 128) {
            ascii_safe += c;
        } else {
            // Skip UTF-8 continuation bytes and replace with ?
            if (c >= 0xC0 && c <= 0xDF && i + 1 < result.length()) {
                ascii_safe += "?"; i++; // 2-byte UTF-8
            } else if (c >= 0xE0 && c <= 0xEF && i + 2 < result.length()) {
                ascii_safe += "?"; i += 2; // 3-byte UTF-8
            } else if (c >= 0xF0 && c <= 0xF7 && i + 3 < result.length()) {
                ascii_safe += "?"; i += 3; // 4-byte UTF-8
            } else {
                ascii_safe += "?";
            }
        }
    }
    
    return ascii_safe;
}

Slide MarkdownParser::parse_slide_content(const std::string& content) {
    Slide slide;
    std::stringstream ss(content);
    std::string line;
    int line_number = 0;
    
    bool in_code_block = false;
    bool in_shell_block = false;
    
    while (std::getline(ss, line)) {
        line_number++;
        
        if (line.empty()) {
            continue;
        }
        
        // Handle shell command blocks
        if (line.substr(0, 4) == "```$") {
            if (!in_shell_block) {
                in_shell_block = true;
                std::string command = line.substr(4);
                
                // Create shell command element
                SlideElement cmd_element("$ " + command, ElementType::SHELL_COMMAND);
                cmd_element.shell_command = command;
                cmd_element.is_bold = true;
                cmd_element.animation = AnimationType::TYPEWRITER;
                cmd_element.delay_ms = line_number * 50;
                slide.push_back(cmd_element);
                
                // Create output placeholder
                SlideElement output_element("[Press ENTER to execute]", ElementType::SHELL_OUTPUT);
                output_element.animation = AnimationType::NONE;
                slide.push_back(output_element);
            } else {
                in_shell_block = false;
            }
            continue;
        }
        
        if (in_shell_block) {
            continue;
        }
        
        // Handle regular code blocks
        if (line.substr(0, 3) == "```") {
            in_code_block = !in_code_block;
            continue;
        }
        
        if (in_code_block) {
            SlideElement element("    " + line, ElementType::CODE_BLOCK);
            element.animation = AnimationType::TYPEWRITER;
            element.delay_ms = line_number * 50;
            slide.push_back(element);
            continue;
        }
        
        // Parse regular markdown line
        SlideElement element = parse_line(line, line_number);
        element.content = apply_character_replacements(element.content);
        slide.push_back(element);
    }
    
    return slide;
}

SlideElement MarkdownParser::parse_line(const std::string& line, int line_number) {
    ElementType type = detect_element_type(line);
    std::string content = extract_content(line, type);
    
    SlideElement element(content, type);
    element.animation = get_default_animation(type);
    element.delay_ms = get_default_delay(type, line_number);
    
    // Set bold flag for headers
    if (type == ElementType::HEADER1 || type == ElementType::HEADER2 || type == ElementType::HEADER3) {
        element.is_bold = true;
    }
    
    // Process bold text markup
    if (content.find("**") != std::string::npos) {
        element.content = process_bold_text(content);
        element.is_bold = true;
    }
    
    return element;
}

ElementType MarkdownParser::detect_element_type(const std::string& line) {
    if (line.substr(0, 2) == "# ") {
        return ElementType::HEADER1;
    } else if (line.substr(0, 3) == "## ") {
        return ElementType::HEADER2;
    } else if (line.substr(0, 4) == "### ") {
        return ElementType::HEADER3;
    } else if (line.substr(0, 2) == "- ") {
        return ElementType::BULLET;
    } else if (std::regex_match(line, std::regex(R"(^\d+\. .*)"))) {
        return ElementType::NUMBERED;
    } else {
        return ElementType::TEXT;
    }
}

std::string MarkdownParser::extract_content(const std::string& line, ElementType type) {
    switch (type) {
        case ElementType::HEADER1:
            return line.substr(2);
        case ElementType::HEADER2:
            return line.substr(3);
        case ElementType::HEADER3:
            return line.substr(4);
        case ElementType::BULLET: {
            std::string bullet = utf8_supported_ ? "• " : "* ";
            return bullet + line.substr(2);
        }
        case ElementType::NUMBERED:
        case ElementType::TEXT:
        default:
            return line;
    }
}

std::string MarkdownParser::process_bold_text(const std::string& text) {
    return std::regex_replace(text, std::regex(R"(\*\*(.*?)\*\*)"), "$1");
}

AnimationType MarkdownParser::get_default_animation(ElementType type) {
    switch (type) {
        case ElementType::HEADER1:
        case ElementType::HEADER2:
            return AnimationType::SLIDE_IN;
        case ElementType::BULLET:
        case ElementType::NUMBERED:
            return AnimationType::SLIDE_IN;
        case ElementType::CODE_BLOCK:
            return AnimationType::TYPEWRITER;
        default:
            return AnimationType::FADE_IN;
    }
}

int MarkdownParser::get_default_delay(ElementType type, int line_number) {
    int base_delay = line_number * 50;
    
    switch (type) {
        case ElementType::HEADER1:
            return base_delay + 200;
        case ElementType::HEADER2:
            return base_delay + 100;
        default:
            return base_delay;
    }
}

} // namespace mdslides
