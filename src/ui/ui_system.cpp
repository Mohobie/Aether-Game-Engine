#include "ui_system.h"
#include <iostream>
#include <algorithm>

namespace vge {

// ============================================
// UIElement Implementation
// ============================================

UIElement::UIElement(const std::string& name) 
    : position(0, 0)
    , size(100, 30)
    , visible(true)
    , enabled(true)
    , hovered(false)
    , pressed(false)
    , name(name)
    , parent(nullptr)
    , backgroundColor(0.2f, 0.2f, 0.2f)
    , foregroundColor(1.0f, 1.0f, 1.0f)
    , hoverColor(0.3f, 0.3f, 0.3f)
    , pressedColor(0.15f, 0.15f, 0.15f)
    , alpha(1.0f)
    , borderRadius(0)
    , borderWidth(0)
    , borderColor(0, 0, 0) {}

Vec2 UIElement::GetAbsolutePosition() const {
    if (parent) {
        Vec2 parentPos = parent->GetAbsolutePosition();
        return Vec2(position.x + parentPos.x, position.y + parentPos.y);
    }
    return position;
}

void UIElement::AddChild(std::unique_ptr<UIElement> child) {
    child->parent = this;
    children.push_back(std::move(child));
}

void UIElement::RemoveChild(UIElement* child) {
    children.erase(
        std::remove_if(children.begin(), children.end(),
            [child](const std::unique_ptr<UIElement>& c) { return c.get() == child; }),
        children.end()
    );
}

UIElement* UIElement::FindChild(const std::string& childName) {
    for (auto& child : children) {
        if (child->GetName() == childName) {
            return child.get();
        }
        UIElement* found = child->FindChild(childName);
        if (found) return found;
    }
    return nullptr;
}

bool UIElement::HandleEvent(const UIEvent& event) {
    if (!visible || !enabled) return false;
    
    // Check children first (front to back)
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
        if ((*it)->HandleEvent(event)) {
            return true;
        }
    }
    
    // Check self
    if (ContainsPoint(event.position)) {
        switch (event.type) {
            case UIEventType::Click:
                OnClick();
                return true;
            case UIEventType::Hover:
                hovered = true;
                OnHover();
                return true;
            case UIEventType::Press:
                pressed = true;
                OnPress();
                return true;
            case UIEventType::Release:
                pressed = false;
                OnRelease();
                return true;
            default:
                if (onEvent) onEvent(event);
                return true;
        }
    } else {
        hovered = false;
    }
    
    return false;
}

void UIElement::Render() {
    if (!visible) return;
    
    // Would render background, border, etc.
    // For now, just render children
    for (auto& child : children) {
        child->Render();
    }
}

void UIElement::Update(float deltaTime) {
    for (auto& child : children) {
        child->Update(deltaTime);
    }
}

void UIElement::Layout() {
    for (auto& child : children) {
        child->Layout();
    }
}

bool UIElement::ContainsPoint(const Vec2& point) const {
    Vec2 absPos = GetAbsolutePosition();
    return point.x >= absPos.x && point.x <= absPos.x + size.x &&
           point.y >= absPos.y && point.y <= absPos.y + size.y;
}

void UIElement::PrintHierarchy(int indent) const {
    std::string prefix(indent * 2, ' ');
    std::cout << prefix << "[" << name << "] " 
              << "pos:(" << position.x << "," << position.y << ") "
              << "size:(" << size.x << "x" << size.y << ") "
              << (visible ? "visible" : "hidden") << std::endl;
    
    for (const auto& child : children) {
        child->PrintHierarchy(indent + 1);
    }
}

// ============================================
// UIButton Implementation
// ============================================

UIButton::UIButton(const std::string& name, const std::string& text)
    : UIElement(name)
    , text(text)
    , toggleMode(false)
    , toggled(false) {}

void UIButton::Render() {
    if (!visible) return;
    
    Vec3 color = backgroundColor;
    if (pressed) color = pressedColor;
    else if (hovered) color = hoverColor;
    if (toggleMode && toggled) color = Vec3(0.4f, 0.6f, 0.4f);
    
    // Would render button background and text
    std::cout << "[UI] Button '" << name << "': " << text 
              << " (" << (hovered ? "hover" : "") 
              << (pressed ? ",pressed" : "") << ")" << std::endl;
    
    UIElement::Render();
}

