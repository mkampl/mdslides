#include "markdown_parser.hh"
#include <ncurses.h>
#include <fstream>
#include <sstream>
#include <regex>
#include <locale.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <cmark-gfm.h>

class CMarkSlideParser
{
private:
    std::vector<SlideElement> &elements;
    int current_y;
    bool utf8_supported;

public:
    CMarkSlideParser(std::vector<SlideElement> &elems, bool utf8_support)
        : elements(elems), current_y(3), utf8_supported(utf8_support) {}

    // Main entry point - process document in order
    void parseDocument(cmark_node *document)
    {
        if (cmark_node_get_type(document) == CMARK_NODE_DOCUMENT)
        {
            // Process all top-level children in order
            cmark_node *child;
            for (child = cmark_node_first_child(document); child; child = cmark_node_next(child))
            {
                processTopLevelNode(child);
            }
        }
    }

private:
    void processTopLevelNode(cmark_node *node)
    {
        cmark_node_type type = cmark_node_get_type(node);

        switch (type)
        {
        case CMARK_NODE_HEADING:
            processHeading(node);
            break;

        case CMARK_NODE_PARAGRAPH:
            processParagraph(node);
            break;

        case CMARK_NODE_CODE_BLOCK:
            processCodeBlock(node);
            break;

        case CMARK_NODE_LIST:
            processList(node);
            break;

        default:
            // For other node types, process recursively
            cmark_node *child;
            for (child = cmark_node_first_child(node); child; child = cmark_node_next(child))
            {
                processTopLevelNode(child);
            }
            break;
        }
    }

    void processHeading(cmark_node *node)
    {
        int level = cmark_node_get_heading_level(node);
        std::string content = getTextContent(node);

        SlideElement element;
        element.y = current_y++;
        element.x = 2;
        element.content = content;
        element.is_bold = true;
        element.animation = AnimationType::SLIDE_IN;

        current_y++; // spacing

        switch (level)
        {
        case 1:
            element.color_pair = 1;
            element.type = ElementType::HEADER1;
            element.x = std::max((COLS - (int)content.length()) / 2, 2);
            break;
        case 2:
            element.color_pair = 2;
            element.type = ElementType::HEADER2;
            break;
        case 3:
            element.color_pair = 4;
            element.type = ElementType::HEADER3;
            break;
        default:
            element.color_pair = 4;
            element.type = ElementType::HEADER3;
            break;
        }

        elements.push_back(element);
    }

    void processParagraph(cmark_node *node)
    {
        std::string content;
        bool is_bold = false;

        // Check if this paragraph contains bold text
        extractTextWithFormatting(node, content, is_bold);

        if (!content.empty())
        {
            SlideElement element;
            element.y = current_y++;
            element.x = 2;
            element.content = content;
            element.color_pair = is_bold ? 4 : 3; // Use different color for bold
            element.is_bold = is_bold;
            element.type = ElementType::TEXT;
            elements.push_back(element);
        }
    }

    void processCodeBlock(cmark_node *node)
    {
        const char *info = cmark_node_get_fence_info(node);
        const char *literal = cmark_node_get_literal(node);

        // Check if it's a shell command
        if (info && strlen(info) > 0 && info[0] == '$')
        {
            std::string command = std::string(info + 1); // Skip the $

            SlideElement element;
            element.y = current_y++;
            element.x = 4;
            element.content = "    $ " + command;
            element.color_pair = 6; // Same as regular code
            element.type = ElementType::SHELL_COMMAND;
            element.shell_command = command;
            element.animation = AnimationType::TYPEWRITER;
            elements.push_back(element);
        }
        else
        {
            // Regular code block
            std::string code = literal ? literal : "";
            std::istringstream iss(code);
            std::string line;

            while (std::getline(iss, line))
            {
                SlideElement element;
                element.y = current_y++;
                element.x = 4;
                element.content = "    " + line;
                element.color_pair = 6;
                element.type = ElementType::CODE_BLOCK;
                element.animation = AnimationType::TYPEWRITER;
                elements.push_back(element);
            }
        }
    }

    void processList(cmark_node *node)
    {
        // Process list items
        cmark_node *child;
        for (child = cmark_node_first_child(node); child; child = cmark_node_next(child))
        {
            if (cmark_node_get_type(child) == CMARK_NODE_ITEM)
            {
                std::string content = getTextContent(child);
                std::string bullet = utf8_supported ? "• " : "* ";

                SlideElement element;
                element.y = current_y++;
                element.x = 4;
                element.content = bullet + content;
                element.color_pair = 3;
                element.type = ElementType::BULLET;
                element.animation = AnimationType::SLIDE_IN;
                elements.push_back(element);
            }
        }
    }

