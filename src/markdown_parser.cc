#include "markdown_parser.hh"
#include <ncurses.h>
#include <fstream>
#include <sstream>
#include <regex>
#include <locale.h>
#include <cstdlib>
#include <cstring>

MarkdownParser::MarkdownParser() : utf8_supported(false) {
    utf8_supported = detect_utf8_support();
    load_char_replacements();
}

bool MarkdownParser::detect_utf8_support() {
    // Check environment variables for UTF-8 support
    const char* lang = getenv("LANG");
    const char* lc_all = getenv("LC_ALL");
    const char* lc_ctype = getenv("LC_CTYPE");
    
    if ((lang && (strstr(lang, "UTF-8") || strstr(lang, "utf8"))) ||
        (lc_all && (strstr(lc_all, "UTF-8") || strstr(lc_all, "utf8"))) ||
        (lc_ctype && (strstr(lc_ctype, "UTF-8") || strstr(lc_ctype, "utf8")))) {
        return true;
    }
    
    // Also check if the current locale supports UTF-8
    char* current_locale = setlocale(LC_CTYPE, nullptr);
    if (current_locale && (strstr(current_locale, "UTF-8") || strstr(current_locale, "utf8"))) {
        return true;
    }
    
    return false;
}

void MarkdownParser::load_char_replacements() {
    char_replacements.clear();
    
    // Built-in replacements for common Unicode characters (used when UTF-8 not supported)
    char_replacements = {
        // German umlauts
        std::make_pair("ä", "ae"), std::make_pair("ö", "oe"), std::make_pair("ü", "ue"), 
        std::make_pair("Ä", "Ae"), std::make_pair("Ö", "Oe"), std::make_pair("Ü", "Ue"), 
        std::make_pair("ß", "ss"),
        // Arrows
        std::make_pair("→", "->"), std::make_pair("←", "<-"), std::make_pair("↑", "^"), 
        std::make_pair("↓", "v"), std::make_pair("⇒", "=>"), std::make_pair("⇐", "<="),
        // Bullets and symbols
        std::make_pair("•", "*"), std::make_pair("◦", "o"), std::make_pair("▪", "*"), 
        std::make_pair("▫", "o"), std::make_pair("★", "*"), std::make_pair("☆", "*"),
        std::make_pair("✓", "v"), std::make_pair("✗", "x"), std::make_pair("✔", "+"), 
        std::make_pair("✘", "x"), std::make_pair("⚠", "!"), std::make_pair("⚡", "!"),
        // French accents
        std::make_pair("é", "e"), std::make_pair("è", "e"), std::make_pair("ê", "e"), 
        std::make_pair("ë", "e"), std::make_pair("à", "a"), std::make_pair("â", "a"),
        std::make_pair("ç", "c"), std::make_pair("î", "i"), std::make_pair("ï", "i"), 
        std::make_pair("ô", "o"), std::make_pair("ù", "u"), std::make_pair("û", "u"),
        std::make_pair("É", "E"), std::make_pair("È", "E"), std::make_pair("Ê", "E"), 
        std::make_pair("À", "A"), std::make_pair("Ç", "C"),
        // Spanish characters
        std::make_pair("ñ", "n"), std::make_pair("Ñ", "N"), std::make_pair("í", "i"), 
        std::make_pair("ó", "o"), std::make_pair("ú", "u"), std::make_pair("á", "a"),
        std::make_pair("Í", "I"), std::make_pair("Ó", "O"), std::make_pair("Ú", "U"), 
        std::make_pair("Á", "A"),
        // Other common characters
        std::make_pair("£", "GBP"), std::make_pair("€", "EUR"), std::make_pair("¥", "YEN"), 
        std::make_pair("©", "(c)"), std::make_pair("®", "(R)"),
        std::make_pair("™", "(TM)"), std::make_pair("°", "deg"), std::make_pair("±", "+/-"), 
        std::make_pair("×", "x"), std::make_pair("÷", "/"),
        // Mathematical symbols
        std::make_pair("≈", "~="), std::make_pair("≠", "!="), std::make_pair("≤", "<="), 
        std::make_pair("≥", ">="), std::make_pair("∞", "inf"),
        std::make_pair("π", "pi"), std::make_pair("α", "alpha"), std::make_pair("β", "beta"), 
        std::make_pair("γ", "gamma"), std::make_pair("δ", "delta"),
        // Quotation marks
        std::make_pair("\u201C", "\""), std::make_pair("\u201D", "\""), std::make_pair("'", "'"), 
        std::make_pair("'", "'"), std::make_pair("«", "\""), std::make_pair("»", "\""),
        // Dashes
        std::make_pair("—", "--"), std::make_pair("–", "-"), std::make_pair("…", "..."),
        // Various symbols
        std::make_pair("§", "S"), std::make_pair("¶", "P"), std::make_pair("†", "+"), 
        std::make_pair("‡", "++"), std::make_pair("‰", "0/00"),
        std::make_pair("⁰", "0"), std::make_pair("¹", "1"), std::make_pair("²", "2"), 
        std::make_pair("³", "3"), std::make_pair("⁴", "4"), std::make_pair("⁵", "5"),
        std::make_pair("½", "1/2"), std::make_pair("¼", "1/4"), std::make_pair("¾", "3/4"), 
        std::make_pair("⅓", "1/3"), std::make_pair("⅔", "2/3")
    };
}

