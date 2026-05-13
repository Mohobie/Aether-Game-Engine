#pragma once
#include "ui/ui_system.h"
#include <functional>

namespace vge {

// ============================================
// Settings Menu
// ============================================
class SettingsMenu {
public:
    SettingsMenu(UIManager* ui);
    ~SettingsMenu();

    // Initialize settings UI
    void Initialize();
    
    // Show/hide
    void Show();
    void Hide();
    bool IsVisible() const;
    
    // Toggle visibility
    void Toggle();

    // Settings callbacks
    void SetOnVolumeChanged(std::function<void(float)> callback);
    void SetOnSensitivityChanged(std::function<void(float)> callback);
    void SetOnRenderDistanceChanged(std::function<void(int)> callback);
    void SetOnFullscreenChanged(std::function<void(bool)> callback);
    void SetOnVSyncChanged(std::function<void(bool)> callback);
    void SetOnBack(std::function<void()> callback);

    // Update from current settings
    void UpdateSettings(float volume, float sensitivity, int renderDist, bool fullscreen, bool vsync);

private:
    UIManager* ui;
    UIPanel* panel;
    bool visible;
    
    // UI elements
    UISlider* volumeSlider;
    UISlider* sensitivitySlider;
    UISlider* renderDistanceSlider;
    UICheckbox* fullscreenCheckbox;
    UICheckbox* vsyncCheckbox;
    UIButton* backButton;
    
    // Callbacks
    std::function<void(float)> onVolumeChanged;
    std::function<void(float)> onSensitivityChanged;
    std::function<void(int)> onRenderDistanceChanged;
    std::function<void(bool)> onFullscreenChanged;
    std::function<void(bool)> onVSyncChanged;
    std::function<void()> onBack;
    
    void CreateUI();
};

} // namespace vge
