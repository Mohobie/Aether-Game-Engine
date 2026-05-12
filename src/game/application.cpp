#include "application.h"
#include "core/utils.h"
#include "platform/timer.h"
#include <iostream>
namespace vge {
Application::Application() {}
bool Application::initialize(int width, int height, const std::string& title) {
    Logger::info("Initializing Aether Engine...");
    window = std::make_unique<Window>();
    if (!window->create(title, width, height)) {
        Logger::error("Failed to create window");
        return false;
    }
    renderer = std::make_unique<Renderer>();
    if (!renderer->initialize()) {
        Logger::error("Failed to initialize renderer");
        return false;
    }
    world = std::make_unique<World>(12345);
    world->initialize();
    audio = std::make_unique<AudioEngine>();
    audio->initialize();
    input = std::make_unique<InputManager>();
    entities = std::make_unique<EntityManager>();
    ui = std::make_unique<UIManager>();
    resources = std::make_unique<ResourceManager>();
    running = true;
    Logger::info("Aether Engine initialized successfully");
    return true;
}
void Application::run() {
    Timer timer;
    while (running) {
        timer.start();
        window->pollEvents();
        if (window->shouldClose()) {
            running = false;
            break;
        }
        input->update();
        renderer->beginFrame();
        // Render world, entities, UI here
        renderer->endFrame();
        window->swapBuffers();
        timer.stop();
        deltaTime = static_cast<float>(timer.elapsedSeconds());
    }
}
void Application::shutdown() {
    Logger::info("Shutting down Aether Engine...");
    if (audio) audio->shutdown();
    if (renderer) renderer->shutdown();
    if (window) window->destroy();
    running = false;
    Logger::info("Aether Engine shutdown complete");
}
bool Application::isRunning() const { return running; }
float Application::getDeltaTime() const { return deltaTime; }
Window& Application::getWindow() { return *window; }
Renderer& Application::getRenderer() { return *renderer; }
World& Application::getWorld() { return *world; }
AudioEngine& Application::getAudio() { return *audio; }
InputManager& Application::getInput() { return *input; }
EntityManager& Application::getEntities() { return *entities; }
UIManager& Application::getUI() { return *ui; }
ResourceManager& Application::getResources() { return *resources; }
} // namespace vge
