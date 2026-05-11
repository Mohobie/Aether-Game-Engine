# UI System

The engine includes a comprehensive UI system for menus, HUD, and in-game interfaces.

## UI Elements

```cpp
#include "ui/ui_element.h"
#include "ui/ui_system.h"

// Create UI system
vge::UISystem ui;
ui.Initialize(renderer);

// Create canvas (root element)
vge::UIElement* canvas = ui.GetCanvas();
canvas->SetSize(1920, 1080);

// Panel (container)
vge::UIElement* panel = canvas->AddChild<vge::UIElement>();
panel->SetPosition(100, 100);
panel->SetSize(400, 300);
panel->SetBackgroundColor(vge::Vec4(0.1f, 0.1f, 0.1f, 0.9f));
panel->SetBorder(2, vge::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
panel->SetCornerRadius(8);

// Text
vge::UIText* title = panel->AddChild<vge::UIText>();
title->SetText("Inventory");
title->SetFont("fonts/roboto-bold.ttf", 24);
title->SetColor(vge::Vec4(1, 1, 1, 1));
title->SetPosition(20, 20);

// Button
vge::UIButton* button = panel->AddChild<vge::UIButton>();
button->SetPosition(20, 80);
button->SetSize(200, 40);
button->SetText("Craft Item");
button->SetFont("fonts/roboto.ttf", 16);
button->SetBackgroundColor(vge::Vec4(0.2f, 0.6f, 1.0f, 1.0f));
button->SetHoverColor(vge::Vec4(0.3f, 0.7f, 1.0f, 1.0f));
button->SetPressedColor(vge::Vec4(0.1f, 0.5f, 0.9f, 1.0f));
button->OnClick([]() {
    CraftSelectedItem();
});

// Image
vge::UIImage* icon = panel->AddChild<vge::UIImage>();
icon->SetTexture("textures/sword.png");
icon->SetPosition(20, 140);
icon->SetSize(64, 64);

// Progress bar
vge::UIProgressBar* healthBar = panel->AddChild<vge::UIProgressBar>();
healthBar->SetPosition(20, 220);
healthBar->SetSize(360, 20);
healthBar->SetValue(75.0f);  // 75%
healthBar->SetFillColor(vge::Vec4(1.0f, 0.2f, 0.2f, 1.0f));
healthBar->SetBackgroundColor(vge::Vec4(0.2f, 0.2f, 0.2f, 1.0f));

// Slider
vge::UISlider* volumeSlider = panel->AddChild<vge::UISlider>();
volumeSlider->SetPosition(20, 260);
volumeSlider->SetSize(360, 30);
volumeSlider->SetRange(0.0f, 100.0f);
volumeSlider->SetValue(50.0f);
volumeSlider->OnValueChanged([](float value) {
    audio.SetMasterVolume(value / 100.0f);
});

// Checkbox
vge::UICheckbox* fullscreenCheck = panel->AddChild<vge::UICheckbox>();
fullscreenCheck->SetPosition(20, 300);
fullscreenCheck->SetText("Fullscreen");
fullscreenCheck->SetChecked(false);
fullscreenCheck->OnToggle([](bool checked) {
    renderer.SetFullscreen(checked);
});

// Text input
vge::UITextInput* nameInput = panel->AddChild<vge::UITextInput>();
nameInput->SetPosition(20, 340);
nameInput->SetSize(360, 30);
nameInput->SetPlaceholder("Enter player name...");
nameInput->SetMaxLength(20);
```

## Layout System

```cpp
// Horizontal layout
vge::UIHorizontalLayout* hLayout = panel->AddChild<vge::UIHorizontalLayout>();
hLayout->SetSpacing(10);
hLayout->SetPadding(20, 20);
hLayout->SetAlignment(UILayoutAlignment::Center);

vge::UIButton* btn1 = hLayout->AddChild<vge::UIButton>();
btn1->SetText("Button 1");
btn1->SetSize(100, 40);

vge::UIButton* btn2 = hLayout->AddChild<vge::UIButton>();
btn2->SetText("Button 2");
btn2->SetSize(100, 40);

vge::UIButton* btn3 = hLayout->AddChild<vge::UIButton>();
btn3->SetText("Button 3");
btn3->SetSize(100, 40);

// Vertical layout
vge::UIVerticalLayout* vLayout = panel->AddChild<vge::UIVerticalLayout>();
vLayout->SetSpacing(5);

// Grid layout (for inventory)
vge::UIGridLayout* grid = panel->AddChild<vge::UIGridLayout>();
grid->SetColumns(9);  // 9 slots like Minecraft
grid->SetCellSize(64, 64);
grid->SetSpacing(4);

for (int i = 0; i < 36; ++i) {
    vge::UIItemSlot* slot = grid->AddChild<vge::UIItemSlot>();
    slot->SetItem(inventory.GetItem(i));
}
```

