# UI System

## Quick Reference

```cpp
vge::UIManager ui;
ui.SetScreenSize(vge::Vec2(1280, 720));

// Create button
auto* button = ui.CreateButton("startBtn", "Start Game");
button->SetPosition(vge::Vec2(100, 100));
button->SetSize(vge::Vec2(200, 50));
button->SetOnClick([]() {
    std::cout << "Game started!" << std::endl;
});

// Update and render
ui.Update(deltaTime);
ui.Render();
```

## UI Elements

| Element | Description | Use Case |
|---------|-------------|----------|
| `UIButton` | Clickable button | Menu buttons, actions |
| `UILabel` | Text display | HUD text, descriptions |
| `UISlider` | Value slider | Volume, brightness |
| `UITextInput` | Text entry | Chat, naming |
| `UIPanel` | Container | Windows, menus |
| `UIImage` | Image display | Icons, backgrounds |
| `UICheckbox` | Toggle option | Settings |
| `UIDropdown` | Select list | Options, filters |

## Creating UI

```cpp
// Panel (container)
auto* panel = ui.CreatePanel("menuPanel");
panel->SetPosition(vge::Vec2(50, 50));
panel->SetSize(vge::Vec2(300, 400));
panel->SetBackgroundColor(vge::Vec3(0.2f, 0.2f, 0.2f));
panel->SetAlpha(0.9f);

// Label
auto* title = ui.CreateLabel("title", "Main Menu");
title->SetPosition(vge::Vec2(20, 20));
title->SetFontSize(24.0f);
panel->AddChild(std::unique_ptr<vge::UIElement>(title));

// Button
auto* playBtn = ui.CreateButton("playBtn", "Play");
playBtn->SetPosition(vge::Vec2(50, 100));
playBtn->SetSize(vge::Vec2(200, 40));
playBtn->SetOnClick([]() {
    // Start game
});
panel->AddChild(std::unique_ptr<vge::UIElement>(playBtn));

// Slider
auto* volumeSlider = ui.CreateSlider("volume", 0.0f, 1.0f);
volumeSlider->SetPosition(vge::Vec2(50, 200));
volumeSlider->SetSize(vge::Vec2(200, 20));
volumeSlider->SetValue(0.5f);
volumeSlider->SetOnValueChanged([](float value) {
    audio.SetMasterVolume(value);
});
panel->AddChild(std::unique_ptr<vge::UIElement>(volumeSlider));
```

## Event Handling

```cpp
// Mouse input
ui.HandleMouseMove(mousePos);
ui.HandleMouseClick(mousePos, true);  // Press
ui.HandleMouseClick(mousePos, false); // Release

// Keyboard input
ui.HandleTextInput("Hello");
ui.HandleKeyInput(GLFW_KEY_ENTER, true);
```

## Styling

```cpp
element->SetBackgroundColor(vge::Vec3(0.3f, 0.5f, 0.8f));
element->SetForegroundColor(vge::Vec3(1.0f, 1.0f, 1.0f));
element->SetHoverColor(vge::Vec3(0.4f, 0.6f, 0.9f));
element->SetPressedColor(vge::Vec3(0.2f, 0.4f, 0.7f));
element->SetAlpha(0.8f);
element->SetBorderRadius(5.0f);
element->SetBorderWidth(2.0f);
element->SetBorderColor(vge::Vec3(0.5f, 0.5f, 0.5f));
```

## HUD Example

```cpp
// Health bar
auto* healthBar = ui.CreatePanel("healthBar");
healthBar->SetPosition(vge::Vec2(20, 20));
healthBar->SetSize(vge::Vec2(200, 20));
healthBar->SetBackgroundColor(vge::Vec3(0.8f, 0.2f, 0.2f));

// Hunger bar
auto* hungerBar = ui.CreatePanel("hungerBar");
hungerBar->SetPosition(vge::Vec2(20, 50));
hungerBar->SetSize(vge::Vec2(200, 20));
hungerBar->SetBackgroundColor(vge::Vec3(0.8f, 0.6f, 0.2f));

// Crosshair (center screen)
auto* crosshair = ui.CreateLabel("crosshair", "+");
crosshair->SetPosition(vge::Vec2(640, 360));
crosshair->SetFontSize(24.0f);
```

## Files
- `src/ui/ui_system.h`
- `src/ui/ui_system.cpp`
