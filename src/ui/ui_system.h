#pragma once
#include "math/vec3.h"
#include "math/vec2.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace vge {

// ============================================
// UI Event Types
// ============================================
enum class UIEventType {
    Click,
    Hover,
    Press,
    Release,
    Drag,
    Scroll,
    TextInput
};

struct UIEvent {
    UIEventType type;
    Vec2 position;  // Mouse/touch position
    Vec2 delta;     // For drag/scroll
    int keyCode;    // For keyboard input
    std::string text; // For text input
};

// ============================================
// Base UI Element
// ============================================
class UIElement {
protected:
    Vec2 position;  // Top-left corner
    Vec2 size;
    bool visible;
    bool enabled;
    bool hovered;
    bool pressed;
    
    std::string name;
    std::vector<std::unique_ptr<UIElement>> children;
    UIElement* parent;
    
    // Styling
    Vec3 backgroundColor;
    Vec3 foregroundColor;
    Vec3 hoverColor;
    Vec3 pressedColor;
    float alpha;
    float borderRadius;
    float borderWidth;
    Vec3 borderColor;
    
    // Callbacks
    std::function<void()> onClick;
    std::function<void()> onHover;
    std::function<void()> onPress;
    std::function<void()> onRelease;
    std::function<void(const UIEvent&)> onEvent;
    
public:
    UIElement(const std::string& name = "Element");
    virtual ~UIElement() = default;
    
    // Position & Size
    void SetPosition(const Vec2& pos) { position = pos; }
    void SetSize(const Vec2& sz) { size = sz; }
    Vec2 GetPosition() const { return position; }
    Vec2 GetSize() const { return size; }
    Vec2 GetAbsolutePosition() const;
    
    // Visibility
    void SetVisible(bool vis) { visible = vis; }
    bool IsVisible() const { return visible; }
    void SetEnabled(bool en) { enabled = en; }
    bool IsEnabled() const { return enabled; }
    
    // State
    void SetHovered(bool h) { hovered = h; }
    bool IsHovered() const { return hovered; }
    bool IsPressed() const { return pressed; }
    
    // Hierarchy
    void AddChild(std::unique_ptr<UIElement> child);
    void RemoveChild(UIElement* child);
    UIElement* GetParent() const { return parent; }
    const std::vector<std::unique_ptr<UIElement>>& GetChildren() const { return children; }
    UIElement* FindChild(const std::string& childName);
    std::string GetName() const { return name; }
    
    // Event handling
    virtual bool HandleEvent(const UIEvent& event);
    virtual void OnClick() { if (onClick) onClick(); }
    virtual void OnHover() { if (onHover) onHover(); }
    virtual void OnPress() { if (onPress) onPress(); }
    virtual void OnRelease() { if (onRelease) onRelease(); }
    
    // Callbacks
    void SetOnClick(std::function<void()> callback) { onClick = callback; }
    void SetOnHover(std::function<void()> callback) { onHover = callback; }
    void SetOnPress(std::function<void()> callback) { onPress = callback; }
    void SetOnRelease(std::function<void()> callback) { onRelease = callback; }
    void SetOnEvent(std::function<void(const UIEvent&)> callback) { onEvent = callback; }
    
    // Styling
    void SetBackgroundColor(const Vec3& color) { backgroundColor = color; }
    void SetForegroundColor(const Vec3& color) { foregroundColor = color; }
    void SetHoverColor(const Vec3& color) { hoverColor = color; }
    void SetPressedColor(const Vec3& color) { pressedColor = color; }
    void SetAlpha(float a) { alpha = a; }
    void SetBorderRadius(float radius) { borderRadius = radius; }
    void SetBorderWidth(float width) { borderWidth = width; }
    void SetBorderColor(const Vec3& color) { borderColor = color; }
    
    // Rendering
    virtual void Render();
    virtual void Update(float deltaTime);
    
    // Layout
    virtual void Layout();
    bool ContainsPoint(const Vec2& point) const;
    
    // Debug
    void PrintHierarchy(int indent = 0) const;
};

