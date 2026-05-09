#pragma once
#include <string>

namespace vge {

class Engine;

class Application {
private:
    Engine* engine;
    bool initialized;
    bool running;
    
public:
    Application();
    ~Application();
    
    bool Initialize();
    void Run();
    void Shutdown();
    
    bool IsRunning() const { return running; }
};

} // namespace vge
