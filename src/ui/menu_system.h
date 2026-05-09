#pragma once
#include "imgui_wrapper.h"
#include "rendering/renderer.h"
#include "platform/window.h"
#include <functional>

namespace vge {

enum class MenuScreen {
    None,
    MainMenu,
    Settings,
    Pause,
    Inventory,
    Crafting
};

class MenuSystem {
private:
    ImGuiWrapper imgui;
    Renderer* renderer;
    Window* window;
    MenuScreen currentScreen;
    MenuScreen previousScreen;
    bool initialized;
    
    void RenderMainMenu();
    void RenderSettings();
    void RenderPauseMenu();
    void RenderInventory();
    void RenderCrafting();
    
public:
    // Callbacks
    std::function<void()> onStartSinglePlayer;
    std::function<void()> onStartMultiplayer;
    std::function<void()> onQuit;
    std::function<void()> onSaveAndQuit;
    
    MenuSystem();
    ~MenuSystem();
    
    bool Initialize(Renderer* renderer, Window* window);
    void Shutdown();
    
    void Update(float deltaTime);
    
    void ShowScreen(MenuScreen screen);
    void ShowPauseMenu();
    void HideMenu();
    bool IsMenuOpen() const;
    MenuScreen GetCurrentScreen() const { return currentScreen; }
};

} // namespace vge