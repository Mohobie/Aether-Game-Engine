#pragma once
#include "platform/window.h"
#include "render/renderer.h"
#include "voxel/world.h"
#include "audio/audio_engine.h"
#include "input/input_manager.h"
#include "entity/entity_manager.h"
#include "ui/ui_manager.h"
#include "resource/resource_manager.h"
#include <memory>
namespace aether {
class Application {
public:
    Application();
    bool initialize(int width, int height, const std::string& title);
    void run();
    void shutdown();
    bool isRunning() const;
    float getDeltaTime() const;
    Window& getWindow();
    Renderer& getRenderer();
    World& getWorld();
    AudioEngine& getAudio();
    InputManager& getInput();
    EntityManager& getEntities();
    UIManager& getUI();
    ResourceManager& getResources();
private:
    bool running = false;
    float deltaTime = 0.0f;
    std::unique_ptr<Window> window;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<World> world;
    std::unique_ptr<AudioEngine> audio;
    std::unique_ptr<InputManager> input;
    std::unique_ptr<EntityManager> entities;
    std::unique_ptr<UIManager> ui;
    std::unique_ptr<ResourceManager> resources;
};
} // namespace aether
