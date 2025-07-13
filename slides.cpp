#include <ncurses.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <regex>
#include <cstdio>
#include <memory>
#include <array>
#include <locale.h>

enum class Theme { DARK, LIGHT, MATRIX, RETRO };
enum class AnimationType { NONE, FADE_IN, SLIDE_IN, TYPEWRITER };
enum class ElementType { TEXT, HEADER1, HEADER2, HEADER3, BULLET, NUMBERED, CODE_BLOCK, SHELL_COMMAND, SHELL_OUTPUT };

struct SlideElement {
    std::string content;
    int y, x;
    int color_pair;
    bool is_bold = false;
    AnimationType animation = AnimationType::FADE_IN;
    int delay_ms = 0;
    ElementType type = ElementType::TEXT;
    
    // Shell command specific
    std::string shell_command;
    bool executed = false;
    std::vector<std::string> shell_output_lines;
    int output_scroll_offset = 0;
    int max_output_lines = 5;
};

struct ThemeConfig {
    int bg_color, title_color, subtitle_color, text_color, accent_color, code_color;
    const char* name;
};

class MarkdownSlideRenderer {
private:
    std::vector<std::vector<SlideElement>> slides;
    int current_slide = 0;
    Theme current_theme = Theme::DARK;
    bool show_timer = false;
    std::chrono::steady_clock::time_point start_time;
    std::vector<std::pair<std::string, std::string>> char_replacements;
    
    const std::vector<ThemeConfig> themes = {
        {COLOR_BLACK, COLOR_CYAN, COLOR_YELLOW, COLOR_WHITE, COLOR_GREEN, COLOR_MAGENTA, "Dark"},
        {COLOR_WHITE, COLOR_BLUE, COLOR_RED, COLOR_BLACK, COLOR_GREEN, COLOR_MAGENTA, "Light"},
        {COLOR_BLACK, COLOR_GREEN, COLOR_GREEN, COLOR_GREEN, COLOR_WHITE, COLOR_GREEN, "Matrix"},
        {COLOR_BLACK, COLOR_YELLOW, COLOR_CYAN, COLOR_WHITE, COLOR_MAGENTA, COLOR_RED, "Retro"}
    };
    
