#include "ui/menu_system.h"
#include "ui/imgui_wrapper.h"
#include "rendering/renderer.h"
#include "platform/window.h"
#include <iostream>

// Stub implementation when Dear ImGui is not available
namespace vge {

MenuSystem::MenuSystem() : currentScreen(MenuScreen::MainMenu), initialized(false) {}

MenuSystem::~MenuSystem() {
    if (initialized) Shutdown();
}

bool MenuSystem::Initialize(Renderer* renderer, Window* window) {
    this->renderer = renderer;
    this->window = window;
    
    if (!imgui.Initialize(static_cast<GLFWwindow*>(window->GetHandle()))) {
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
    std::cout << "[Menu] Main Menu" << std::endl;
    std::cout << "  1. Single Player" << std::endl;
    std::cout << "  2. Multiplayer" << std::endl;
    std::cout << "  3. Settings" << std::endl;
    std::cout << "  4. Quit" << std::endl;
    std::cout << "\nPress 1 to start game..." << std::endl;
}

void MenuSystem::RenderSettings() {
    std::cout << "[Menu] Settings" << std::endl;
}

void MenuSystem::RenderPauseMenu() {
    std::cout << "[Menu] Paused" << std::endl;
    std::cout << "  1. Resume" << std::endl;
    std::cout << "  2. Settings" << std::endl;
    std::cout << "  3. Save & Quit" << std::endl;
}

void MenuSystem::RenderInventory() {
    std::cout << "[Menu] Inventory" << std::endl;
}

void MenuSystem::RenderCrafting() {
    std::cout << "[Menu] Crafting" << std::endl;
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