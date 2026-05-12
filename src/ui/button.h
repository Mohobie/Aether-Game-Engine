#pragma once
#include "ui_element.h"
#include <functional>
namespace vge {
class Button : public UIElement {
public:
    Button();
    void setText(const std::string& text);
    std::string getText() const;
    void setCallback(std::function<void()> callback);
    void render() override;
    bool onClick(const Vec2& pos) override;
private:
    std::string text;
    std::function<void()> callback;
};
} // namespace vge