    void load_char_replacements(const std::string& config_file = "slide_config.txt") {
        char_replacements.clear();
        std::ifstream file(config_file);
        if (!file.is_open()) return;
        
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            
            size_t separator_pos = line.find(" -> ");
            if (separator_pos == std::string::npos) continue;
            
            std::string search_term = line.substr(0, separator_pos);
            std::string replacement = line.substr(separator_pos + 4);
            
            // Remove quotes if present
            auto remove_quotes = [](std::string& s) {
                if (s.length() >= 2 && s.front() == '"' && s.back() == '"') {
                    s = s.substr(1, s.length() - 2);
                }
            };
            remove_quotes(search_term);
            remove_quotes(replacement);
            
            char_replacements.emplace_back(search_term, replacement);
        }
    }
    
    void safe_mvprintw(int y, int x, const std::string& text) {
        std::string safe_text = text;
        
        // Apply replacements from config file
        for (const auto& [search, replace] : char_replacements) {
            size_t pos = 0;
            while ((pos = safe_text.find(search, pos)) != std::string::npos) {
                safe_text.replace(pos, search.length(), replace);
                pos += replace.length();
            }
        }
        
        // Handle remaining problematic UTF-8 characters
        std::string result;
        for (size_t i = 0; i < safe_text.length(); ++i) {
            unsigned char c = static_cast<unsigned char>(safe_text[i]);
            
            if (c >= 0xC0 && c <= 0xDF && i + 1 < safe_text.length()) {
                result += "?"; i++; // 2-byte UTF-8
            } else if (c >= 0xE0 && c <= 0xEF && i + 2 < safe_text.length()) {
                result += "?"; i += 2; // 3-byte UTF-8
            } else if (c >= 0xF0 && c <= 0xF7 && i + 3 < safe_text.length()) {
                result += "?"; i += 3; // 4-byte UTF-8
            } else if (c < 128) {
                result += c; // ASCII
            } else {
                result += "?"; // Other problematic character
            }
        }
        
        mvprintw(y, x, "%s", result.c_str());
    }
    
    std::string execute_shell_command(const std::string& command) {
        std::array<char, 128> buffer;
        std::string result;
        
        FILE* pipe_raw = popen(command.c_str(), "r");
        if (!pipe_raw) return "Error: Could not execute command";
        
        std::unique_ptr<FILE, int(*)(FILE*)> pipe(pipe_raw, pclose);
        
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        
        if (!result.empty() && result.back() == '\n') {
            result.pop_back();
        }
        return result;
    }
    
    void parse_slide(const std::string& content) {
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
            // Lists
            else if (line.substr(0, 2) == "- ") {
                element.content = "• " + line.substr(2);
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
        
        slides.push_back(elements);
    }
    
    void setup_theme() {
        const auto& theme = themes[static_cast<int>(current_theme)];
        
        bkgd(' ' | COLOR_PAIR(0));
        
        init_pair(1, theme.title_color, theme.bg_color);
        init_pair(2, theme.subtitle_color, theme.bg_color);
        init_pair(3, theme.text_color, theme.bg_color);
        init_pair(4, theme.accent_color, theme.bg_color);
        init_pair(5, theme.bg_color, theme.text_color);
        init_pair(6, theme.code_color, theme.bg_color);
        init_pair(7, COLOR_GREEN, theme.bg_color);
        init_pair(8, COLOR_YELLOW, theme.bg_color);
        init_pair(9, COLOR_RED, theme.bg_color);
        init_pair(0, theme.text_color, theme.bg_color);
    }
    
    void render_element_animated(const SlideElement& element) {
        int attrs = COLOR_PAIR(element.color_pair);
        if (element.is_bold) attrs |= A_BOLD;
        if (element.type == ElementType::SHELL_OUTPUT && !element.executed) attrs |= A_DIM;
        
        switch (element.animation) {
            case AnimationType::TYPEWRITER: {
                attron(attrs);
                for (size_t i = 0; i <= element.content.length(); ++i) {
                    safe_mvprintw(element.y, element.x, element.content.substr(0, i));
                    refresh();
                    std::this_thread::sleep_for(std::chrono::milliseconds(30));
                }
                attroff(attrs);
                break;
            }
                
            case AnimationType::SLIDE_IN: {
                attron(attrs);
                int start_x = element.x + element.content.length() + 10;
                for (int x = start_x; x >= element.x; x -= 3) {
                    mvprintw(element.y, 0, "%*s", COLS, "");
                    safe_mvprintw(element.y, std::max(x, element.x), element.content);
                    refresh();
                    std::this_thread::sleep_for(std::chrono::milliseconds(30));
                }
                mvprintw(element.y, 0, "%*s", COLS, "");
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
    
    void render_slide_animated() {
        clear();
        draw_header();
        draw_footer();
        refresh();
        
        for (const auto& element : slides[current_slide]) {
            if (element.type != ElementType::SHELL_OUTPUT) {
                std::this_thread::sleep_for(std::chrono::milliseconds(element.delay_ms));
                render_element_animated(element);
            }
        }
        
        // Render executed shell outputs
        for (const auto& element : slides[current_slide]) {
            if (element.type == ElementType::SHELL_COMMAND && element.executed) {
                update_shell_output_display(element);
            }
        }
        
        draw_progress_bar();
        refresh();
    }
    
    void render_slide_instant() {
        clear();
        draw_header();
        draw_footer();
        
        for (const auto& element : slides[current_slide]) {
            if (element.type == ElementType::SHELL_OUTPUT) continue;
            
            int attrs = COLOR_PAIR(element.color_pair);
            if (element.is_bold) attrs |= A_BOLD;
            if (element.type == ElementType::SHELL_OUTPUT && !element.executed) attrs |= A_DIM;
            
            attron(attrs);
            safe_mvprintw(element.y, element.x, element.content);
            attroff(attrs);
        }
        
        // Render executed shell outputs
        for (const auto& element : slides[current_slide]) {
            if (element.type == ElementType::SHELL_COMMAND && element.executed) {
                update_shell_output_display(element);
            }
        }
        
        draw_progress_bar();
        refresh();
    }
    
    void execute_shell_commands_on_slide() {
        bool has_shell_commands = std::any_of(slides[current_slide].begin(), slides[current_slide].end(),
            [](const auto& e) { return e.type == ElementType::SHELL_COMMAND; });
        
        if (!has_shell_commands) return;
        
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(LINES-4, 2, "Shell commands detected! Press ENTER to execute, ESC to skip");
        attroff(COLOR_PAIR(4) | A_BOLD);
        refresh();
        
        int ch = getch();
        mvprintw(LINES-4, 2, "%*s", COLS-4, ""); // Clear instruction
        
        if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
            for (auto& element : slides[current_slide]) {
                if (element.type == ElementType::SHELL_COMMAND && !element.executed) {
                    execute_single_shell_command(element);
                }
            }
        }
        refresh();
    }
    
    void execute_single_shell_command(SlideElement& shell_element) {
        if (shell_element.shell_command.empty()) return;
        
        attron(COLOR_PAIR(8) | A_BOLD);
        safe_mvprintw(shell_element.y + 1, shell_element.x, "Executing...");
        attroff(COLOR_PAIR(8) | A_BOLD);
        refresh();
        
        std::string output = execute_shell_command(shell_element.shell_command);
        shell_element.executed = true;
        
        // Split output into lines
        std::istringstream iss(output);
        std::string line;
        shell_element.shell_output_lines.clear();
        
        while (std::getline(iss, line)) {
            shell_element.shell_output_lines.push_back(line);
        }
        
        if (shell_element.shell_output_lines.empty()) {
            shell_element.shell_output_lines.push_back("[No output]");
        }
        
        update_shell_output_display(shell_element);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    void update_shell_output_display(const SlideElement& shell_element) {
        int output_y = shell_element.y + 1;
        int lines_to_show = std::min((int)shell_element.shell_output_lines.size(), shell_element.max_output_lines);
        
        // Clear old output
        for (int i = 0; i < 10; ++i) {
            mvprintw(output_y + i, 0, "%*s", COLS, "");
        }
        
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
        refresh();
    }
    
    void scroll_shell_output(int direction) {
        for (auto& element : slides[current_slide]) {
            if (element.type == ElementType::SHELL_COMMAND && element.executed) {
                int max_scroll = std::max(0, (int)element.shell_output_lines.size() - element.max_output_lines);
                
                if (direction > 0) {
                    element.output_scroll_offset = std::min(element.output_scroll_offset + 1, max_scroll);
                } else {
                    element.output_scroll_offset = std::max(element.output_scroll_offset - 1, 0);
                }
                
                update_shell_output_display(element);
            }
        }
    }
    
    void draw_header() {
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(0, 2, "Slide %d/%d", current_slide + 1, (int)slides.size());
        mvprintw(0, COLS - 15, "Theme: %s", themes[static_cast<int>(current_theme)].name);
        
        if (show_timer) {
            auto now = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - start_time);
            int minutes = duration.count() / 60;
            int seconds = duration.count() % 60;
            mvprintw(0, COLS - 35, "Time: %02d:%02d", minutes, seconds);
        }
        
        attroff(COLOR_PAIR(1) | A_BOLD);
        
        attron(COLOR_PAIR(4));
        mvhline(1, 0, '-', COLS);
        attroff(COLOR_PAIR(4));
    }
    
    void draw_footer() {
        attron(COLOR_PAIR(4));
        mvhline(LINES-2, 0, '-', COLS);
        attroff(COLOR_PAIR(4));
        
        attron(COLOR_PAIR(3));
        mvprintw(LINES-1, 2, "Controls: <-/-> Navigate | ENTER Execute | u/d Scroll | 'c' Reload Config | 't' Theme | 'h' Help | 'q' Quit");
        attroff(COLOR_PAIR(3));
    }
    
    void draw_progress_bar() {
        if (slides.empty()) return;
        
        int progress_width = (current_slide * (COLS - 4)) / slides.size();
        
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
    
    void show_help() {
        clear();
        
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
            "  c                Reload character config",
            "",
            "Other:",
            "  h                Show this help",
            "  q / Escape       Quit",
            "",
            "Supported Markdown:",
            "  # H1 Headers     ## H2 Headers    ### H3 Headers",
            "  - Bullet points  1. Numbered lists **Bold text**",
            "  ```code blocks```  ```$shell command```"
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
        getch();
        render_slide_instant();
    }
    
    void goto_slide() {
        clear();
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(LINES/2, 2, "Go to slide (1-%d): ", (int)slides.size());
        attroff(COLOR_PAIR(2) | A_BOLD);
        refresh();
        
        echo();
        char input[10];
        getstr(input);
        noecho();
        
        int slide_num = std::atoi(input);
        if (slide_num >= 1 && slide_num <= (int)slides.size()) {
            current_slide = slide_num - 1;
        }
        
        render_slide_instant();
    }
    
public:
    void load_slides(const std::string& filename) {
        load_char_replacements();
        
        std::ifstream file(filename);
        std::string line, slide_content;
        
        while (std::getline(file, line)) {
            if (line == "---") {
                if (!slide_content.empty()) {
                    parse_slide(slide_content);
                    slide_content.clear();
                }
            } else {
                slide_content += line + "\n";
            }
        }
        if (!slide_content.empty()) {
            parse_slide(slide_content);
        }
    }
    
    void run() {
        if (slides.empty()) {
            printf("No slides loaded!\n");
            return;
        }
        
        setlocale(LC_ALL, "");
        initscr();
        noecho();
        cbreak();
        keypad(stdscr, TRUE);
        curs_set(0);
        
        if (has_colors()) {
            start_color();
            use_default_colors();
        }
        
        setup_theme();
        start_time = std::chrono::steady_clock::now();
        
        bool use_animations = true;
        
        if (use_animations) {
            render_slide_animated();
        } else {
            render_slide_instant();
        }
        
        execute_shell_commands_on_slide();
        
        int ch;
        while ((ch = getch()) != 'q' && ch != 27) {
            switch(ch) {
                case KEY_RIGHT: case ' ': case 'l':
                    if (current_slide < (int)slides.size() - 1) {
                        current_slide++;
                        use_animations ? render_slide_animated() : render_slide_instant();
                        execute_shell_commands_on_slide();
                    }
                    break;
                    
                case KEY_LEFT: case KEY_BACKSPACE: case 'h':
                    if (current_slide > 0) {
                        current_slide--;
                        use_animations ? render_slide_animated() : render_slide_instant();
                        execute_shell_commands_on_slide();
                    }
                    break;
                    
                case '\n': case '\r': case KEY_ENTER:
                    execute_shell_commands_on_slide();
                    break;
                    
                case KEY_HOME: case '0':
                    current_slide = 0;
                    render_slide_instant();
                    break;
                    
                case KEY_END: case '$':
                    current_slide = slides.size() - 1;
                    render_slide_instant();
                    break;
                    
                case 'g':
                    goto_slide();
                    execute_shell_commands_on_slide();
                    break;
                    
                case 't':
                    current_theme = static_cast<Theme>((static_cast<int>(current_theme) + 1) % themes.size());
                    setup_theme();
                    render_slide_instant();
                    break;
                    
                case 'a':
                    use_animations = !use_animations;
                    render_slide_instant();
                    break;
                    
                case 'T':
                    show_timer = !show_timer;
                    render_slide_instant();
                    break;
                    
                case 'r':
                    render_slide_instant();
                    break;
                    
                case 'u':
                    scroll_shell_output(-1);
                    break;
                    
                case 'd':
                    scroll_shell_output(1);
                    break;
                    
                case 'c':
                    load_char_replacements();
                    clear();
                    attron(COLOR_PAIR(2) | A_BOLD);
                    mvprintw(LINES/2, 2, "Configuration reloaded! Press any key to continue...");
                    attroff(COLOR_PAIR(2) | A_BOLD);
                    refresh();
                    getch();
                    render_slide_instant();
                    break;
                    
                case '?':
                    show_help();
                    break;
            }
        }
        
        endwin();
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <markdown_file>\n", argv[0]);
        printf("\nExample markdown format:\n");
        printf("# Title Slide\n");
        printf("This is the content\n");
        printf("---\n");
        printf("## Second Slide\n");
        printf("- Bullet point 1\n");
        printf("- Bullet point 2\n");
        printf("---\n");
        printf("### Code Example\n");
        printf("```cpp\n");
        printf("int main() {\n");
        printf("    return 0;\n");
        printf("}\n");
        printf("```\n");
        printf("---\n");
        printf("### Shell Command Demo\n");
        printf("```$ls -la\n");
        printf("```\n");
        printf("```$date\n");
        printf("```\n");
        return 1;
    }
    
    MarkdownSlideRenderer renderer;
    renderer.load_slides(argv[1]);
    renderer.run();
    
    return 0;
}
