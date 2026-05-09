#include "core/engine.h"
#include "core/logger.h"
#include "platform/window.h"
#include "platform/input.h"
#include "rendering/renderer.h"
#include "rendering/camera.h"
#include "voxel/world.h"
#include "voxel/world_generator.h"
#include "voxel/chunk_manager.h"
#include "voxel/world_renderer.h"
#include "audio/audio_engine.h"
#include "audio/sound_manager.h"
#include "ui/menu_system.h"
#include "core/time_system.h"
#include "core/achievements.h"
#include "voxel/block_registry.h"
#include "voxel/block_mesh_builder.h"
#include "rendering/lighting.h"
#include "network/network_manager.h"
#include "core/save_system.h"
#include "core/config.h"
#include "ui/console.h"
#include "ui/imgui_wrapper.h"
#include <iostream>
#include <chrono>

namespace vge {

Engine::Engine() 
    : window(nullptr), renderer(nullptr), camera(nullptr),
      world(nullptr), worldGenerator(nullptr), chunkManager(nullptr),
      worldRenderer(nullptr), audioEngine(nullptr), soundManager(nullptr),
      menuSystem(nullptr), timeSystem(nullptr), achievementManager(nullptr),
      running(false), deltaTime(0.016f) {}

Engine::~Engine() {
    Shutdown();
}

bool Engine::Initialize() {
    Logger::Info("Engine initializing...");
    
    window = new Window();
    renderer = new Renderer();
    camera = new Camera();
    world = new World();
    worldGenerator = new WorldGenerator();
    chunkManager = new ChunkManager();
    worldRenderer = new WorldRenderer();
    audioEngine = new AudioEngine();
    soundManager = new SoundManager();
    menuSystem = new MenuSystem();
    timeSystem = new TimeSystem();
    achievementManager = new AchievementManager();
    
    if (!window->Initialize(1280, 720, "Voxel Engine")) {
        Logger::Error("Failed to create window");
        return false;
    }
    
    if (!renderer->Initialize()) {
        Logger::Error("Failed to initialize renderer");
        return false;
    }
    
    if (audioEngine->Initialize()) {
        soundManager->Initialize(audioEngine);
    }
    
    worldGenerator->SetSeed(12345);
    chunkManager->Initialize(world, worldGenerator);
    
    // Generate spawn chunks
    for (int x = -2; x <= 2; x++) {
        for (int z = -2; z <= 2; z++) {
            Chunk* chunk = chunkManager->LoadChunk(x, 0, z);
            if (chunk) {
                worldGenerator->GenerateChunk(*chunk, x, 0, z);
            }
        }
    }
    
    menuSystem->Initialize(renderer, window);
    achievementManager->Initialize();
    timeSystem = new TimeSystem();
    
    camera->SetPosition(Vec3(0, 40, 0));
    camera->SetRotation(0, -20, 0);
    
    Logger::Info("Engine initialized successfully");
    running = true;
    return true;
}

void Engine::Shutdown() {
    Logger::Info("Engine shutting down...");
    
    delete achievementManager;
    delete timeSystem;
    delete menuSystem;
    delete soundManager;
    delete audioEngine;
    delete worldRenderer;
    delete chunkManager;
    delete worldGenerator;
    delete world;
    delete camera;
    delete renderer;
    delete window;
    
    Logger::Info("Engine shutdown complete");
    running = false;
}

void Engine::Run() {
    auto lastTime = std::chrono::high_resolution_clock::now();
    int frameCount = 0;
    
    while (running) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        window->PollEvents();
        
        if (window->ShouldClose()) {
            running = false;
            break;
        }
        
        Update(deltaTime);
        Render();
        
        frameCount++;
        if (frameCount >= 10) {
            std::cout << "\n[Engine] Rendered 10 frames. Press Enter to continue..." << std::endl;
            std::cin.get();
            running = false;
        }
    }
}

void Engine::Update(float dt) {
    timeSystem->Update(dt);
    menuSystem->Update(dt);
}

void Engine::Render() {
    renderer->BeginFrame();
    renderer->RenderWorld(*world, *camera);
    renderer->EndFrame();
}

} // namespace vge
