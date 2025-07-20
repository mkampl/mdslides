#include "presentation_state.hh"

const std::vector<SlideElement> &PresentationState::get_current_slide() const
{
    if (current_slide >= 0 && current_slide < slides.get_slide_count())
    {
        return slides.get_slide(current_slide);
    }
    static std::vector<SlideElement> empty;
    return empty;
}

int PresentationState::get_elapsed_seconds() const
{
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - start_time);
    return duration.count();
}

void PresentationState::next_slide()
{
    if (current_slide < slides.get_slide_count() - 1)
    {
        current_slide++;
        start_slide_animation();
    }
}

void PresentationState::prev_slide()
{
    if (current_slide > 0)
    {
        current_slide--;
        start_slide_animation();
    }
}

void PresentationState::goto_slide(int slide_num)
{
    if (slide_num >= 1 && slide_num <= slides.get_slide_count())
    {
        current_slide = slide_num - 1;
        start_slide_animation();
    }
}

void PresentationState::start_slide_animation()
{
    slide_changed = true;
    slide_start_time = std::chrono::steady_clock::now();
    animation_step = 0;

    // Reset element visibility
    const auto &current_slide_elements = get_current_slide();
    element_visible.clear();
    element_visible.resize(current_slide_elements.size(), false);

    // Reset shell command selection
    shell_selection_active = false;
    selected_shell_command = 0;
    update_shell_commands();
}

void PresentationState::update_shell_commands()
{
    shell_command_indices.clear();
    const auto &current_slide_elements = get_current_slide();

    for (int i = 0; i < static_cast<int>(current_slide_elements.size()); ++i)
    {
        if (current_slide_elements[i].type == ElementType::SHELL_COMMAND)
        {
            shell_command_indices.push_back(i);
        }
    }

    // Reset selection if no commands or selection out of range
    if (shell_command_indices.empty() || selected_shell_command >= static_cast<int>(shell_command_indices.size()))
    {
        selected_shell_command = 0;
    }
}

void PresentationState::start_shell_selection()
{
    if (!shell_command_indices.empty())
    {
        shell_selection_active = true;
        selected_shell_command = 0;
    }
}

void PresentationState::exit_shell_selection()
{
    shell_selection_active = false;
    selected_shell_command = 0;
}

bool PresentationState::navigate_shell_up()
{
    if (shell_selection_active && selected_shell_command > 0)
    {
        selected_shell_command--;
        return true;
    }
    return false;
}

bool PresentationState::navigate_shell_down()
{
    if (shell_selection_active && selected_shell_command < static_cast<int>(shell_command_indices.size()) - 1)
    {
        selected_shell_command++;
        return true;
    }
    return false;
}

std::string PresentationState::get_selected_shell_command() const
{
    if (shell_selection_active && !shell_command_indices.empty() &&
        selected_shell_command >= 0 && selected_shell_command < static_cast<int>(shell_command_indices.size()))
    {
        int element_index = shell_command_indices[selected_shell_command];
        const auto &current_slide_elements = get_current_slide();
        if (element_index < static_cast<int>(current_slide_elements.size()))
        {
            return current_slide_elements[element_index].shell_command;
        }
    }
    return "";
}

int PresentationState::get_selected_shell_index() const
{
    if (shell_selection_active && !shell_command_indices.empty() &&
        selected_shell_command >= 0 && selected_shell_command < static_cast<int>(shell_command_indices.size()))
    {
        return shell_command_indices[selected_shell_command];
    }
    return -1;
}

void PresentationState::update_animation()
{
    if (!animations_enabled)
    {
        // No animation - show all elements immediately
        element_visible.assign(get_current_slide().size(), true);
        slide_changed = false;
        return;
    }

    if (!slide_changed)
    {
        // Animation already complete - show all elements
        element_visible.assign(get_current_slide().size(), true);
        return;
    }

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - slide_start_time);

    const auto &current_slide_elements = get_current_slide();

    // Ensure element_visible has correct size
    if (element_visible.size() != current_slide_elements.size())
    {
        element_visible.resize(current_slide_elements.size(), false);
    }

    // Calculate when each element should start based on when previous finishes
    int current_time = 0;
    for (int i = 0; i < static_cast<int>(current_slide_elements.size()); ++i)
    {
        // Element becomes visible when its start time is reached
        element_visible[i] = (elapsed.count() >= current_time);

        // Check if this element and the next are both code elements
        bool is_code_block = (current_slide_elements[i].type == ElementType::CODE_BLOCK ||
                              current_slide_elements[i].type == ElementType::SHELL_COMMAND);
        bool next_is_code = (i + 1 < static_cast<int>(current_slide_elements.size()) &&
                             (current_slide_elements[i + 1].type == ElementType::CODE_BLOCK ||
                              current_slide_elements[i + 1].type == ElementType::SHELL_COMMAND));

        // Calculate duration for this element type
        int element_duration;
        switch (current_slide_elements[i].type)
        {
        case ElementType::CODE_BLOCK:
        case ElementType::SHELL_COMMAND:
            // Duration based on actual content length for code
            element_duration = static_cast<int>(current_slide_elements[i].content.length()) * 40; // 40ms per char
            break;
        case ElementType::HEADER1:
        case ElementType::HEADER2:
        case ElementType::HEADER3:
            element_duration = 600; // Slide duration
            break;
        default:
            element_duration = 400; // Fade duration
            break;
        }

        current_time += element_duration;

        // Add gap - smaller gap between code lines, normal gap otherwise
        if (is_code_block && next_is_code)
        {
            current_time += 100; // Small gap between code lines
        }
        else
        {
            current_time += 200; // Normal gap between elements
        }
    }

    // Animation is done when all elements are complete
    if (elapsed.count() >= current_time)
    {
        slide_changed = false;
        element_visible.assign(current_slide_elements.size(), true);
    }
}

bool PresentationState::is_element_visible(int index) const
{
    if (!animations_enabled || !slide_changed)
    {
        return true;
    }
    return index < static_cast<int>(element_visible.size()) && element_visible[index];
}

void PresentationState::cycle_theme()
{
    current_theme = static_cast<Theme>((static_cast<int>(current_theme) + 1) % 4);
}