#include "button.h"
#include <iostream>
namespace aether {
Button::Button() {}
void Button::setText(const std::string& text) { this->text = text; }
std::string Button::getText() const { return text; }
void Button::setCallback(std::function<void()> callback) { this->callback = callback; }
void Button::render() {}
bool Button::onClick(const Vec2& pos) {
    if (UIElement::onClick(pos)) {
        if (callback) callback();
        return true;
    }
    return false;
}
} // namespace aether