    void extractTextWithFormatting(cmark_node *node, std::string &content, bool &is_bold)
    {
        cmark_node_type type = cmark_node_get_type(node);

        if (type == CMARK_NODE_TEXT)
        {
            const char *literal = cmark_node_get_literal(node);
            if (literal)
            {
                content += literal;
            }
        }
        else if (type == CMARK_NODE_STRONG)
        {
            is_bold = true;
            // Process children of the strong node
            cmark_node *child;
            for (child = cmark_node_first_child(node); child; child = cmark_node_next(child))
            {
                extractTextWithFormatting(child, content, is_bold);
            }
        }
        else
        {
            // Process children for other node types
            cmark_node *child;
            for (child = cmark_node_first_child(node); child; child = cmark_node_next(child))
            {
                extractTextWithFormatting(child, content, is_bold);
            }
        }
    }

    std::string getTextContent(cmark_node *node)
    {
        std::string result;

        if (cmark_node_get_type(node) == CMARK_NODE_TEXT)
        {
            const char *literal = cmark_node_get_literal(node);
            if (literal)
            {
                result = literal;
            }
        }
        else
        {
            cmark_node *child;
            for (child = cmark_node_first_child(node); child; child = cmark_node_next(child))
            {
                result += getTextContent(child);
            }
        }

        return result;
    }
};

MarkdownParser::MarkdownParser() : utf8_supported(false)
{
    utf8_supported = detect_utf8_support();
    load_char_replacements();
}

bool MarkdownParser::detect_utf8_support()
{
    // Check environment variables for UTF-8 support
    const char *lang = getenv("LANG");
    const char *lc_all = getenv("LC_ALL");
    const char *lc_ctype = getenv("LC_CTYPE");

    if ((lang && (strstr(lang, "UTF-8") || strstr(lang, "utf8"))) ||
        (lc_all && (strstr(lc_all, "UTF-8") || strstr(lc_all, "utf8"))) ||
        (lc_ctype && (strstr(lc_ctype, "UTF-8") || strstr(lc_ctype, "utf8"))))
    {
        return true;
    }

    // Also check if the current locale supports UTF-8
    char *current_locale = setlocale(LC_CTYPE, nullptr);
    if (current_locale && (strstr(current_locale, "UTF-8") || strstr(current_locale, "utf8")))
    {
        return true;
    }

    return false;
}

void MarkdownParser::load_char_replacements()
{
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
        std::make_pair("⅓", "1/3"), std::make_pair("⅔", "2/3")};
}

void MarkdownParser::set_utf8_support(bool enabled)
{
    utf8_supported = enabled;
}

void MarkdownParser::load_slides(const std::string &filename, SlideCollection &slides)
{
    slides.clear();

    std::ifstream file(filename);
    std::string line, slide_content;

    while (std::getline(file, line))
    {
        if (line == "---")
        {
            if (!slide_content.empty())
            {
                parse_slide(slide_content, slides);
                slide_content.clear();
            }
        }
        else
        {
            slide_content += line + "\n";
        }
    }
    if (!slide_content.empty())
    {
        parse_slide(slide_content, slides);
    }
}

void MarkdownParser::parse_slide_with_cmark(const std::string &content, SlideCollection &slides)
{
    // Create parser with basic options (no extensions)
    cmark_parser *parser = cmark_parser_new(CMARK_OPT_DEFAULT);

    if (!parser)
    {
        throw std::runtime_error("Failed to create cmark parser");
    }

    // Parse markdown
    cmark_parser_feed(parser, content.c_str(), content.length());
    cmark_node *document = cmark_parser_finish(parser);

    if (!document)
    {
        cmark_parser_free(parser);
        throw std::runtime_error("Failed to parse markdown");
    }

    // Convert to slide elements using the fixed parser
    std::vector<SlideElement> elements;
    CMarkSlideParser slide_parser(elements, utf8_supported);
    slide_parser.parseDocument(document);

    // Cleanup
    cmark_node_free(document);
    cmark_parser_free(parser);

    // Add to slides
    slides.add_slide(elements);
}

// Updated parse_slide method
void MarkdownParser::parse_slide(const std::string &content, SlideCollection &slides)
{
    parse_slide_with_cmark(content, slides);
}
