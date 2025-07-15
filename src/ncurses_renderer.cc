#include "ncurses_renderer.hh"
#include <ncurses.h>
#include <algorithm>
#include <thread>
#include <chrono>
#include <locale.h>
#include <cstdlib>
#include <cstring>

NCursesRenderer::NCursesRenderer() : utf8_supported(false) {
    utf8_supported = detect_utf8_support();
    load_char_replacements();
}

NCursesRenderer::~NCursesRenderer() {
    cleanup();
}

void NCursesRenderer::initialize() {
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    if (has_colors()) {
        start_color();
    }
    
    theme_manager.setup_theme(Theme::DARK);
}

void NCursesRenderer::cleanup() {
    endwin();
}

// In NCursesRenderer, add a helper method:
bool NCursesRenderer::check_for_input_during_animation() {
    // Set nodelay mode to make getch() non-blocking
    nodelay(stdscr, TRUE);
    int ch = getch();
    nodelay(stdscr, FALSE);
    
    if (ch != ERR) {
        // Put the character back in the input buffer
        ungetch(ch);
        return true;  // Input detected
    }
    return false;  // No input
}

void NCursesRenderer::render_slide(const std::vector<SlideElement>& elements, bool animated) {
    attron(COLOR_PAIR(0));
    for (int y = 2; y < LINES - 3; ++y) {  // Skip header (0-1) and footer (LINES-3 to LINES-1)
        for (int x = 0; x < COLS; ++x) {
            mvaddch(y, x, ' ');
        }
    }
    attroff(COLOR_PAIR(0));
    
    if (animated) {
        for (const auto& element : elements) {
            if (element.type != ElementType::SHELL_OUTPUT) {
                std::this_thread::sleep_for(std::chrono::milliseconds(element.delay_ms));
                render_element_animated(element);
            }
        }
    } else {
        for (const auto& element : elements) {
            if (element.type != ElementType::SHELL_OUTPUT) {
                render_element_instant(element);
            }
        }
    }
    
    // Render executed shell outputs
    for (const auto& element : elements) {
        if (element.type == ElementType::SHELL_COMMAND && element.executed) {
            update_shell_output(element);
        }
    }
    
    refresh();
}

void NCursesRenderer::clear_screen() {
    clear_with_background();
}

void NCursesRenderer::draw_header(int current_slide, int total_slides, const std::string& theme_name, 
                                 bool show_timer, int minutes, int seconds, bool utf8_mode) {
    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(0, 2, "Slide %d/%d", current_slide + 1, total_slides);
    
    // Show UTF-8 mode indicator
    std::string mode_indicator = utf8_mode ? "UTF-8" : "ASCII";
    mvprintw(0, COLS - 25, "Mode: %s", mode_indicator.c_str());
    mvprintw(0, COLS - 15, "Theme: %s", theme_name.c_str());
    
    if (show_timer) {
        mvprintw(0, COLS - 45, "Time: %02d:%02d", minutes, seconds);
    }
    
    attroff(COLOR_PAIR(1) | A_BOLD);
    
    attron(COLOR_PAIR(4));
    mvhline(1, 0, '-', COLS);
    attroff(COLOR_PAIR(4));
}

void NCursesRenderer::draw_footer() {
    attron(COLOR_PAIR(4));
    mvhline(LINES-2, 0, '-', COLS);
    attroff(COLOR_PAIR(4));
    
    attron(COLOR_PAIR(3));
    mvprintw(LINES-1, 2, "Controls: <-/-> Navigate | ENTER Execute | u/d Scroll | 't' Theme | 'h' Help | 'q' Quit");
    attroff(COLOR_PAIR(3));
}

void NCursesRenderer::draw_progress_bar(int current_slide, int total_slides) {
    if (total_slides == 0) return;
    
    int progress_width = (current_slide * (COLS - 4)) / total_slides;
    
    attron(COLOR_PAIR(4));
    mvprintw(LINES-3, 2, "[");
    mvprintw(LINES-3, COLS-3, "]");
    
    attron(COLOR_PAIR(1) | A_BOLD);
    for (int i = 0; i < progress_width; ++i) {
        mvprintw(LINES-3, 3 + i, "#");
    }
    attroff(COLOR_PAIR(1) | A_BOLD);
    attroff(COLOR_PAIR(4));
}

