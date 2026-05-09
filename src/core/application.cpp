#include "core/application.h"
#include "core/engine.h"
#include "core/logger.h"
#include <iostream>

// Stub implementation
namespace vge {

Application::Application() : engine(nullptr), initialized(false), running(false) {}

Application::~Application() {
    Shutdown();
}

bool Application::Initialize() {
    Logger::Info("Application initializing...");
    
    engine = new Engine();
    if (!engine->Initialize()) {
        Logger::Error("Failed to initialize engine");
        return false;
    }
    
    initialized = true;
    running = true;
    Logger::Info("Application initialized");
    return true;
}

void Application::Shutdown() {
    if (engine) {
        engine->Shutdown();
        delete engine;
        engine = nullptr;
    }
    
    initialized = false;
    running = false;
    Logger::Info("Application shutdown");
}

void Application::Run() {
    if (!initialized || !engine) {
        Logger::Error("Cannot run - not initialized");
        return;
    }
    
    engine->Run();
    running = false;
}

} // namespace vge
