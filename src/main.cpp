#include <iostream>
#include "platform/window.h"
#include "platform/input.h"
#include "rendering/renderer.h"
#include "rendering/camera.h"
#include "rendering/shader.h"
#include "voxel/world.h"
#include "voxel/chunk.h"
#include "voxel/world_generator.h"
#include "voxel/chunk_manager.h"
#include "voxel/block_mesh_builder.h"
#include "core/logger.h"
#include <glad/gl.h>
#include <GLFW/glfw3.h>

using namespace vge;

int main() {
    Logger::GetInstance().SetLogLevel(LogLevel::Info);
    Logger::GetInstance().Log(LogLevel::Info, "Voxel Engine Starting...");
    
    // Create window
    Window window;
    if (!window.Initialize(1280, 720, "Voxel Engine")) {
        Logger::GetInstance().Log(LogLevel::Fatal, "Failed to create window");
        return -1;
    }
    
    // Initialize renderer
    Renderer renderer;
    if (!renderer.Initialize()) {
        Logger::GetInstance().Log(LogLevel::Fatal, "Failed to initialize renderer");
        return -1;
    }
    
    // Create camera
    Camera camera;
    camera.SetPosition(Vec3(0.0f, 40.0f, 0.0f));
    
    // Input system
    Input input;
    
    // World generation
    World world;
    WorldGenerator generator(12345);
    ChunkManager chunkManager;
    
    // Generate initial chunks
    for (int x = -2; x <= 2; ++x) {
        for (int z = -2; z <= 2; ++z) {
            Chunk* chunk = chunkManager.GetChunk(x, 0, z);
            generator.GenerateChunk(chunk);
        }
    }
    
    Logger::GetInstance().Log(LogLevel::Info, "World generated. Entering main loop...");
    
    // Main loop
    while (!window.ShouldClose()) {
        window.PollEvents();
        input.Update(window.window);
        
        // Camera movement
        float speed = 0.1f;
        if (input.IsKeyPressed(GLFW_KEY_W)) camera.Move(Vec3(0, 0, -speed));
        if (input.IsKeyPressed(GLFW_KEY_S)) camera.Move(Vec3(0, 0, speed));
        if (input.IsKeyPressed(GLFW_KEY_A)) camera.Move(Vec3(-speed, 0, 0));
        if (input.IsKeyPressed(GLFW_KEY_D)) camera.Move(Vec3(speed, 0, 0));
        
        // Render
        renderer.BeginFrame();
        renderer.SetClearColor(0.5f, 0.7f, 1.0f, 1.0f);
        
        // TODO: Render chunks here
        
        renderer.EndFrame();
        window.SwapBuffers();
    }
    
    Logger::GetInstance().Log(LogLevel::Info, "Shutting down...");
    renderer.Shutdown();
    window.Shutdown();
    
    return 0;
}
