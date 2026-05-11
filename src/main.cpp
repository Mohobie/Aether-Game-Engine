#include "core/logger.h"
#include "platform/window.h"
#include "platform/input_manager.h"
#include "rendering/renderer.h"
#include "rendering/camera.h"
#include "voxel/world.h"
#include "voxel/world_generator.h"
#include "voxel/chunk_manager.h"
#include "voxel/block_registry.h"
#include "audio/audio_engine.h"
#include "game/game_state.h"
#include "game/block_interaction.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>
#include <memory>

using namespace vge;

// ============================================
// Game State Management
// ============================================
enum class GameState {
    Loading,
    Playing,
    Paused,
    Quitting
};

struct GameContext {
    GameState state = GameState::Loading;
    float deltaTime = 0.016f;
    float totalTime = 0.0f;
    int frameCount = 0;
    bool shouldQuit = false;
    
    // Subsystems
    std::unique_ptr<Window> window;
    std::unique_ptr<Input> input;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<World> world;
    std::unique_ptr<WorldGenerator> worldGenerator;
    std::unique_ptr<ChunkManager> chunkManager;
    std::unique_ptr<AudioEngine> audioEngine;
    std::unique_ptr<BlockInteraction> blockInteraction;
};

// ============================================
// Initialization
// ============================================
static bool InitializeWindow(GameContext& ctx) {
    Logger::Info("[Main] Initializing window...");
    
    ctx.window = std::make_unique<Window>();
    if (!ctx.window->Initialize(1920, 1080, "Aether Game Engine")) {
        Logger::Error("[Main] Failed to create window");
        return false;
    }
    
    // Make window resizable
    GLFWwindow* glfwWin = static_cast<GLFWwindow*>(ctx.window->GetHandle());
    glfwSetWindowAttrib(glfwWin, GLFW_RESIZABLE, GLFW_TRUE);
    
    Logger::Info("[Main] Window created: 1920x1080 (resizable)");
    return true;
}

static bool InitializeRenderer(GameContext& ctx) {
    Logger::Info("[Main] Initializing renderer...");
    
    ctx.renderer = std::make_unique<Renderer>();
    if (!ctx.renderer->Initialize()) {
        Logger::Error("[Main] Failed to initialize renderer");
        return false;
    }
    
    ctx.renderer->SetViewport(0, 0, 1920, 1080);
    ctx.renderer->SetClearColor(0.53f, 0.81f, 0.92f, 1.0f); // Sky blue
    
    Logger::Info("[Main] Renderer initialized");
    return true;
}

static bool InitializeWorld(GameContext& ctx) {
    Logger::Info("[Main] Initializing world...");
    
    ctx.world = std::make_unique<World>();
    ctx.worldGenerator = std::make_unique<WorldGenerator>();
    ctx.chunkManager = std::make_unique<ChunkManager>();
    
    ctx.world->SetSeed(12345);
    ctx.worldGenerator->SetSeed(12345);
    ctx.chunkManager->Initialize(ctx.world.get(), ctx.worldGenerator.get());
    
    // Generate spawn chunks
    for (int x = -2; x <= 2; x++) {
        for (int z = -2; z <= 2; z++) {
            Chunk* chunk = ctx.chunkManager->LoadChunk(x, 0, z);
            if (chunk) {
                ctx.worldGenerator->GenerateChunk(*chunk, x, 0, z);
            }
        }
    }
    
    Logger::Info("[Main] World initialized with seed 12345");
    return true;
}

static bool InitializeInput(GameContext& ctx) {
    Logger::Info("[Main] Initializing input...");
    
    ctx.input = std::make_unique<Input>();
    
    Logger::Info("[Main] Input initialized");
    return true;
}

static bool InitializeAudio(GameContext& ctx) {
    Logger::Info("[Main] Initializing audio...");
    
    ctx.audioEngine = std::make_unique<AudioEngine>();
    if (!ctx.audioEngine->Initialize()) {
        Logger::Info("[Main] Audio initialization failed - continuing without audio");
        // Non-fatal: game can run without audio
    } else {
        Logger::Info("[Main] Audio initialized");
    }
    
    return true;
}

static bool InitializeCamera(GameContext& ctx) {
    Logger::Info("[Main] Initializing camera...");
    
    ctx.camera = std::make_unique<Camera>();
    ctx.camera->SetPosition(Vec3(0.0f, 41.6f, 0.0f));
    ctx.camera->SetRotation(0.0f, -20.0f, 0.0f);
    
    Logger::Info("[Main] Camera initialized");
    return true;
}

