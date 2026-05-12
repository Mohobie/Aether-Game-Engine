#pragma once
#include "ui_element.h"
namespace vge {
class Label : public UIElement {
public:
    Label();
    void setText(const std::string& text);
    std::string getText() const;
    void setColor(uint32_t color);
    void render() override;
private:
    std::string text;
    uint32_t color = 0xFFFFFFFF;
};
} // namespace vge
