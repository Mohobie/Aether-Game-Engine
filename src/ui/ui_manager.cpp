#include "ui_manager.h"
#include <algorithm>
namespace aether {
UIManager::UIManager() {}
void UIManager::addElement(std::shared_ptr<UIElement> element) { elements.push_back(element); }
void UIManager::removeElement(UIElement* element) {
    auto it = std::remove_if(elements.begin(), elements.end(),
        [element](const auto& e) { return e.get() == element; });
    elements.erase(it, elements.end());
}
void UIManager::render() {
    for (auto& element : elements) element->render();
}
bool UIManager::onClick(const Vec2& pos) {
    for (auto& element : elements) {
        if (element->onClick(pos)) return true;
    }
    return false;
}
void UIManager::clear() { elements.clear(); }
} // namespace aether