bool UIButton::HandleEvent(const UIEvent& event) {
    if (!visible || !enabled) return false;
    
    bool handled = UIElement::HandleEvent(event);
    
    if (handled && event.type == UIEventType::Click && toggleMode) {
        toggled = !toggled;
    }
    
    return handled;
}

// ============================================
// UILabel Implementation
// ============================================

UILabel::UILabel(const std::string& name, const std::string& text)
    : UIElement(name)
    , text(text)
    , fontSize(14)
    , autoSize(true)
    , wordWrap(false) {}

void UILabel::Render() {
    if (!visible) return;
    
    // Would render text
    std::cout << "[UI] Label '" << name << "': " << text << std::endl;
    
    UIElement::Render();
}

void UILabel::Layout() {
    if (autoSize) {
        // Estimate size based on text length
        SetSize(Vec2(text.length() * fontSize * 0.6f, fontSize * 1.2f));
    }
    UIElement::Layout();
}

// ============================================
// UISlider Implementation
// ============================================

UISlider::UISlider(const std::string& name, float min, float max)
    : UIElement(name)
    , minValue(min)
    , maxValue(max)
    , currentValue(min)
    , step(0.01f)
    , vertical(false) {
    SetSize(Vec2(100, 20));
}

void UISlider::SetValue(float value) {
    float oldValue = currentValue;
    currentValue = std::max(minValue, std::min(maxValue, value));
    
    // Round to step
    if (step > 0) {
        currentValue = std::round(currentValue / step) * step;
    }
    
    if (currentValue != oldValue && onValueChanged) {
        onValueChanged(currentValue);
    }
}

void UISlider::Render() {
    if (!visible) return;
    
    float ratio = (currentValue - minValue) / (maxValue - minValue);
    
    // Would render slider track and handle
    std::cout << "[UI] Slider '" << name << "': " << currentValue 
              << " (" << (ratio * 100) << "%)" << std::endl;
    
    UIElement::Render();
}

bool UISlider::HandleEvent(const UIEvent& event) {
    if (!visible || !enabled) return false;
    
    if (event.type == UIEventType::Drag || 
        (event.type == UIEventType::Press && ContainsPoint(event.position))) {
        
        Vec2 absPos = GetAbsolutePosition();
        float ratio;
        
        if (vertical) {
            ratio = (event.position.y - absPos.y) / GetSize().y;
        } else {
            ratio = (event.position.x - absPos.x) / GetSize().x;
        }
        
        ratio = std::max(0.0f, std::min(1.0f, ratio));
        SetValue(minValue + ratio * (maxValue - minValue));
        
        return true;
    }
    
    return UIElement::HandleEvent(event);
}

// ============================================
// UITextInput Implementation
// ============================================

UITextInput::UITextInput(const std::string& name, const std::string& placeholder)
    : UIElement(name)
    , placeholder(placeholder)
    , maxLength(100)
    , password(false)
    , multiline(false)
    , focused(false)
    , cursorBlinkTimer(0)
    , cursorVisible(true) {
    SetSize(Vec2(150, 30));
}

void UITextInput::SetText(const std::string& t) {
    text = t;
    if (static_cast<int>(text.length()) > maxLength) {
        text = text.substr(0, maxLength);
    }
    if (onTextChanged) onTextChanged(text);
}

void UITextInput::Render() {
    if (!visible) return;
    
    std::string displayText = text;
    if (password) {
        displayText = std::string(text.length(), '*');
    }
    
    if (displayText.empty() && !focused) {
        displayText = placeholder;
    }
    
    // Would render input box and text
    std::cout << "[UI] TextInput '" << name << "': " << displayText 
              << (focused && cursorVisible ? "|" : "") << std::endl;
    
    UIElement::Render();
}