void NCursesRenderer::show_help(bool utf8_supported) {
    clear_with_background();
    
    const char* help_text[] = {
        "MARKDOWN SLIDE PRESENTER - HELP",
        "",
        "Navigation:",
        "  -> / Space / l    Next slide",
        "  <- / Backspace / h Previous slide", 
        "  g                Go to specific slide",
        "  Home / 0         First slide",
        "  End / $          Last slide",
        "  ENTER            Execute shell commands",
        "  u / d            Scroll shell output up/down",
        "",
        "Display:",
        "  t                Cycle themes",
        "  a                Toggle animations", 
        "  T                Toggle timer",
        "  r                Refresh/redraw",
        "",
        "Other:",
        "  h                Show this help",
        "  q / Escape       Quit",
        "",
        "Supported Markdown:",
        "  # H1 Headers     ## H2 Headers    ### H3 Headers",
        "  - Bullet points  1. Numbered lists **Bold text**",
        "  ```code blocks```  ```$shell command```",
        "",
        "Unicode Support:",
        utf8_supported ? "  UTF-8 mode: Unicode characters displayed natively" : "  ASCII mode: Unicode characters replaced with ASCII equivalents"
    };
    
    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(2, 2, "%s", help_text[0]);
    attroff(COLOR_PAIR(1) | A_BOLD);
    
    attron(COLOR_PAIR(3));
    for (size_t i = 1; i < sizeof(help_text)/sizeof(help_text[0]); ++i) {
        mvprintw(3 + i, 2, "%s", help_text[i]);
    }
    attroff(COLOR_PAIR(3));
    
    attron(COLOR_PAIR(4) | A_BOLD);
    mvprintw(LINES-2, 2, "Press any key to continue...");
    attroff(COLOR_PAIR(4) | A_BOLD);
    
    refresh();
}

void NCursesRenderer::show_message(const std::string& message, int y) {
    if (y == -1) y = LINES - 4;
    
    attron(COLOR_PAIR(4) | A_BOLD);
    mvprintw(y, 2, "%s", message.c_str());
    attroff(COLOR_PAIR(4) | A_BOLD);
    refresh();
}

void NCursesRenderer::clear_message_area() {
    mvprintw(LINES-4, 2, "%*s", COLS-4, "");
    refresh();
}

int NCursesRenderer::get_input() {
    return getch();
}

int NCursesRenderer::get_screen_width() const {
    return COLS;
}

int NCursesRenderer::get_screen_height() const {
    return LINES;
}

void NCursesRenderer::enable_echo() {
    echo();
}

void NCursesRenderer::disable_echo() {
    noecho();
}

void NCursesRenderer::get_string(char* buffer, int max_length) {
    getnstr(buffer, max_length - 1);
}

void NCursesRenderer::apply_theme(Theme theme) {
    theme_manager.setup_theme(theme);
}

void NCursesRenderer::update_shell_output(const SlideElement& shell_element) {
    int output_y = shell_element.y + 1;
    int lines_to_show = std::min((int)shell_element.shell_output_lines.size(), shell_element.max_output_lines);

    // Clear old output WITH background color
    attron(COLOR_PAIR(0));
    for (int i = 0; i < 10; ++i) {
        for (int x = 0; x < COLS; ++x) {
            mvaddch(output_y + i, x, ' ');
        }
    }
    attroff(COLOR_PAIR(0));

    // Display output lines
    for (int i = 0; i < lines_to_show; ++i) {
        int line_idx = shell_element.output_scroll_offset + i;
        if (line_idx < (int)shell_element.shell_output_lines.size()) {
            attron(COLOR_PAIR(8));
            safe_mvprintw(output_y + i, shell_element.x + 2, shell_element.shell_output_lines[line_idx]);
            attroff(COLOR_PAIR(8));
        }
    }
    
    // Show scroll indicators if needed
    if ((int)shell_element.shell_output_lines.size() > shell_element.max_output_lines) {
        attron(COLOR_PAIR(4) | A_BOLD);
        
        std::string scroll_info = "(" + std::to_string(shell_element.output_scroll_offset + 1) + 
                                "-" + std::to_string(std::min(shell_element.output_scroll_offset + lines_to_show, 
                                                             (int)shell_element.shell_output_lines.size())) +
                                "/" + std::to_string(shell_element.shell_output_lines.size()) + " lines)";
        
        mvprintw(output_y + lines_to_show, shell_element.x, "%s", scroll_info.c_str());
        
        if (shell_element.output_scroll_offset > 0) {
            mvprintw(output_y + lines_to_show + 1, shell_element.x, "Press 'u' for up");
        }
        if (shell_element.output_scroll_offset + lines_to_show < (int)shell_element.shell_output_lines.size()) {
            mvprintw(output_y + lines_to_show + 1, shell_element.x + 20, "Press 'd' for down");
        }
        
        attroff(COLOR_PAIR(4) | A_BOLD);
    }
}

void NCursesRenderer::refresh_display() {
    refresh();
}

