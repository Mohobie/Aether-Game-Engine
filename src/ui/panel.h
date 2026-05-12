#pragma once
#include "ui_element.h"
namespace vge {
class Panel : public UIElement {
public:
    Panel();
    void setBackgroundColor(uint32_t color);
    void render() override;
private:
    uint32_t backgroundColor = 0x333333FF;
};
} // namespace vge
