#pragma once
#include "core/math.h"
#include <string>
#include <vector>
#include <memory>
namespace aether {
class UIElement {
public:
    UIElement();
    virtual ~UIElement() = default;
    void setPosition(const Vec2& pos);
    void setSize(const Vec2& size);
    Vec2 getPosition() const;
    Vec2 getSize() const;
    void setVisible(bool visible);
    bool isVisible() const;
    void addChild(std::shared_ptr<UIElement> child);
    virtual void render();
    virtual bool onClick(const Vec2& pos);
protected:
    Vec2 position;
    Vec2 size;
    bool visible = true;
    std::vector<std::shared_ptr<UIElement>> children;
};
} // namespace aether