## Inventory UI

```cpp
class InventoryUI {
    vge::UIElement* inventoryPanel;
    std::vector<vge::UIItemSlot*> slots;
    
public:
    void Initialize(vge::UISystem* ui, vge::Inventory* inventory) {
        // Background panel
        inventoryPanel = ui->GetCanvas()->AddChild<vge::UIElement>();
        inventoryPanel->SetSize(400, 300);
        inventoryPanel->SetPosition(
            (1920 - 400) / 2,
            (1080 - 300) / 2
        );
        inventoryPanel->SetBackgroundColor(vge::Vec4(0, 0, 0, 0.8f));
        inventoryPanel->SetVisible(false);
        
        // Title
        vge::UIText* title = inventoryPanel->AddChild<vge::UIText>();
        title->SetText("Inventory");
        title->SetPosition(20, 20);
        title->SetFontSize(24);
        
        // Grid for items
        vge::UIGridLayout* grid = inventoryPanel->AddChild<vge::UIGridLayout>();
        grid->SetPosition(20, 60);
        grid->SetColumns(9);
        grid->SetCellSize(48, 48);
        grid->SetSpacing(4);
        
        // Create slots
        for (int i = 0; i < inventory->GetCapacity(); ++i) {
            vge::UIItemSlot* slot = grid->AddChild<vge::UIItemSlot>();
            slot->SetSlotIndex(i);
            slot->OnClick([this, i]() {
                OnSlotClicked(i);
            });
            slot->OnRightClick([this, i]() {
                OnSlotRightClicked(i);
            });
            slots.push_back(slot);
        }
        
        // Hotbar (always visible)
        vge::UIElement* hotbar = ui->GetCanvas()->AddChild<vge::UIElement>();
        hotbar->SetSize(9 * 52, 56);
        hotbar->SetPosition((1920 - 9 * 52) / 2, 1080 - 80);
        
        for (int i = 0; i < 9; ++i) {
            vge::UIItemSlot* slot = hotbar->AddChild<vge::UIItemSlot>();
            slot->SetSlotIndex(i);
            slot->SetSize(48, 48);
            slot->SetPosition(i * 52, 4);
        }
    }
    
    void Update(vge::Inventory* inventory) {
        // Update slot visuals
        for (int i = 0; i < slots.size(); ++i) {
            const ItemStack& stack = inventory->GetStack(i);
            slots[i]->SetItem(stack.item);
            slots[i]->SetCount(stack.count);
        }
    }
    
    void Show() {
        inventoryPanel->SetVisible(true);
        input.SetCursorLocked(false);
    }
    
    void Hide() {
        inventoryPanel->SetVisible(false);
        input.SetCursorLocked(true);
    }
    
    void Toggle() {
        if (inventoryPanel->IsVisible()) {
            Hide();
        } else {
            Show();
        }
    }
    
private:
    void OnSlotClicked(int slot) {
        // Handle item pickup/placement
        if (heldItem.IsEmpty()) {
            heldItem = inventory->TakeItem(slot);
        } else {
            heldItem = inventory->PlaceItem(slot, heldItem);
        }
    }
    
    void OnSlotRightClicked(int slot) {
        // Handle half-stack placement
        if (!heldItem.IsEmpty()) {
            heldItem = inventory->PlaceOneItem(slot, heldItem);
        }
    }
    
    ItemStack heldItem;
};
```

## HUD (Heads-Up Display)