bool UITextInput::HandleEvent(const UIEvent& event) {
    if (!visible || !enabled) return false;
    
    if (event.type == UIEventType::Click) {
        if (ContainsPoint(event.position)) {
            focused = true;
            return true;
        } else {
            focused = false;
        }
    }
    
    if (focused) {
        if (event.type == UIEventType::TextInput) {
            if (static_cast<int>(text.length()) < maxLength) {
                SetText(text + event.text);
            }
            return true;
        }
        
        if (event.type == UIEventType::Press && event.keyCode == 257) { // Enter
            if (onSubmit) onSubmit(text);
            return true;
        }
    }
    
    return UIElement::HandleEvent(event);
}

void UITextInput::Update(float deltaTime) {
    if (focused) {
        cursorBlinkTimer += deltaTime;
        if (cursorBlinkTimer > 0.5f) {
            cursorVisible = !cursorVisible;
            cursorBlinkTimer = 0;
        }
    }
    UIElement::Update(deltaTime);
}

// ============================================
// UIPanel Implementation
// ============================================

UIPanel::UIPanel(const std::string& name)
    : UIElement(name)
    , scrollable(false)
    , scrollOffset(0, 0)
    , contentSize(0, 0)
    , autoLayout(false)
    , padding(10)
    , spacing(5) {}

void UIPanel::Render() {
    if (!visible) return;
    
    // Would render panel background
    std::cout << "[UI] Panel '" << name << "'" << std::endl;
    
    UIElement::Render();
}

void UIPanel::Layout() {
    if (autoLayout) {
        float y = padding;
        for (auto& child : children) {
            child->SetPosition(Vec2(padding, y));
            y += child->GetSize().y + spacing;
        }
        contentSize = Vec2(GetSize().x - padding * 2, y - spacing);
    }
    UIElement::Layout();
}

bool UIPanel::HandleEvent(const UIEvent& event) {
    if (!visible || !enabled) return false;
    
    // Handle scrolling
    if (scrollable && event.type == UIEventType::Scroll) {
        scrollOffset = Vec2(
            scrollOffset.x + event.delta.x,
            scrollOffset.y + event.delta.y
        );
        // Clamp scroll
        scrollOffset.x = std::max(0.0f, std::min(scrollOffset.x, contentSize.x - GetSize().x));
        scrollOffset.y = std::max(0.0f, std::min(scrollOffset.y, contentSize.y - GetSize().y));
        return true;
    }
    
    return UIElement::HandleEvent(event);
}

// ============================================
// UIImage Implementation
// ============================================

UIImage::UIImage(const std::string& name, const std::string& texture)
    : UIElement(name)
    , texturePath(texture)
    , uvMin(0, 0)
    , uvMax(1, 1)
    , preserveAspect(true) {}

void UIImage::Render() {
    if (!visible) return;
    
    // Would render image texture
    std::cout << "[UI] Image '" << name << "': " << texturePath << std::endl;
    
    UIElement::Render();
}

// ============================================
// UICheckbox Implementation
// ============================================

UICheckbox::UICheckbox(const std::string& name, const std::string& label)
    : UIElement(name)
    , checked(false)
    , label(label) {
    SetSize(Vec2(20, 20));
}

void UICheckbox::SetChecked(bool c) {
    if (checked != c) {
        checked = c;
        if (onCheckedChanged) onCheckedChanged(checked);
    }
}

void UICheckbox::Render() {
    if (!visible) return;
    
    std::cout << "[UI] Checkbox '" << name << "': " << (checked ? "[X]" : "[ ]") 
              << " " << label << std::endl;
    
    UIElement::Render();
}

bool UICheckbox::HandleEvent(const UIEvent& event) {
    if (!visible || !enabled) return false;
    
    if (event.type == UIEventType::Click && ContainsPoint(event.position)) {
        SetChecked(!checked);
        OnClick();
        return true;
    }
    
    return UIElement::HandleEvent(event);
}

// ============================================
// UIDropdown Implementation
// ============================================

UIDropdown::UIDropdown(const std::string& name)
    : UIElement(name)
    , selectedIndex(-1)
    , expanded(false) {
    SetSize(Vec2(120, 25));
}

void UIDropdown::AddOption(const std::string& option) {
    options.push_back(option);
    if (selectedIndex < 0) {
        selectedIndex = 0;
    }
}

