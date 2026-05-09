#pragma once
#include <string>

namespace vge {

class Window;
class Input;
class Renderer;
class Camera;
class World;
class WorldGenerator;
class ChunkManager;
class WorldRenderer;
class AudioEngine;
class SoundManager;
class MenuSystem;
class TimeSystem;
class AchievementManager;

class Engine {
private:
    Window* window;
    Input* input;
    Renderer* renderer;
    Camera* camera;
    World* world;
    WorldGenerator* worldGenerator;
    ChunkManager* chunkManager;
    WorldRenderer* worldRenderer;
    AudioEngine* audioEngine;
    SoundManager* soundManager;
    MenuSystem* menuSystem;
    TimeSystem* timeSystem;
    AchievementManager* achievementManager;
    
    bool running;
    float deltaTime;
    
    void Update(float dt);
    void Render();
    
public:
    Engine();
    ~Engine();
    
    bool Initialize();
    void Run();
    void Shutdown();
    void Stop() { running = false; }
    void SetRunning(bool value) { running = value; }
    
    bool IsRunning() const { return running; }
    float GetDeltaTime() const { return deltaTime; }
};

} // namespace vge