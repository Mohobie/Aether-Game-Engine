#pragma once
#include "ui/ui_system.h"
#include "math/vec3.h"
#include <functional>

namespace vge {

// ============================================
// HUD System - Health, Hunger, Hotbar
// ============================================
class HUDSystem {
public:
    HUDSystem(UIManager* ui);
    ~HUDSystem();

    // Initialize HUD elements
    void Initialize();
    
    // Update values
    void SetHealth(float current, float max);
    void SetHunger(float current, float max);
    void SetStamina(float current, float max);
    void SetExperience(float current, float max);
    void SetHotbarSlot(int slot);
    void SetHotbarItem(int slot, const std::string& itemId, int count);
    
    // Show/hide
    void Show();
    void Hide();
    bool IsVisible() const;

private:
    UIManager* ui;
    bool visible;
    
    // Health bar
    UIPanel* healthBarBg;
    UIPanel* healthBarFill;
    UILabel* healthText;
    
    // Hunger bar
    UIPanel* hungerBarBg;
    UIPanel* hungerBarFill;
    UILabel* hungerText;
    
    // Stamina bar
    UIPanel* staminaBarBg;
    UIPanel* staminaBarFill;
    
    // Experience bar
    UIPanel* expBarBg;
    UIPanel* expBarFill;
    
    // Hotbar
    std::vector<UIPanel*> hotbarSlots;
    std::vector<UILabel*> hotbarLabels;
    UIPanel* hotbarSelector;
    
    void CreateHealthBar();
    void CreateHungerBar();
    void CreateStaminaBar();
    void CreateExperienceBar();
    void CreateHotbar();
};

} // namespace vge
