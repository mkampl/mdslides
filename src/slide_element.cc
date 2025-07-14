#include "slide_element.hh"

void SlideCollection::add_slide(const std::vector<SlideElement>& slide) {
    slides.push_back(slide);
}

std::vector<SlideElement>& SlideCollection::get_slide(int index) {
    return slides[index];
}

const std::vector<SlideElement>& SlideCollection::get_slide(int index) const {
    return slides[index];
}

int SlideCollection::get_slide_count() const {
    return static_cast<int>(slides.size());
}

bool SlideCollection::is_empty() const {
    return slides.empty();
}

void SlideCollection::clear() {
    slides.clear();
}
