#include "ui_element.h"
namespace vge {
UIElement::UIElement() : position(0,0), size(100,100) {}
void UIElement::setPosition(const Vec2& pos) { position = pos; }
void UIElement::setSize(const Vec2& size) { this->size = size; }
Vec2 UIElement::getPosition() const { return position; }
Vec2 UIElement::getSize() const { return size; }
void UIElement::setVisible(bool visible) { this->visible = visible; }
bool UIElement::isVisible() const { return visible; }
void UIElement::addChild(std::shared_ptr<UIElement> child) { children.push_back(child); }
void UIElement::render() {}
bool UIElement::onClick(const Vec2& pos) {
    if (!visible) return false;
    if (pos.x >= position.x && pos.x <= position.x + size.x &&
        pos.y >= position.y && pos.y <= position.y + size.y) {
        for (auto& child : children) {
            if (child->onClick(pos)) return true;
        }
        return true;
    }
    return false;
}
} // namespace vge