// ============================================
// Subsystem Initialization (in correct order)
// ============================================
static bool InitializeAll(GameContext& ctx) {
    Logger::Info("============================================");
    Logger::Info("  Aether Game Engine - Starting Up");
    Logger::Info("============================================");
    
    // Order matters: window first (creates GL context), then renderer, then world, input, audio
    if (!InitializeWindow(ctx))    return false;
    if (!InitializeRenderer(ctx))  return false;
    if (!InitializeWorld(ctx))     return false;
    if (!InitializeInput(ctx))     return false;
    if (!InitializeAudio(ctx))     return false;
    if (!InitializeCamera(ctx))    return false;

    // Initialize block interaction system
    Logger::Info("[Main] Initializing block interaction...");
    ctx.blockInteraction = std::make_unique<BlockInteraction>();
    ctx.blockInteraction->Initialize(*ctx.world);
    Logger::Info("[Main] Block interaction initialized");

    ctx.state = GameState::Playing;
    
    Logger::Info("============================================");
    Logger::Info("  All systems initialized successfully");
    Logger::Info("============================================");
    
    // Print controls
    std::cout << "\n=== CONTROLS ===" << std::endl;
    std::cout << "WASD       = Move" << std::endl;
    std::cout << "Arrow Keys = Look around" << std::endl;
    std::cout << "Space      = Jump / Up" << std::endl;
    std::cout << "Shift      = Down" << std::endl;
    std::cout << "E          = Place block (right click)" << std::endl;
    std::cout << "Q          = Break block (left click)" << std::endl;
    std::cout << "1-9        = Select block type" << std::endl;
    std::cout << "ESC        = Quit" << std::endl;
    std::cout << "================\n" << std::endl;
    
    return true;
}

// ============================================
// Cleanup
// ============================================
static void ShutdownAll(GameContext& ctx) {
    Logger::Info("[Main] Shutting down...");
    
    // Cleanup in reverse order of initialization
    ctx.audioEngine.reset();
    ctx.input.reset();
    ctx.chunkManager.reset();
    ctx.worldGenerator.reset();
    ctx.world.reset();
    ctx.camera.reset();
    ctx.renderer.reset();
    ctx.window.reset();
    
    Logger::Info("[Main] Shutdown complete");
}

// ============================================
// Update
// ============================================
static void Update(GameContext& ctx) {
    // Poll window events
    ctx.window->PollEvents();
    
    // Check for window close
    if (ctx.window->ShouldClose()) {
        ctx.shouldQuit = true;
        return;
    }
    
    // Update input
    ctx.input->Update(ctx.window->GetHandle());
    
    // Check quit key
    if (ctx.input->IsKeyPressed(KeyCode::Escape)) {
        ctx.shouldQuit = true;
        return;
    }
    
    // Camera movement (simple fly camera)
    const float moveSpeed = 10.0f * ctx.deltaTime;
    Vec3 camPos = ctx.camera->GetPosition();
    
    if (ctx.input->IsKeyPressed(KeyCode::W)) {
        camPos = camPos + ctx.camera->GetForward() * moveSpeed;
    }
    if (ctx.input->IsKeyPressed(KeyCode::S)) {
        camPos = camPos - ctx.camera->GetForward() * moveSpeed;
    }
    if (ctx.input->IsKeyPressed(KeyCode::A)) {
        camPos = camPos - ctx.camera->GetRight() * moveSpeed;
    }
    if (ctx.input->IsKeyPressed(KeyCode::D)) {
        camPos = camPos + ctx.camera->GetRight() * moveSpeed;
    }
    if (ctx.input->IsKeyPressed(KeyCode::Space)) {
        camPos.y += moveSpeed;
    }
    if (ctx.input->IsKeyPressed(KeyCode::Shift)) {
        camPos.y -= moveSpeed;
    }
    
    ctx.camera->SetPositionDirect(camPos);

    // Update block interaction (raycast + input)
    ctx.blockInteraction->Update(*ctx.camera, *ctx.input, *ctx.world);

    // Update total time
    ctx.totalTime += ctx.deltaTime;
    ctx.frameCount++;
}

// ============================================
// Render
// ============================================
static void Render(GameContext& ctx) {
    ctx.renderer->BeginFrame();
    ctx.renderer->RenderWorld(*ctx.world, *ctx.camera);

    // Render crosshair and block highlight
    int w = ctx.renderer->GetWidth();
    int h = ctx.renderer->GetHeight();
    ctx.renderer->RenderCrosshair(w, h);

    if (ctx.blockInteraction->HasTarget()) {
        ctx.renderer->RenderBlockHighlight(
            ctx.blockInteraction->GetTarget().blockPosition,
            *ctx.camera, w, h
        );
    }

    ctx.renderer->EndFrame();

    // Swap buffers
    ctx.window->SwapBuffers();
}

// ============================================
// Main Loop
// ============================================
static void RunMainLoop(GameContext& ctx) {
    Logger::Info("[Main] Entering main loop...");
    
    auto lastTime = std::chrono::high_resolution_clock::now();
    
    while (!ctx.shouldQuit) {
        // Calculate delta time
        auto currentTime = std::chrono::high_resolution_clock::now();
        ctx.deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        // Clamp delta time to prevent large jumps
        if (ctx.deltaTime > 0.1f) {
            ctx.deltaTime = 0.1f;
        }
        
        // Update and render
        Update(ctx);
        
        if (!ctx.shouldQuit) {
            Render(ctx);
        }
    }
    
    Logger::Info("[Main] Exited main loop");
    Logger::Info("[Main] Total frames: " + std::to_string(ctx.frameCount));
    Logger::Info("[Main] Total time: " + std::to_string(ctx.totalTime) + "s");
}

// ============================================
// Entry Point
// ============================================
int main() {
    GameContext ctx;
    
    // Initialize all subsystems
    if (!InitializeAll(ctx)) {
        Logger::Error("[Main] Initialization failed - exiting");
        ShutdownAll(ctx);
        return 1;
    }
    
    // Run the game
    RunMainLoop(ctx);
    
    // Cleanup
    ShutdownAll(ctx);
    
    return 0;
}