```cpp
class HUD {
    vge::UISystem* ui;
    
    // Health bar
    vge::UIProgressBar* healthBar;
    vge::UIText* healthText;
    
    // Crosshair
    vge::UIElement* crosshair;
    
    // Hotbar
    vge::UIElement* hotbar;
    std::vector<vge::UIItemSlot*> hotbarSlots;
    vge::UIElement* hotbarSelector;
    
    // Debug info
    vge::UIText* fpsText;
    vge::UIText* coordsText;
    
public:
    void Initialize(vge::UISystem* uiSystem) {
        ui = uiSystem;
        
        // Health bar (bottom left)
        healthBar = ui->GetCanvas()->AddChild<vge::UIProgressBar>();
        healthBar->SetPosition(20, 1080 - 60);
        healthBar->SetSize(200, 20);
        healthBar->SetBackgroundColor(vge::Vec4(0.2f, 0, 0, 0.8f));
        healthBar->SetFillColor(vge::Vec4(1, 0, 0, 1));
        
        healthText = ui->GetCanvas()->AddChild<vge::UIText>();
        healthText->SetPosition(20, 1080 - 85);
        healthText->SetColor(vge::Vec4(1, 1, 1, 1));
        
        // Crosshair (center)
        crosshair = ui->GetCanvas()->AddChild<vge::UIElement>();
        crosshair->SetSize(4, 4);
        crosshair->SetPosition(1920/2 - 2, 1080/2 - 2);
        crosshair->SetBackgroundColor(vge::Vec4(1, 1, 1, 0.8f));
        
        // Hotbar (bottom center)
        hotbar = ui->GetCanvas()->AddChild<vge::UIElement>();
        hotbar->SetSize(9 * 52, 56);
        hotbar->SetPosition((1920 - 9 * 52) / 2, 1080 - 80);
        
        for (int i = 0; i < 9; ++i) {
            vge::UIItemSlot* slot = hotbar->AddChild<vge::UIItemSlot>();
            slot->SetSize(48, 48);
            slot->SetPosition(i * 52, 4);
            hotbarSlots.push_back(slot);
        }
        
        // Selection indicator
        hotbarSelector = hotbar->AddChild<vge::UIElement>();
        hotbarSelector->SetSize(52, 52);
        hotbarSelector->SetPosition(0, 2);
        hotbarSelector->SetBorder(2, vge::Vec4(1, 1, 1, 0.8f));
        
        // Debug text (top left)
        fpsText = ui->GetCanvas()->AddChild<vge::UIText>();
        fpsText->SetPosition(10, 10);
        fpsText->SetColor(vge::Vec4(0, 1, 0, 1));
        fpsText->SetFontSize(14);
        
        coordsText = ui->GetCanvas()->AddChild<vge::UIText>();
        coordsText->SetPosition(10, 30);
        coordsText->SetColor(vge::Vec4(0, 1, 0, 1));
        coordsText->SetFontSize(14);
    }
    
    void Update(const Player& player, float fps) {
        // Update health
        float healthPercent = (player.GetHealth() / player.GetMaxHealth()) * 100;
        healthBar->SetValue(healthPercent);
        healthText->SetText(std::to_string((int)player.GetHealth()) + " / " +
                           std::to_string((int)player.GetMaxHealth()));
        
        // Update hotbar selection
        int selectedSlot = player.GetSelectedHotbarSlot();
        hotbarSelector->SetPosition(selectedSlot * 52, 2);
        
        // Update hotbar items
        for (int i = 0; i < 9; ++i) {
            hotbarSlots[i]->SetItem(player.GetHotbarItem(i));
        }
        
        // Update debug info
        fpsText->SetText("FPS: " + std::to_string((int)fps));
        vge::Vec3 pos = player.GetPosition();
        coordsText->SetText("X: " + std::to_string((int)pos.x) +
                           " Y: " + std::to_string((int)pos.y) +
                           " Z: " + std::to_string((int)pos.z));
    }
};
```

## Console

