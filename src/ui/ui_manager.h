#pragma once
#include "ui_element.h"
#include <vector>
#include <memory>
namespace aether {
class UIManager {
public:
    UIManager();
    void addElement(std::shared_ptr<UIElement> element);
    void removeElement(UIElement* element);
    void render();
    bool onClick(const Vec2& pos);
    void clear();
private:
    std::vector<std::shared_ptr<UIElement>> elements;
};
} // namespace aether
