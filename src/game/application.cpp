#include "application.h"
#include "core/logger.h"
#include <chrono>

namespace vge {

Application::Application() = default;

bool Application::initialize(int width, int height, const std::string& title) {
    Logger::Info("Initializing Aether Engine application facade...");

    window = std::make_unique<Window>();
    if (!window->Initialize(width, height, title)) {
        Logger::Error("Failed to create window");
        return false;
    }

    renderer = std::make_unique<ModernRenderer>();
    if (!renderer->Initialize(width, height)) {
        Logger::Error("Failed to initialize renderer");
        return false;
    }

    camera = std::make_unique<Camera>();
    camera->SetPosition(Vec3(0.0f, 41.6f, 0.0f));
    camera->SetRotation(0.0f, -20.0f, 0.0f);

    world = std::make_unique<World>();
    world->SetSeed(12345);
    world->Initialize();
    world->GenerateTerrain(0, 0, 1);

    audio = std::make_unique<AudioEngine>();
    audio->Initialize();

    input = std::make_unique<Input>();
    entities = std::make_unique<EntityManager>();
    ui = std::make_unique<UIManager>();
    resources = std::make_unique<ResourceManager>();
    running = true;

    Logger::Info("Aether Engine application facade initialized successfully");
    return true;
}

void Application::run() {
    using clock = std::chrono::high_resolution_clock;
    auto lastFrame = clock::now();

    while (running) {
        const auto now = clock::now();
        deltaTime = std::chrono::duration<float>(now - lastFrame).count();
        lastFrame = now;

        window->PollEvents();
        if (window->ShouldClose()) {
            running = false;
            break;
        }

        input->Update(window->GetHandle());
        world->Update(camera->GetPosition());
        entities->Update(deltaTime);
        audio->Update(deltaTime);

        renderer->BeginFrame();
        renderer->RenderWorld(*world, *camera);
        renderer->EndFrame();
        ui->Render();
        window->SwapBuffers();
    }
}

void Application::shutdown() {
    Logger::Info("Shutting down Aether Engine application facade...");

    if (audio) audio->Shutdown();
    if (renderer) renderer->Shutdown();
    if (window) window->Shutdown();

    running = false;
    Logger::Info("Aether Engine application facade shutdown complete");
}

bool Application::isRunning() const { return running; }
float Application::getDeltaTime() const { return deltaTime; }
Window& Application::getWindow() { return *window; }
Renderer& Application::getRenderer() { return *renderer; }
Camera& Application::getCamera() { return *camera; }
World& Application::getWorld() { return *world; }
AudioEngine& Application::getAudio() { return *audio; }
Input& Application::getInput() { return *input; }
EntityManager& Application::getEntities() { return *entities; }
UIManager& Application::getUI() { return *ui; }
ResourceManager& Application::getResources() { return *resources; }
} // namespace vge