// ============================================
// Button
// ============================================
class UIButton : public UIElement {
private:
    std::string text;
    std::string icon;  // Optional icon path
    bool toggleMode;
    bool toggled;
    
public:
    UIButton(const std::string& name = "Button", const std::string& text = "");
    
    void SetText(const std::string& t) { text = t; }
    std::string GetText() const { return text; }
    void SetIcon(const std::string& iconPath) { icon = iconPath; }
    
    void SetToggleMode(bool toggle) { toggleMode = toggle; }
    bool IsToggled() const { return toggled; }
    void SetToggled(bool t) { toggled = t; }
    
    void Render() override;
    bool HandleEvent(const UIEvent& event) override;
};

// ============================================
// Label/Text
// ============================================
class UILabel : public UIElement {
private:
    std::string text;
    float fontSize;
    bool autoSize;
    bool wordWrap;
    
public:
    UILabel(const std::string& name = "Label", const std::string& text = "");
    
    void SetText(const std::string& t) { text = t; }
    std::string GetText() const { return text; }
    void SetFontSize(float newFontSize) { fontSize = newFontSize; }
    float GetFontSize() const { return fontSize; }
    void SetAutoSize(bool autoSz) { autoSize = autoSz; }
    void SetWordWrap(bool wrap) { wordWrap = wrap; }
    
    void Render() override;
    void Layout() override;
};

// ============================================
// Slider
// ============================================
class UISlider : public UIElement {
private:
    float minValue;
    float maxValue;
    float currentValue;
    float step;
    bool vertical;
    
    std::function<void(float)> onValueChanged;
    
public:
    UISlider(const std::string& name = "Slider", float min = 0, float max = 1);
    
    void SetValue(float value);
    float GetValue() const { return currentValue; }
    void SetMin(float min) { minValue = min; }
    void SetMax(float max) { maxValue = max; }
    void SetStep(float s) { step = s; }
    void SetVertical(bool vert) { vertical = vert; }
    
    void SetOnValueChanged(std::function<void(float)> callback) { onValueChanged = callback; }
    
    void Render() override;
    bool HandleEvent(const UIEvent& event) override;
};

// ============================================
// Text Input
// ============================================
class UITextInput : public UIElement {
private:
    std::string text;
    std::string placeholder;
    std::string hint;
    int maxLength;
    bool password;
    bool multiline;
    bool focused;
    float cursorBlinkTimer;
    bool cursorVisible;
    
    std::function<void(const std::string&)> onTextChanged;
    std::function<void(const std::string&)> onSubmit;
    
public:
    UITextInput(const std::string& name = "TextInput", const std::string& placeholder = "");
    
    void SetText(const std::string& t);
    std::string GetText() const { return text; }
    void SetPlaceholder(const std::string& p) { placeholder = p; }
    void SetHint(const std::string& h) { hint = h; }
    void SetMaxLength(int max) { maxLength = max; }
    void SetPassword(bool pass) { password = pass; }
    void SetMultiline(bool multi) { multiline = multi; }
    
    void SetOnTextChanged(std::function<void(const std::string&)> callback) { onTextChanged = callback; }
    void SetOnSubmit(std::function<void(const std::string&)> callback) { onSubmit = callback; }
    
    void Render() override;
    bool HandleEvent(const UIEvent& event) override;
    void Update(float deltaTime) override;
};

// ============================================
// Panel/Container
// ============================================
class UIPanel : public UIElement {
private:
    bool scrollable;
    Vec2 scrollOffset;
    Vec2 contentSize;
    bool autoLayout;
    float padding;
    float spacing;
    
public:
    UIPanel(const std::string& name = "Panel");
    
    void SetScrollable(bool scroll) { scrollable = scroll; }
    void SetScrollOffset(const Vec2& offset) { scrollOffset = offset; }
    Vec2 GetScrollOffset() const { return scrollOffset; }
    void SetAutoLayout(bool autoLay) { autoLayout = autoLay; }
    void SetPadding(float pad) { padding = pad; }
    void SetSpacing(float space) { spacing = space; }
    