void UIDropdown::RemoveOption(int index) {
    if (index >= 0 && static_cast<size_t>(index) < options.size()) {
        options.erase(options.begin() + index);
        if (selectedIndex >= static_cast<int>(options.size())) {
            selectedIndex = static_cast<int>(options.size()) - 1;
        }
    }
}

void UIDropdown::ClearOptions() {
    options.clear();
    selectedIndex = -1;
}

void UIDropdown::SetSelectedIndex(int index) {
    if (index >= 0 && static_cast<size_t>(index) < options.size() && index != selectedIndex) {
        selectedIndex = index;
        if (onSelectionChanged) {
            onSelectionChanged(selectedIndex, options[selectedIndex]);
        }
    }
}

std::string UIDropdown::GetSelectedOption() const {
    if (selectedIndex >= 0 && static_cast<size_t>(selectedIndex) < options.size()) {
        return options[selectedIndex];
    }
    return "";
}

void UIDropdown::Render() {
    if (!visible) return;
    
    std::string display = selectedIndex >= 0 ? options[selectedIndex] : "Select...";
    std::cout << "[UI] Dropdown '" << name << "': " << display 
              << (expanded ? " [v]" : " [>") << std::endl;
    
    if (expanded) {
        for (size_t i = 0; i < options.size(); ++i) {
            std::cout << "  " << (static_cast<int>(i) == selectedIndex ? "> " : "  ") << options[i] << std::endl;
        }
    }
    
    UIElement::Render();
}

bool UIDropdown::HandleEvent(const UIEvent& event) {
    if (!visible || !enabled) return false;
    
    if (event.type == UIEventType::Click) {
        if (ContainsPoint(event.position)) {
            expanded = !expanded;
            return true;
        } else if (expanded) {
            // Check if clicked on option
            Vec2 absPos = GetAbsolutePosition();
            float optionY = event.position.y - (absPos.y + GetSize().y);
            int optionIndex = static_cast<int>(optionY / GetSize().y);
            
            if (optionIndex >= 0 && static_cast<size_t>(optionIndex) < options.size()) {
                SetSelectedIndex(optionIndex);
                expanded = false;
                return true;
            }
            
            expanded = false;
        }
    }
    
    return UIElement::HandleEvent(event);
}

// ============================================
// UIManager Implementation
// ============================================

UIManager::UIManager()
    : focusedElement(nullptr)
    , hoveredElement(nullptr)
    , screenSize(800, 600)
    , scale(1.0f) {}

UIManager::~UIManager() {
    Clear();
}

UIElement* UIManager::CreateElement(const std::string& type, const std::string& name) {
    if (type == "button") return CreateButton(name, "");
    if (type == "label") return CreateLabel(name, "");
    if (type == "slider") return CreateSlider(name, 0, 1);
    if (type == "textinput") return CreateTextInput(name, "");
    if (type == "panel") return CreatePanel(name);
    if (type == "image") return CreateImage(name, "");
    if (type == "checkbox") return CreateCheckbox(name, "");
    if (type == "dropdown") return CreateDropdown(name);
    return nullptr;
}

UIButton* UIManager::CreateButton(const std::string& name, const std::string& text) {
    auto button = std::make_unique<UIButton>(name, text);
    UIButton* ptr = button.get();
    AddRootElement(std::move(button));
    return ptr;
}

UILabel* UIManager::CreateLabel(const std::string& name, const std::string& text) {
    auto label = std::make_unique<UILabel>(name, text);
    UILabel* ptr = label.get();
    AddRootElement(std::move(label));
    return ptr;
}

UISlider* UIManager::CreateSlider(const std::string& name, float min, float max) {
    auto slider = std::make_unique<UISlider>(name, min, max);
    UISlider* ptr = slider.get();
    AddRootElement(std::move(slider));
    return ptr;
}

UITextInput* UIManager::CreateTextInput(const std::string& name, const std::string& placeholder) {
    auto input = std::make_unique<UITextInput>(name, placeholder);
    UITextInput* ptr = input.get();
    AddRootElement(std::move(input));
    return ptr;
}

UIPanel* UIManager::CreatePanel(const std::string& name) {
    auto panel = std::make_unique<UIPanel>(name);
    UIPanel* ptr = panel.get();
    AddRootElement(std::move(panel));
    return ptr;
}