void MarkdownParser::set_utf8_support(bool enabled) {
    utf8_supported = enabled;
}

void MarkdownParser::load_slides(const std::string& filename, SlideCollection& slides) {
    slides.clear();
    
    std::ifstream file(filename);
    std::string line, slide_content;
    
    while (std::getline(file, line)) {
        if (line == "---") {
            if (!slide_content.empty()) {
                parse_slide(slide_content, slides);
                slide_content.clear();
            }
        } else {
            slide_content += line + "\n";
        }
    }
    if (!slide_content.empty()) {
        parse_slide(slide_content, slides);
    }
}

void MarkdownParser::parse_slide(const std::string& content, SlideCollection& slides) {
    std::vector<SlideElement> elements;
    std::istringstream iss(content);
    std::string line;
    int y = 3;
    bool in_code_block = false;
    bool in_shell_block = false;
    
    while (std::getline(iss, line)) {
        if (line.empty()) { y++; continue; }
        
        SlideElement element;
        element.y = y;
        element.x = 2;
        element.delay_ms = y * 50;
        
        // Shell block detection
        if (line.substr(0, 4) == "```$") {
            in_shell_block = !in_shell_block;
            if (in_shell_block) {
                std::string command = line.substr(4);
                
                // Shell command element
                element.content = "$ " + command;
                element.color_pair = 7;
                element.is_bold = true;
                element.x = 4;
                element.type = ElementType::SHELL_COMMAND;
                element.shell_command = command;
                element.animation = AnimationType::TYPEWRITER;
                elements.push_back(element);
                y++;
                
                // Shell output placeholder
                element.y = y;
                element.content = "[Press ENTER to execute]";
                element.color_pair = 8;
                element.is_bold = false;
                element.type = ElementType::SHELL_OUTPUT;
                element.animation = AnimationType::NONE;
                elements.push_back(element);
                y += 7;
            }
            continue;
        }
        
        if (in_shell_block) continue;
        
        // Regular code block
        if (line.substr(0, 3) == "```") {
            in_code_block = !in_code_block;
            y++;
            continue;
        }
        
        if (in_code_block) {
            element.content = "    " + line;
            element.color_pair = 6;
            element.x = 4;
            element.type = ElementType::CODE_BLOCK;
            element.animation = AnimationType::TYPEWRITER;
        }
        // Headers
        else if (line.substr(0, 2) == "# ") {
            element.content = line.substr(2);
            element.color_pair = 1;
            element.is_bold = true;
            element.type = ElementType::HEADER1;
            element.x = std::max((COLS - (int)element.content.length()) / 2, 2);
            element.animation = AnimationType::SLIDE_IN;
        }
        else if (line.substr(0, 3) == "## ") {
            element.content = line.substr(3);
            element.color_pair = 2;
            element.is_bold = true;
            element.type = ElementType::HEADER2;
            element.animation = AnimationType::SLIDE_IN;
        }
        else if (line.substr(0, 4) == "### ") {
            element.content = line.substr(4);
            element.color_pair = 4;
            element.is_bold = true;
            element.type = ElementType::HEADER3;
        }
        // Lists - preserve Unicode bullet if supported, otherwise use ASCII
        else if (line.substr(0, 2) == "- ") {
            std::string bullet = utf8_supported ? "• " : "* ";
            element.content = bullet + line.substr(2);
            element.color_pair = 3;
            element.x = 4;
            element.type = ElementType::BULLET;
            element.animation = AnimationType::SLIDE_IN;
        }
        else if (std::regex_match(line, std::regex(R"(^\d+\. .*)"))) {
            element.content = line;
            element.color_pair = 3;
            element.x = 4;
            element.type = ElementType::NUMBERED;
            element.animation = AnimationType::SLIDE_IN;
        }
        // Bold text
        else if (line.find("**") != std::string::npos) {
            element.content = std::regex_replace(line, std::regex(R"(\*\*(.*?)\*\*)"), "$1");
            element.color_pair = 4;
            element.is_bold = true;
        }
        // Regular text
        else {
            element.content = line;
            element.color_pair = 3;
        }
        
        elements.push_back(element);
        y++;
    }
    
    slides.add_slide(elements);
}
