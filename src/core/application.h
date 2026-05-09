#pragma once
#include <string>

namespace vge {
    class Application {
    public:
        void Initialize(int argc, char** argv);
        void Run();
        void Shutdown();
        bool IsRunning() const;
        float GetDeltaTime() const;
    private:
        bool running = false;
        float deltaTime = 0.0f;
    };
}
