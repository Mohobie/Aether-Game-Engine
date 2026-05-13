#pragma once
#include "ui/ui_system.h"
#include <functional>
#include <string>

namespace vge {

// ============================================
// Main Menu
// ============================================
class MainMenu {
public:
    MainMenu(UIManager* ui);
    ~MainMenu();

    // Initialize menu UI
    void Initialize();
    
    // Show/hide
    void Show();
    void Hide();
    bool IsVisible() const;
    
    // Callbacks
    void SetOnSingleplayer(std::function<void()> callback);
    void SetOnMultiplayer(std::function<void()> callback);
    void SetOnSettings(std::function<void()> callback);
    void SetOnQuit(std::function<void()> callback);

private:
    UIManager* ui;
    UIPanel* panel;
    bool visible;
    
    // UI elements
    UILabel* titleLabel;
    UIButton* singleplayerBtn;
    UIButton* multiplayerBtn;
    UIButton* settingsBtn;
    UIButton* quitBtn;
    
    // Callbacks
    std::function<void()> onSingleplayer;
    std::function<void()> onMultiplayer;
    std::function<void()> onSettings;
    std::function<void()> onQuit;
    
    void CreateUI();
};

} // namespace vge
