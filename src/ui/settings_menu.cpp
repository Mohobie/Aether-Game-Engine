#include "settings_menu.h"
#include "core/logger.h"

namespace vge {

SettingsMenu::SettingsMenu(UIManager* uiManager)
    : ui(uiManager), panel(nullptr), visible(false),
      volumeSlider(nullptr), sensitivitySlider(nullptr),
      renderDistanceSlider(nullptr), fullscreenCheckbox(nullptr),
      vsyncCheckbox(nullptr), backButton(nullptr) {
}

SettingsMenu::~SettingsMenu() {
}

void SettingsMenu::Initialize() {
    CreateUI();
    Hide();
    Logger::Info("[SettingsMenu] Initialized");
}

void SettingsMenu::CreateUI() {
    if (!ui) return;

    // Main panel
    panel = ui->CreatePanel("settingsPanel");
    panel->SetSize(vge::Vec2(400, 500));
    panel->SetPosition(vge::Vec2(440, 140)); // Center on 1280x720
    panel->SetBackgroundColor(vge::Vec3(0.15f, 0.15f, 0.15f));
    panel->SetAlpha(0.95f);
    panel->SetBorderRadius(10.0f);

    // Title
    auto* title = ui->CreateLabel("settingsTitle", "Settings");
    title->SetPosition(vge::Vec2(150, 20));
    title->SetFontSize(28.0f);
    panel->AddChild(std::unique_ptr<vge::UIElement>(title));

    // Volume
    auto* volumeLabel = ui->CreateLabel("volumeLabel", "Volume");
    volumeLabel->SetPosition(vge::Vec2(30, 80));
    volumeLabel->SetFontSize(18.0f);
    panel->AddChild(std::unique_ptr<vge::UIElement>(volumeLabel));

    volumeSlider = ui->CreateSlider("volumeSlider", 0.0f, 1.0f);
    volumeSlider->SetPosition(vge::Vec2(30, 110));
    volumeSlider->SetSize(vge::Vec2(340, 20));
    volumeSlider->SetValue(0.5f);
    volumeSlider->SetOnValueChanged([this](float value) {
        if (onVolumeChanged) onVolumeChanged(value);
    });
    panel->AddChild(std::unique_ptr<vge::UIElement>(volumeSlider));

    // Mouse Sensitivity
    auto* sensLabel = ui->CreateLabel("sensLabel", "Mouse Sensitivity");
    sensLabel->SetPosition(vge::Vec2(30, 160));
    sensLabel->SetFontSize(18.0f);
    panel->AddChild(std::unique_ptr<vge::UIElement>(sensLabel));

    sensitivitySlider = ui->CreateSlider("sensitivitySlider", 0.1f, 2.0f);
    sensitivitySlider->SetPosition(vge::Vec2(30, 190));
    sensitivitySlider->SetSize(vge::Vec2(340, 20));
    sensitivitySlider->SetValue(1.0f);
    sensitivitySlider->SetOnValueChanged([this](float value) {
        if (onSensitivityChanged) onSensitivityChanged(value);
    });
    panel->AddChild(std::unique_ptr<vge::UIElement>(sensitivitySlider));

    // Render Distance
    auto* renderLabel = ui->CreateLabel("renderLabel", "Render Distance");
    renderLabel->SetPosition(vge::Vec2(30, 240));
    renderLabel->SetFontSize(18.0f);
    panel->AddChild(std::unique_ptr<vge::UIElement>(renderLabel));

    renderDistanceSlider = ui->CreateSlider("renderDistanceSlider", 2.0f, 16.0f);
    renderDistanceSlider->SetPosition(vge::Vec2(30, 270));
    renderDistanceSlider->SetSize(vge::Vec2(340, 20));
    renderDistanceSlider->SetValue(8.0f);
    renderDistanceSlider->SetOnValueChanged([this](float value) {
        if (onRenderDistanceChanged) onRenderDistanceChanged(static_cast<int>(value));
    });
    panel->AddChild(std::unique_ptr<vge::UIElement>(renderDistanceSlider));

    // Fullscreen
    fullscreenCheckbox = ui->CreateCheckbox("fullscreenCheckbox", "Fullscreen");
    fullscreenCheckbox->SetPosition(vge::Vec2(30, 330));
    fullscreenCheckbox->SetOnCheckedChanged([this](bool checked) {
        if (onFullscreenChanged) onFullscreenChanged(checked);
    });
    panel->AddChild(std::unique_ptr<vge::UIElement>(fullscreenCheckbox));

    // VSync
    vsyncCheckbox = ui->CreateCheckbox("vsyncCheckbox", "VSync");
    vsyncCheckbox->SetPosition(vge::Vec2(30, 380));
    vsyncCheckbox->SetOnCheckedChanged([this](bool checked) {
        if (onVSyncChanged) onVSyncChanged(checked);
    });
    panel->AddChild(std::unique_ptr<vge::UIElement>(vsyncCheckbox));

    // Back button
    backButton = ui->CreateButton("backButton", "Back");
    backButton->SetPosition(vge::Vec2(150, 440));
    backButton->SetSize(vge::Vec2(100, 40));
    backButton->SetOnClick([this]() {
        Hide();
        if (onBack) onBack();
    });
    panel->AddChild(std::unique_ptr<vge::UIElement>(backButton));

    ui->AddRootElement(std::unique_ptr<vge::UIElement>(panel));
}

void SettingsMenu::Show() {
    visible = true;
    if (panel) panel->SetVisible(true);
}

void SettingsMenu::Hide() {
    visible = false;
    if (panel) panel->SetVisible(false);
}

bool SettingsMenu::IsVisible() const {
    return visible;
}

void SettingsMenu::Toggle() {
    if (visible) Hide();
    else Show();
}

void SettingsMenu::SetOnVolumeChanged(std::function<void(float)> callback) {
    onVolumeChanged = callback;
}

void SettingsMenu::SetOnSensitivityChanged(std::function<void(float)> callback) {
    onSensitivityChanged = callback;
}

void SettingsMenu::SetOnRenderDistanceChanged(std::function<void(int)> callback) {
    onRenderDistanceChanged = callback;
}

void SettingsMenu::SetOnFullscreenChanged(std::function<void(bool)> callback) {
    onFullscreenChanged = callback;
}

void SettingsMenu::SetOnVSyncChanged(std::function<void(bool)> callback) {
    onVSyncChanged = callback;
}

void SettingsMenu::SetOnBack(std::function<void()> callback) {
    onBack = callback;
}

void SettingsMenu::UpdateSettings(float volume, float sensitivity, int renderDist, bool fullscreen, bool vsync) {
    if (volumeSlider) volumeSlider->SetValue(volume);
    if (sensitivitySlider) sensitivitySlider->SetValue(sensitivity);
    if (renderDistanceSlider) renderDistanceSlider->SetValue(static_cast<float>(renderDist));
    if (fullscreenCheckbox) fullscreenCheckbox->SetChecked(fullscreen);
    if (vsyncCheckbox) vsyncCheckbox->SetChecked(vsync);
}

} // namespace vge