UIImage* UIManager::CreateImage(const std::string& name, const std::string& texture) {
    auto image = std::make_unique<UIImage>(name, texture);
    UIImage* ptr = image.get();
    AddRootElement(std::move(image));
    return ptr;
}

UICheckbox* UIManager::CreateCheckbox(const std::string& name, const std::string& label) {
    auto checkbox = std::make_unique<UICheckbox>(name, label);
    UICheckbox* ptr = checkbox.get();
    AddRootElement(std::move(checkbox));
    return ptr;
}

UIDropdown* UIManager::CreateDropdown(const std::string& name) {
    auto dropdown = std::make_unique<UIDropdown>(name);
    UIDropdown* ptr = dropdown.get();
    AddRootElement(std::move(dropdown));
    return ptr;
}

void UIManager::AddRootElement(std::unique_ptr<UIElement> element) {
    rootElements.push_back(std::move(element));
}

void UIManager::RemoveElement(UIElement* element) {
    rootElements.erase(
        std::remove_if(rootElements.begin(), rootElements.end(),
            [element](const std::unique_ptr<UIElement>& e) { return e.get() == element; }),
        rootElements.end()
    );
}

UIElement* UIManager::FindElement(const std::string& name) {
    for (auto& element : rootElements) {
        if (element->GetName() == name) {
            return element.get();
        }
        UIElement* found = element->FindChild(name);
        if (found) return found;
    }
    return nullptr;
}

void UIManager::HandleMouseMove(const Vec2& position) {
    UIEvent event;
    event.type = UIEventType::Hover;
    event.position = position;
    
    // Find hovered element
    UIElement* newHovered = nullptr;
    for (auto it = rootElements.rbegin(); it != rootElements.rend(); ++it) {
        if ((*it)->IsVisible() && (*it)->ContainsPoint(position)) {
            newHovered = it->get();
            break;
        }
    }
    
    if (newHovered != hoveredElement) {
        if (hoveredElement) hoveredElement->SetHovered(false);
        hoveredElement = newHovered;
        if (hoveredElement) hoveredElement->SetHovered(true);
    }
    
    // Send event
    for (auto it = rootElements.rbegin(); it != rootElements.rend(); ++it) {
        if ((*it)->HandleEvent(event)) break;
    }
}

void UIManager::HandleMouseClick(const Vec2& position, bool pressed) {
    UIEvent event;
    event.type = pressed ? UIEventType::Press : UIEventType::Release;
    event.position = position;
    
    for (auto it = rootElements.rbegin(); it != rootElements.rend(); ++it) {
        if ((*it)->HandleEvent(event)) {
            if (pressed) {
                SetFocus(it->get());
            }
            break;
        }
    }
    
    if (!pressed) {
        // Also send click event
        UIEvent clickEvent;
        clickEvent.type = UIEventType::Click;
        clickEvent.position = position;
        
        for (auto it = rootElements.rbegin(); it != rootElements.rend(); ++it) {
            if ((*it)->HandleEvent(clickEvent)) break;
        }
    }
}

void UIManager::HandleTextInput(const std::string& text) {
    if (focusedElement) {
        UIEvent event;
        event.type = UIEventType::TextInput;
        event.text = text;
        focusedElement->HandleEvent(event);
    }
}

void UIManager::HandleKeyInput(int keyCode, bool pressed) {
    if (focusedElement) {
        UIEvent event;
        event.type = pressed ? UIEventType::Press : UIEventType::Release;
        event.keyCode = keyCode;
        focusedElement->HandleEvent(event);
    }
}

void UIManager::SetFocus(UIElement* element) {
    focusedElement = element;
}

void UIManager::Update(float deltaTime) {
    for (auto& element : rootElements) {
        element->Update(deltaTime);
    }
}

void UIManager::Render() {
    for (auto& element : rootElements) {
        element->Render();
    }
}

void UIManager::Layout() {
    for (auto& element : rootElements) {
        element->Layout();
    }
}

void UIManager::Clear() {
    rootElements.clear();
    focusedElement = nullptr;
    hoveredElement = nullptr;
}

} // namespace vge