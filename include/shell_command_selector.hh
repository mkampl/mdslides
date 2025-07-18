#pragma once

#include "slide_element.hh"
#include <vector>

class ISlideRenderer;

class ShellCommandSelector
{
private:
    std::vector<SlideElement *> shell_commands;
    int selected_index;
    bool selection_mode;
    ISlideRenderer *renderer;

public:
    ShellCommandSelector();

    void set_renderer(ISlideRenderer *r);
    void enter_selection_mode(std::vector<SlideElement> &slide_elements);
    void exit_selection_mode();

    bool is_active() const;
    bool navigate_up();
    bool navigate_down();
    SlideElement *get_selected_command();
    int get_command_count() const;
    int get_selected_index() const;

private:
    void update_selection_display();
    void clear_all_highlights();
    void highlight_command(int index, bool highlight);
};