void NCursesRenderer::sleep_ms(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void NCursesRenderer::set_utf8_support(bool enabled) {
    utf8_supported = enabled;
}

bool NCursesRenderer::detect_utf8_support() {
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

void NCursesRenderer::load_char_replacements() {
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

void NCursesRenderer::safe_mvprintw(int y, int x, const std::string& text) {
    std::string output_text = text;
    
    // If UTF-8 is not supported, apply character replacements
    if (!utf8_supported) {
        for (const auto& [search, replace] : char_replacements) {
            size_t pos = 0;
            while ((pos = output_text.find(search, pos)) != std::string::npos) {
                output_text.replace(pos, search.length(), replace);
                pos += replace.length();
            }
        }
        
        // Additional fallback for any remaining non-ASCII characters
        std::string ascii_safe;
        for (size_t i = 0; i < output_text.length(); ++i) {
            unsigned char c = static_cast<unsigned char>(output_text[i]);
            if (c < 128) {
                ascii_safe += c;
            } else {
                // Skip UTF-8 continuation bytes and replace with ?
                if (c >= 0xC0 && c <= 0xDF && i + 1 < output_text.length()) {
                    ascii_safe += "?"; i++; // 2-byte UTF-8
                } else if (c >= 0xE0 && c <= 0xEF && i + 2 < output_text.length()) {
                    ascii_safe += "?"; i += 2; // 3-byte UTF-8
                } else if (c >= 0xF0 && c <= 0xF7 && i + 3 < output_text.length()) {
                    ascii_safe += "?"; i += 3; // 4-byte UTF-8
                } else {
                    ascii_safe += "?"; // Other problematic character
                }
            }
        }
        output_text = ascii_safe;
    }
    
    mvprintw(y, x, "%s", output_text.c_str());
}

void NCursesRenderer::render_element_animated(const SlideElement& element) {
    int attrs = COLOR_PAIR(element.color_pair);
    if (element.is_bold) attrs |= A_BOLD;
    if (element.type == ElementType::SHELL_OUTPUT && !element.executed) attrs |= A_DIM;
    
    switch (element.animation) {
        case AnimationType::TYPEWRITER: {
            attron(attrs);
            for (size_t i = 0; i <= element.content.length(); ++i) {
                safe_mvprintw(element.y, element.x, element.content.substr(0, i));
                refresh();
                // Check for input before sleeping
        if (check_for_input_during_animation()) {
            // Skip to end of animation
            safe_mvprintw(element.y, element.x, element.content);
            refresh();
            break;
        }
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
            }
            attroff(attrs);
            break;
        }
            
        case AnimationType::SLIDE_IN: {
            attron(attrs);
            int start_x = element.x + element.content.length() + 10;
            for (int x = start_x; x >= element.x; x -= 3) {
                // Clear line with background color
                attron(COLOR_PAIR(0));
                for (int col = 0; col < COLS; ++col) {
                    mvaddch(element.y, col, ' ');
                }
                attroff(COLOR_PAIR(0));

                attron(attrs);
                safe_mvprintw(element.y, std::max(x, element.x), element.content);
                attroff(attrs);
                refresh();
                // Check for input before sleeping
        if (check_for_input_during_animation()) {
            // Skip to end of animation
            safe_mvprintw(element.y, element.x, element.content);
            refresh();
            break;
        }
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
            }
            // Final clear and print
            attron(COLOR_PAIR(0));
            for (int col = 0; col < COLS; ++col) {
                mvaddch(element.y, col, ' ');
            }
            attroff(COLOR_PAIR(0));

            attron(attrs);
            safe_mvprintw(element.y, element.x, element.content);
            attroff(attrs);
            break;
        }
            
        case AnimationType::FADE_IN: {
            for (int i = 0; i < 4; ++i) {
                attron(attrs | (i < 2 ? A_DIM : 0));
                safe_mvprintw(element.y, element.x, element.content);
                refresh();
                std::this_thread::sleep_for(std::chrono::milliseconds(80));
                if (i < 3) {
                    mvprintw(element.y, element.x, "%*s", (int)element.content.length(), "");
                    refresh();
                    // Check for input before sleeping
        if (check_for_input_during_animation()) {
            // Skip to end of animation
            safe_mvprintw(element.y, element.x, element.content);
            refresh();
            break;
        }
                    std::this_thread::sleep_for(std::chrono::milliseconds(40));
                }
                attroff(attrs | A_DIM);
            }
            break;
        }
            
        default: {
            attron(attrs);
            safe_mvprintw(element.y, element.x, element.content);
            attroff(attrs);
            break;
        }
    }
    refresh();
}

void NCursesRenderer::render_element_instant(const SlideElement& element) {
    int attrs = COLOR_PAIR(element.color_pair);
    if (element.is_bold) attrs |= A_BOLD;
    if (element.type == ElementType::SHELL_OUTPUT && !element.executed) attrs |= A_DIM;
    
    attron(attrs);
    safe_mvprintw(element.y, element.x, element.content);
    attroff(attrs);
}

void NCursesRenderer::clear_with_background() {
    // Also explicitly fill any remaining areas
    attron(COLOR_PAIR(0));
    std::string line(COLS, ' ');
    for (int y = 0; y < LINES; ++y) {
        for (int x = 0; x < COLS; ++x) {
            mvprintw(y, 0, "%s", line.c_str());
        }
    }
    attroff(COLOR_PAIR(0));
    move(0, 0);
    refresh();
}