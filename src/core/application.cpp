#include "application.h"
#include <chrono>
#include <iostream>

namespace VoxelEngine {
    void Application::Initialize(int argc, char** argv) {
        std::cout << "Application initializing..." << std::endl;
        running = true;
    }
    
    void Application::Run() {
        auto lastTime = std::chrono::high_resolution_clock::now();
        
        while (running) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;
        }
    }
    
    void Application::Shutdown() {
        std::cout << "Application shutting down..." << std::endl;
        running = false;
    }
    
    bool Application::IsRunning() const {
        return running;
    }
    
    float Application::GetDeltaTime() const {
        return deltaTime;
    }
}
