#include "core/engine.h"
#include "core/application.h"
#include "core/logger.h"
#include <iostream>

using namespace vge;

int main() {
    Logger::Info("Voxel Engine Starting...");
    
    Application app;
    if (!app.Initialize()) {
        Logger::Error("Failed to initialize application");
        return -1;
    }
    
    Logger::Info("Running main loop...");
    app.Run();
    
    Logger::Info("Shutting down...");
    app.Shutdown();
    
    return 0;
}
