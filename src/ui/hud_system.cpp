#include "hud_system.h"
#include "core/logger.h"

namespace vge {

HUDSystem::HUDSystem(UIManager* uiManager)
    : ui(uiManager), visible(false),
      healthBarBg(nullptr), healthBarFill(nullptr), healthText(nullptr),
      hungerBarBg(nullptr), hungerBarFill(nullptr), hungerText(nullptr),
      staminaBarBg(nullptr), staminaBarFill(nullptr),
      expBarBg(nullptr), expBarFill(nullptr),
      hotbarSelector(nullptr) {
}

HUDSystem::~HUDSystem() {
}

void HUDSystem::Initialize() {
    CreateHealthBar();
    CreateHungerBar();
    CreateStaminaBar();
    CreateExperienceBar();
    CreateHotbar();
    Show();
    Logger::Info("[HUDSystem] Initialized");
}

void HUDSystem::CreateHealthBar() {
    if (!ui) return;

    // Background
    healthBarBg = ui->CreatePanel("healthBarBg");
    healthBarBg->SetPosition(vge::Vec2(20, 20));
    healthBarBg->SetSize(vge::Vec2(200, 20));
    healthBarBg->SetBackgroundColor(vge::Vec3(0.2f, 0.2f, 0.2f));
    healthBarBg->SetAlpha(0.8f);
    ui->AddRootElement(std::unique_ptr<vge::UIElement>(healthBarBg));

    // Fill
    healthBarFill = ui->CreatePanel("healthBarFill");
    healthBarFill->SetPosition(vge::Vec2(20, 20));
    healthBarFill->SetSize(vge::Vec2(200, 20));
    healthBarFill->SetBackgroundColor(vge::Vec3(1.0f, 0.2f, 0.2f));
    ui->AddRootElement(std::unique_ptr<vge::UIElement>(healthBarFill));

    // Text
    healthText = ui->CreateLabel("healthText", "20 / 20");
    healthText->SetPosition(vge::Vec2(80, 22));
    healthText->SetFontSize(14.0f);
    ui->AddRootElement(std::unique_ptr<vge::UIElement>(healthText));
}

void HUDSystem::CreateHungerBar() {
    if (!ui) return;

    // Background
    hungerBarBg = ui->CreatePanel("hungerBarBg");
    hungerBarBg->SetPosition(vge::Vec2(20, 50));
    hungerBarBg->SetSize(vge::Vec2(200, 20));
    hungerBarBg->SetBackgroundColor(vge::Vec3(0.2f, 0.2f, 0.2f));
    hungerBarBg->SetAlpha(0.8f);
    ui->AddRootElement(std::unique_ptr<vge::UIElement>(hungerBarBg));

    // Fill
    hungerBarFill = ui->CreatePanel("hungerBarFill");
    hungerBarFill->SetPosition(vge::Vec2(20, 50));
    hungerBarFill->SetSize(vge::Vec2(200, 20));
    hungerBarFill->SetBackgroundColor(vge::Vec3(0.8f, 0.5f, 0.2f));
    ui->AddRootElement(std::unique_ptr<vge::UIElement>(hungerBarFill));

    // Text
    hungerText = ui->CreateLabel("hungerText", "20 / 20");
    hungerText->SetPosition(vge::Vec2(80, 52));
    hungerText->SetFontSize(14.0f);
    ui->AddRootElement(std::unique_ptr<vge::UIElement>(hungerText));
}

void HUDSystem::CreateStaminaBar() {
    if (!ui) return;

    // Background
    staminaBarBg = ui->CreatePanel("staminaBarBg");
    staminaBarBg->SetPosition(vge::Vec2(20, 80));
    staminaBarBg->SetSize(vge::Vec2(200, 10));
    staminaBarBg->SetBackgroundColor(vge::Vec3(0.2f, 0.2f, 0.2f));
    staminaBarBg->SetAlpha(0.8f);
    ui->AddRootElement(std::unique_ptr<vge::UIElement>(staminaBarBg));

    // Fill
    staminaBarFill = ui->CreatePanel("staminaBarFill");
    staminaBarFill->SetPosition(vge::Vec2(20, 80));
    staminaBarFill->SetSize(vge::Vec2(200, 10));
    staminaBarFill->SetBackgroundColor(vge::Vec3(0.2f, 0.8f, 0.2f));
    ui->AddRootElement(std::unique_ptr<vge::UIElement>(staminaBarFill));
}

void HUDSystem::CreateExperienceBar() {
    if (!ui) return;

    // Background
    expBarBg = ui->CreatePanel("expBarBg");
    expBarBg->SetPosition(vge::Vec2(340, 680));
    expBarBg->SetSize(vge::Vec2(600, 10));
    expBarBg->SetBackgroundColor(vge::Vec3(0.2f, 0.2f, 0.2f));
    expBarBg->SetAlpha(0.8f);
    ui->AddRootElement(std::unique_ptr<vge::UIElement>(expBarBg));

    // Fill
    expBarFill = ui->CreatePanel("expBarFill");
    expBarFill->SetPosition(vge::Vec2(340, 680));
    expBarFill->SetSize(vge::Vec2(0, 10));
    expBarFill->SetBackgroundColor(vge::Vec3(0.2f, 0.8f, 0.2f));
    ui->AddRootElement(std::unique_ptr<vge::UIElement>(expBarFill));
}

void HUDSystem::CreateHotbar() {
    if (!ui) return;

    // Create 9 hotbar slots
    for (int i = 0; i < 9; i++) {
        auto* slot = ui->CreatePanel("hotbarSlot" + std::to_string(i));
        slot->SetPosition(vge::Vec2(440 + i * 45, 650));
        slot->SetSize(vge::Vec2(40, 40));
        slot->SetBackgroundColor(vge::Vec3(0.3f, 0.3f, 0.3f));
        slot->SetAlpha(0.8f);
        slot->SetBorderWidth(2.0f);
        slot->SetBorderColor(vge::Vec3(0.5f, 0.5f, 0.5f));
        ui->AddRootElement(std::unique_ptr<vge::UIElement>(slot));
        hotbarSlots.push_back(slot);

        auto* label = ui->CreateLabel("hotbarLabel" + std::to_string(i), "");
        label->SetPosition(vge::Vec2(450 + i * 45, 660));
        label->SetFontSize(12.0f);
        ui->AddRootElement(std::unique_ptr<vge::UIElement>(label));
        hotbarLabels.push_back(label);
    }

    // Selector
    hotbarSelector = ui->CreatePanel("hotbarSelector");
    hotbarSelector->SetPosition(vge::Vec2(440, 650));
    hotbarSelector->SetSize(vge::Vec2(40, 40));
    hotbarSelector->SetBackgroundColor(vge::Vec3(1.0f, 1.0f, 1.0f));
    hotbarSelector->SetAlpha(0.3f);
    hotbarSelector->SetBorderWidth(3.0f);
    hotbarSelector->SetBorderColor(vge::Vec3(1.0f, 1.0f, 1.0f));
    ui->AddRootElement(std::unique_ptr<vge::UIElement>(hotbarSelector));
}

void HUDSystem::SetHealth(float current, float max) {
    if (!healthBarFill || !healthText) return;

    float ratio = current / max;
    healthBarFill->SetSize(vge::Vec2(200 * ratio, 20));

    // Color based on health
    if (ratio > 0.5f) {
        healthBarFill->SetBackgroundColor(vge::Vec3(1.0f, 0.2f, 0.2f)); // Red
    } else if (ratio > 0.25f) {
        healthBarFill->SetBackgroundColor(vge::Vec3(1.0f, 0.5f, 0.0f)); // Orange
    } else {
        healthBarFill->SetBackgroundColor(vge::Vec3(1.0f, 0.0f, 0.0f)); // Dark red
    }

    healthText->SetText(std::to_string((int)current) + " / " + std::to_string((int)max));
}

void HUDSystem::SetHunger(float current, float max) {
    if (!hungerBarFill || !hungerText) return;

    float ratio = current / max;
    hungerBarFill->SetSize(vge::Vec2(200 * ratio, 20));

    // Color based on hunger
    if (ratio > 0.5f) {
        hungerBarFill->SetBackgroundColor(vge::Vec3(0.8f, 0.5f, 0.2f)); // Brown
    } else if (ratio > 0.25f) {
        hungerBarFill->SetBackgroundColor(vge::Vec3(0.9f, 0.3f, 0.1f)); // Orange
    } else {
        hungerBarFill->SetBackgroundColor(vge::Vec3(1.0f, 0.0f, 0.0f)); // Red
    }

    hungerText->SetText(std::to_string((int)current) + " / " + std::to_string((int)max));
}

void HUDSystem::SetStamina(float current, float max) {
    if (!staminaBarFill) return;

    float ratio = current / max;
    staminaBarFill->SetSize(vge::Vec2(200 * ratio, 10));
}

void HUDSystem::SetExperience(float current, float max) {
    if (!expBarFill) return;

    float ratio = current / max;
    expBarFill->SetSize(vge::Vec2(600 * ratio, 10));
}

void HUDSystem::SetHotbarSlot(int slot) {
    if (slot < 0 || slot >= 9 || !hotbarSelector) return;

    hotbarSelector->SetPosition(vge::Vec2(440 + slot * 45, 650));
}

void HUDSystem::SetHotbarItem(int slot, const std::string& itemId, int count) {
    if (slot < 0 || slot >= 9 || !hotbarLabels[slot]) return;

    if (itemId.empty()) {
        hotbarLabels[slot]->SetText("");
    } else {
        hotbarLabels[slot]->SetText(itemId + " x" + std::to_string(count));
    }
}

void HUDSystem::Show() {
    visible = true;
    if (healthBarBg) healthBarBg->SetVisible(true);
    if (healthBarFill) healthBarFill->SetVisible(true);
    if (healthText) healthText->SetVisible(true);
    if (hungerBarBg) hungerBarBg->SetVisible(true);
    if (hungerBarFill) hungerBarFill->SetVisible(true);
    if (hungerText) hungerText->SetVisible(true);
    if (staminaBarBg) staminaBarBg->SetVisible(true);
    if (staminaBarFill) staminaBarFill->SetVisible(true);
    if (expBarBg) expBarBg->SetVisible(true);
    if (expBarFill) expBarFill->SetVisible(true);
    if (hotbarSelector) hotbarSelector->SetVisible(true);
    for (auto* slot : hotbarSlots) {
        if (slot) slot->SetVisible(true);
    }
    for (auto* label : hotbarLabels) {
        if (label) label->SetVisible(true);
    }
}

void HUDSystem::Hide() {
    visible = false;
    if (healthBarBg) healthBarBg->SetVisible(false);
    if (healthBarFill) healthBarFill->SetVisible(false);
    if (healthText) healthText->SetVisible(false);
    if (hungerBarBg) hungerBarBg->SetVisible(false);
    if (hungerBarFill) hungerBarFill->SetVisible(false);
    if (hungerText) hungerText->SetVisible(false);
    if (staminaBarBg) staminaBarBg->SetVisible(false);
    if (staminaBarFill) staminaBarFill->SetVisible(false);
    if (expBarBg) expBarBg->SetVisible(false);
    if (expBarFill) expBarFill->SetVisible(false);
    if (hotbarSelector) hotbarSelector->SetVisible(false);
    for (auto* slot : hotbarSlots) {
        if (slot) slot->SetVisible(false);
    }
    for (auto* label : hotbarLabels) {
        if (label) label->SetVisible(false);
    }
}

bool HUDSystem::IsVisible() const {
    return visible;
}

} // namespace vge
