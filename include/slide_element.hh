#pragma once

#include <string>
#include <vector>

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

class SlideCollection {
public:
    void add_slide(const std::vector<SlideElement>& slide);
    std::vector<SlideElement>& get_slide(int index);
    const std::vector<SlideElement>& get_slide(int index) const;
    int get_slide_count() const;
    bool is_empty() const;
    void clear();

private:
    std::vector<std::vector<SlideElement>> slides;
};