    void Render() override;
    void Layout() override;
    bool HandleEvent(const UIEvent& event) override;
};

// ============================================
// Image
// ============================================
class UIImage : public UIElement {
private:
    std::string texturePath;
    Vec2 uvMin;
    Vec2 uvMax;
    bool preserveAspect;
    
public:
    UIImage(const std::string& name = "Image", const std::string& texture = "");
    
    void SetTexture(const std::string& path) { texturePath = path; }
    std::string GetTexture() const { return texturePath; }
    void SetUVRect(const Vec2& min, const Vec2& max) { uvMin = min; uvMax = max; }
    void SetPreserveAspect(bool preserve) { preserveAspect = preserve; }
    
    void Render() override;
};

// ============================================
// Checkbox
// ============================================
class UICheckbox : public UIElement {
private:
    bool checked;
    std::string label;
    
    std::function<void(bool)> onCheckedChanged;
    
public:
    UICheckbox(const std::string& name = "Checkbox", const std::string& label = "");
    
    void SetChecked(bool c);
    bool IsChecked() const { return checked; }
    void SetLabel(const std::string& l) { label = l; }
    
    void SetOnCheckedChanged(std::function<void(bool)> callback) { onCheckedChanged = callback; }
    
    void Render() override;
    bool HandleEvent(const UIEvent& event) override;
};

// ============================================
// Dropdown
// ============================================
class UIDropdown : public UIElement {
private:
    std::vector<std::string> options;
    int selectedIndex;
    bool expanded;
    
    std::function<void(int, const std::string&)> onSelectionChanged;
    
public:
    UIDropdown(const std::string& name = "Dropdown");
    
    void AddOption(const std::string& option);
    void RemoveOption(int index);
    void ClearOptions();
    void SetSelectedIndex(int index);
    int GetSelectedIndex() const { return selectedIndex; }
    std::string GetSelectedOption() const;
    
    void SetOnSelectionChanged(std::function<void(int, const std::string&)> callback) { onSelectionChanged = callback; }
    
    void Render() override;
    bool HandleEvent(const UIEvent& event) override;
};

// ============================================
// UI Manager
// ============================================
class UIManager {
private:
    std::vector<std::unique_ptr<UIElement>> rootElements;
    UIElement* focusedElement;
    UIElement* hoveredElement;
    Vec2 screenSize;
    float scale;
    
public:
    UIManager();
    ~UIManager();
    
    // Element creation
    UIElement* CreateElement(const std::string& type, const std::string& name);
    UIButton* CreateButton(const std::string& name, const std::string& text);
    UILabel* CreateLabel(const std::string& name, const std::string& text);
    UISlider* CreateSlider(const std::string& name, float min, float max);
    UITextInput* CreateTextInput(const std::string& name, const std::string& placeholder);
    UIPanel* CreatePanel(const std::string& name);
    UIImage* CreateImage(const std::string& name, const std::string& texture);
    UICheckbox* CreateCheckbox(const std::string& name, const std::string& label);
    UIDropdown* CreateDropdown(const std::string& name);
    
    // Hierarchy
    void AddRootElement(std::unique_ptr<UIElement> element);
    void RemoveElement(UIElement* element);
    UIElement* FindElement(const std::string& name);
    
    // Input handling
    void HandleMouseMove(const Vec2& position);
    void HandleMouseClick(const Vec2& position, bool pressed);
    void HandleTextInput(const std::string& text);
    void HandleKeyInput(int keyCode, bool pressed);
    
    // Updates
    void Update(float deltaTime);
    void Render();
    void Layout();
    
    // Screen
    void SetScreenSize(const Vec2& size) { screenSize = size; }
    Vec2 GetScreenSize() const { return screenSize; }
    void SetScale(float s) { scale = s; }
    float GetScale() const { return scale; }
    
    // Focus
    void SetFocus(UIElement* element);
    UIElement* GetFocusedElement() const { return focusedElement; }
    
    // Clear
    void Clear();
};

} // namespace vge
