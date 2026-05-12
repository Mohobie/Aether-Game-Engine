#include "label.h"
namespace vge {
Label::Label() {}
void Label::setText(const std::string& text) { this->text = text; }
std::string Label::getText() const { return text; }
void Label::setColor(uint32_t color) { this->color = color; }
void Label::render() {}
} // namespace vge