```cpp
#include "ui/console_commands.h"

class GameConsole {
    vge::UIElement* consolePanel;
    vge::UIText* logText;
    vge::UITextInput* inputField;
    std::vector<std::string> commandHistory;
    int historyIndex = -1;
    
public:
    void Initialize(vge::UISystem* ui) {
        // Console panel (top half of screen)
        consolePanel = ui->GetCanvas()->AddChild<vge::UIElement>();
        consolePanel->SetSize(1920, 540);
        consolePanel->SetPosition(0, 0);
        consolePanel->SetBackgroundColor(vge::Vec4(0, 0, 0, 0.8f));
        consolePanel->SetVisible(false);
        
        // Log output
        logText = consolePanel->AddChild<vge::UIText>();
        logText->SetPosition(10, 10);
        logText->SetSize(1900, 480);
        logText->SetColor(vge::Vec4(1, 1, 1, 1));
        logText->SetFontSize(14);
        logText->SetMultiline(true);
        
        // Input field
        inputField = consolePanel->AddChild<vge::UITextInput>();
        inputField->SetPosition(10, 500);
        inputField->SetSize(1900, 30);
        inputField->SetBackgroundColor(vge::Vec4(0.1f, 0.1f, 0.1f, 1));
        inputField->SetTextColor(vge::Vec4(0, 1, 0, 1));
        
        // Register commands
        RegisterCommand("help", [this](const std::vector<std::string>& args) {
            Log("Available commands:");
            Log("  help - Show this help");
            Log("  give [item] [count] - Give items");
            Log("  teleport [x] [y] [z] - Teleport player");
            Log("  time [value] - Set time of day");
            Log("  weather [clear/rain/storm] - Set weather");
            Log("  godmode - Toggle invincibility");
            Log("  fly - Toggle flight mode");
            Log("  kill - Kill player");
            Log("  save - Save game");
            Log("  load - Load game");
        });
        
        RegisterCommand("give", [this](const std::vector<std::string>& args) {
            if (args.size() < 2) {
                Log("Usage: give [item] [count]");
                return;
            }
            std::string item = args[1];
            int count = (args.size() > 2) ? std::stoi(args[2]) : 1;
            player.GiveItem(item, count);
            Log("Gave " + std::to_string(count) + "x " + item);
        });
        
        RegisterCommand("teleport", [this](const std::vector<std::string>& args) {
            if (args.size() < 4) {
                Log("Usage: teleport [x] [y] [z]");
                return;
            }
            float x = std::stof(args[1]);
            float y = std::stof(args[2]);
            float z = std::stof(args[3]);
            player.Teleport(vge::Vec3(x, y, z));
            Log("Teleported to " + std::to_string(x) + ", " +
                std::to_string(y) + ", " + std::to_string(z));
        });
        
        RegisterCommand("godmode", [this](const std::vector<std::string>& args) {
            player.SetGodMode(!player.IsGodMode());
            Log(player.IsGodMode() ? "God mode enabled" : "God mode disabled");
        });
    }
    
    void Show() {
        consolePanel->SetVisible(true);
        input->SetCursorLocked(false);
        input->StartTextInput();
    }
    
    void Hide() {
        consolePanel->SetVisible(false);
        input->SetCursorLocked(true);
        input->StopTextInput();
    }
    
    void Toggle() {
        if (consolePanel->IsVisible()) {
            Hide();
        } else {
            Show();
        }
    }
    
    void Log(const std::string& message) {
        logText->AppendText(message + "\n");
    }
    
    void ExecuteCommand(const std::string& command) {
        // Parse command
        std::vector<std::string> args;
        std::stringstream ss(command);
        std::string arg;
        while (ss >> arg) {
            args.push_back(arg);
        }
        
        if (args.empty()) return;
        
        // Add to history
        commandHistory.push_back(command);
        historyIndex = commandHistory.size();
        
        // Execute
        auto it = commands.find(args[0]);
        if (it != commands.end()) {
            it->second(args);
        } else {
            Log("Unknown command: " + args[0]);
        }
    }
    
    void OnInput(const std::string& text) {
        if (text == "\n" || text == "\r") {
            ExecuteCommand(inputField->GetText());
            inputField->SetText("");
        } else if (text == "\t") {
            // Auto-complete
            AutoComplete();
        }
    }
    
    void OnKeyDown(Key key) {
        if (key == Key::Up) {
            // Previous command
            if (historyIndex > 0) {
                historyIndex--;
                inputField->SetText(commandHistory[historyIndex]);
            }
        } else if (key == Key::Down) {
            // Next command
            if (historyIndex < commandHistory.size() - 1) {
                historyIndex++;
                inputField->SetText(commandHistory[historyIndex]);
            } else {
                inputField->SetText("");
            }
        }
    }
    
private:
    std::unordered_map<std::string, std::function<void(const std::vector<std::string>&)>> commands;
    
    void RegisterCommand(const std::string& name, std::function<void(const std::vector<std::string>&)> func) {
        commands[name] = func;
    }
    
    void AutoComplete() {
        std::string partial = inputField->GetText();
        for (const auto& cmd : commands) {
            if (cmd.first.find(partial) == 0) {
                inputField->SetText(cmd.first + " ");
                break;
            }
        }
    }
};
```

## Animations

```cpp
// Fade in/out
panel->FadeIn(0.3f);   // 300ms fade in
panel->FadeOut(0.3f);  // 300ms fade out

// Slide in from bottom
panel->SlideIn(vge::Vec2(0, 1080), vge::Vec2(0, 0), 0.5f);

// Scale animation
panel->ScaleFrom(vge::Vec2(0, 0), vge::Vec2(1, 1), 0.3f);

// Color transition
panel->ColorTransition(
    vge::Vec4(1, 0, 0, 1),  // From red
    vge::Vec4(0, 1, 0, 1),  // To green
    1.0f                     // Over 1 second
);

// Chained animations
panel->Sequence()
    .FadeIn(0.3f)
    .Wait(1.0f)
    .MoveTo(vge::Vec2(100, 100), 0.5f)
    .ScaleTo(vge::Vec2(1.5f, 1.5f), 0.3f)
    .Wait(0.5f)
    .FadeOut(0.3f);
```
