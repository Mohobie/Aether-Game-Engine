#include "main_menu.h"
#include "core/logger.h"

namespace vge {

MainMenu::MainMenu(UIManager* uiManager)
    : ui(uiManager), panel(nullptr), visible(false),
      titleLabel(nullptr), singleplayerBtn(nullptr),
      multiplayerBtn(nullptr), settingsBtn(nullptr), quitBtn(nullptr) {
}

MainMenu::~MainMenu() {
}

void MainMenu::Initialize() {
    CreateUI();
    Show();
    Logger::Info("[MainMenu] Initialized");
}

void MainMenu::CreateUI() {
    if (!ui) return;

    // Main panel (full screen)
    panel = ui->CreatePanel("mainMenuPanel");
    panel->SetPosition(vge::Vec2(0, 0));
    panel->SetSize(vge::Vec2(1280, 720));
    panel->SetBackgroundColor(vge::Vec3(0.1f, 0.1f, 0.15f));
    panel->SetAlpha(1.0f);

    // Title
    titleLabel = ui->CreateLabel("titleLabel", "AETHER VOXEL GAME");
    titleLabel->SetPosition(vge::Vec2(340, 100));
    titleLabel->SetFontSize(48.0f);
    titleLabel->SetForegroundColor(vge::Vec3(0.2f, 0.8f, 1.0f));
    panel->AddChild(std::unique_ptr<vge::UIElement>(titleLabel));

    // Subtitle
    auto* subtitle = ui->CreateLabel("subtitleLabel", "A Voxel Engine Demo");
    subtitle->SetPosition(vge::Vec2(460, 170));
    subtitle->SetFontSize(20.0f);
    subtitle->SetForegroundColor(vge::Vec3(0.6f, 0.6f, 0.6f));
    panel->AddChild(std::unique_ptr<vge::UIElement>(subtitle));

    // Singleplayer button
    singleplayerBtn = ui->CreateButton("singleplayerBtn", "Singleplayer");
    singleplayerBtn->SetPosition(vge::Vec2(490, 280));
    singleplayerBtn->SetSize(vge::Vec2(300, 50));
    singleplayerBtn->SetOnClick([this]() {
        if (onSingleplayer) onSingleplayer();
    });
    panel->AddChild(std::unique_ptr<vge::UIElement>(singleplayerBtn));

    // Multiplayer button
    multiplayerBtn = ui->CreateButton("multiplayerBtn", "Multiplayer");
    multiplayerBtn->SetPosition(vge::Vec2(490, 350));
    multiplayerBtn->SetSize(vge::Vec2(300, 50));
    multiplayerBtn->SetOnClick([this]() {
        if (onMultiplayer) onMultiplayer();
    });
    panel->AddChild(std::unique_ptr<vge::UIElement>(multiplayerBtn));

    // Settings button
    settingsBtn = ui->CreateButton("settingsBtn", "Settings");
    settingsBtn->SetPosition(vge::Vec2(490, 420));
    settingsBtn->SetSize(vge::Vec2(300, 50));
    settingsBtn->SetOnClick([this]() {
        if (onSettings) onSettings();
    });
    panel->AddChild(std::unique_ptr<vge::UIElement>(settingsBtn));

    // Quit button
    quitBtn = ui->CreateButton("quitBtn", "Quit");
    quitBtn->SetPosition(vge::Vec2(490, 490));
    quitBtn->SetSize(vge::Vec2(300, 50));
    quitBtn->SetOnClick([this]() {
        if (onQuit) onQuit();
    });
    panel->AddChild(std::unique_ptr<vge::UIElement>(quitBtn));

    // Version info
    auto* versionLabel = ui->CreateLabel("versionLabel", "v0.1.0 - Aether Engine");
    versionLabel->SetPosition(vge::Vec2(20, 680));
    versionLabel->SetFontSize(12.0f);
    versionLabel->SetForegroundColor(vge::Vec3(0.4f, 0.4f, 0.4f));
    panel->AddChild(std::unique_ptr<vge::UIElement>(versionLabel));

    ui->AddRootElement(std::unique_ptr<vge::UIElement>(panel));
}

void MainMenu::Show() {
    visible = true;
    if (panel) panel->SetVisible(true);
}

void MainMenu::Hide() {
    visible = false;
    if (panel) panel->SetVisible(false);
}

bool MainMenu::IsVisible() const {
    return visible;
}

void MainMenu::SetOnSingleplayer(std::function<void()> callback) {
    onSingleplayer = callback;
}

void MainMenu::SetOnMultiplayer(std::function<void()> callback) {
    onMultiplayer = callback;
}

void MainMenu::SetOnSettings(std::function<void()> callback) {
    onSettings = callback;
}

void MainMenu::SetOnQuit(std::function<void()> callback) {
    onQuit = callback;
}

} // namespace vge
