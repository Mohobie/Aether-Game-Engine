#include "menu_system.h"
#include "imgui_wrapper.h"
#include "rendering/renderer.h"
#include "platform/window.h"
#include <iostream>

namespace vge {

MenuSystem::MenuSystem() : currentScreen(MenuScreen::MainMenu), initialized(false) {}

MenuSystem::~MenuSystem() {
    if (initialized) Shutdown();
}

bool MenuSystem::Initialize(Renderer* renderer, Window* window) {
    this->renderer = renderer;
    this->window = window;
    
    if (!imgui.Initialize(window->GetHandle())) {
        std::cerr << "[MenuSystem] Failed to initialize ImGui" << std::endl;
        return false;
    }
    
    initialized = true;
    std::cout << "[MenuSystem] Initialized" << std::endl;
    return true;
}

void MenuSystem::Shutdown() {
    imgui.Shutdown();
    initialized = false;
    std::cout << "[MenuSystem] Shutdown" << std::endl;
}

void MenuSystem::Update(float deltaTime) {
    if (!initialized) return;
    
    imgui.BeginFrame();
    
    switch (currentScreen) {
        case MenuScreen::MainMenu:
            RenderMainMenu();
            break;
        case MenuScreen::Settings:
            RenderSettings();
            break;
        case MenuScreen::Pause:
            RenderPauseMenu();
            break;
        case MenuScreen::Inventory:
            RenderInventory();
            break;
        case MenuScreen::Crafting:
            RenderCrafting();
            break;
        case MenuScreen::None:
            // No menu - game is playing
            break;
    }
    
    imgui.EndFrame();
}

void MenuSystem::RenderMainMenu() {
    // Center the menu
    ImVec2 windowSize = ImGui::GetIO().DisplaySize;
    float menuWidth = 300;
    float menuHeight = 400;
    
    ImGui::SetNextWindowPos(ImVec2((windowSize.x - menuWidth) * 0.5f, (windowSize.y - menuHeight) * 0.5f));
    ImGui::SetNextWindowSize(ImVec2(menuWidth, menuHeight));
    
    ImGui::Begin("Aether Voxel Engine", nullptr, 
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | 
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    
    ImGui::Text("Aether Voxel Engine");
    ImGui::Separator();
    
    if (ImGui::Button("Single Player", ImVec2(-1, 40))) {
        currentScreen = MenuScreen::None;
        if (onStartSinglePlayer) onStartSinglePlayer();
    }
    
    if (ImGui::Button("Multiplayer", ImVec2(-1, 40))) {
        // TODO: Show server connection dialog
    }
    
    if (ImGui::Button("Settings", ImVec2(-1, 40))) {
        currentScreen = MenuScreen::Settings;
    }
    
    if (ImGui::Button("Quit", ImVec2(-1, 40))) {
        if (onQuit) onQuit();
    }
    
    ImGui::End();
}

void MenuSystem::RenderSettings() {
    ImVec2 windowSize = ImGui::GetIO().DisplaySize;
    float menuWidth = 400;
    float menuHeight = 500;
    
    ImGui::SetNextWindowPos(ImVec2((windowSize.x - menuWidth) * 0.5f, (windowSize.y - menuHeight) * 0.5f));
    ImGui::SetNextWindowSize(ImVec2(menuWidth, menuHeight));
    
    ImGui::Begin("Settings", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoCollapse);
    
    // Graphics settings
    ImGui::Text("Graphics");
    ImGui::Separator();
    
    static int renderDistance = 16;
    ImGui::SliderInt("Render Distance", &renderDistance, 2, 32);
    
    static bool vsync = true;
    ImGui::Checkbox("VSync", &vsync);
    
    static bool fullscreen = false;
    ImGui::Checkbox("Fullscreen", &fullscreen);
    
    // Audio settings
    ImGui::Text("Audio");
    ImGui::Separator();
    
    static float masterVolume = 1.0f;
    ImGui::SliderFloat("Master Volume", &masterVolume, 0.0f, 1.0f);
    
    static float musicVolume = 0.7f;
    ImGui::SliderFloat("Music Volume", &musicVolume, 0.0f, 1.0f);
    
    static float sfxVolume = 1.0f;
    ImGui::SliderFloat("SFX Volume", &sfxVolume, 0.0f, 1.0f);
    
    ImGui::Separator();
    
    if (ImGui::Button("Back", ImVec2(-1, 40))) {
        currentScreen = MenuScreen::MainMenu;
    }
    
    ImGui::End();
}

void MenuSystem::RenderPauseMenu() {
    ImVec2 windowSize = ImGui::GetIO().DisplaySize;
    float menuWidth = 300;
    float menuHeight = 300;
    
    ImGui::SetNextWindowPos(ImVec2((windowSize.x - menuWidth) * 0.5f, (windowSize.y - menuHeight) * 0.5f));
    ImGui::SetNextWindowSize(ImVec2(menuWidth, menuHeight));
    
    ImGui::Begin("Paused", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    
    ImGui::Text("Paused");
    ImGui::Separator();
    
    if (ImGui::Button("Resume", ImVec2(-1, 40))) {
        currentScreen = MenuScreen::None;
    }
    
    if (ImGui::Button("Settings", ImVec2(-1, 40))) {
        currentScreen = MenuScreen::Settings;
    }
    
    if (ImGui::Button("Save & Quit", ImVec2(-1, 40))) {
        if (onSaveAndQuit) onSaveAndQuit();
    }
    
    ImGui::End();
}

void MenuSystem::RenderInventory() {
    // Would render the inventory grid with items
    // For now, just a placeholder
    ImGui::Begin("Inventory");
    ImGui::Text("Inventory - Not yet implemented");
    if (ImGui::Button("Close")) {
        currentScreen = MenuScreen::None;
    }
    ImGui::End();
}

void MenuSystem::RenderCrafting() {
    // Would render the crafting grid
    // For now, just a placeholder
    ImGui::Begin("Crafting");
    ImGui::Text("Crafting - Not yet implemented");
    if (ImGui::Button("Close")) {
        currentScreen = MenuScreen::None;
    }
    ImGui::End();
}

void MenuSystem::ShowScreen(MenuScreen screen) {
    currentScreen = screen;
}

void MenuSystem::ShowPauseMenu() {
    previousScreen = currentScreen;
    currentScreen = MenuScreen::Pause;
}

void MenuSystem::HideMenu() {
    currentScreen = MenuScreen::None;
}

bool MenuSystem::IsMenuOpen() const {
    return currentScreen != MenuScreen::None;
}

} // namespace vge