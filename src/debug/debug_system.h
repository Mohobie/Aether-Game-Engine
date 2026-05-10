#pragma once
#include "math/vec3.h"
#include <string>
#include <vector>
#include <map>
#include <chrono>

namespace vge {

// ============================================
// Performance Profiler
// ============================================
struct ProfileSample {
    std::string name;
    double elapsedMs;
    int callCount;
    double totalMs;
    double avgMs;
    double minMs;
    double maxMs;
};

class Profiler {
private:
    std::map<std::string, ProfileSample> samples;
    std::map<std::string, std::chrono::high_resolution_clock::time_point> activeTimers;
    
public:
    void BeginSample(const std::string& name);
    void EndSample(const std::string& name);
    
    const std::map<std::string, ProfileSample>& GetSamples() const { return samples; }
    void Reset();
    
    // Get formatted report
    std::vector<std::string> GetReport() const;
};

// ============================================
// Debug Metrics
// ============================================
struct DebugMetrics {
    // Frame stats
    float fps;
    float frameTimeMs;
    float deltaTime;
    int frameCount;
    
    // Memory
    size_t memoryUsedMB;
    size_t memoryTotalMB;
    size_t chunkMemoryMB;
    
    // World
    int loadedChunks;
    int visibleChunks;
    int totalBlocks;
    Vec3 playerPosition;
    Vec3 playerChunk;
    
    // Rendering
    int drawCalls;
    int trianglesRendered;
    int verticesRendered;
    int texturesBound;
    
    // Physics
    int rigidbodies;
    int collisions;
    int constraints;
    
    // Network
    int bytesSent;
    int bytesReceived;
    float latencyMs;
    
    DebugMetrics()
        : fps(0), frameTimeMs(0), deltaTime(0), frameCount(0)
        , memoryUsedMB(0), memoryTotalMB(0), chunkMemoryMB(0)
        , loadedChunks(0), visibleChunks(0), totalBlocks(0)
        , drawCalls(0), trianglesRendered(0), verticesRendered(0), texturesBound(0)
        , rigidbodies(0), collisions(0), constraints(0)
        , bytesSent(0), bytesReceived(0), latencyMs(0) {}
};

// ============================================
// Debug Visualizer
// ============================================
enum class DebugDrawMode {
    None = 0,
    Wireframe = 1,
    Normals = 2,
    BoundingBoxes = 4,
    ChunkBorders = 8,
    LightVolumes = 16,
    CollisionShapes = 32,
    NavMesh = 64,
    All = -1
};

class DebugVisualizer {
private:
    uint32_t drawMode;
    bool showGrid;
    bool showAxes;
    bool showStats;
    bool showConsole;
    
public:
    DebugVisualizer();
    
    // Mode toggles
    void SetDrawMode(uint32_t mode) { drawMode = mode; }
    void ToggleMode(DebugDrawMode mode);
    bool IsModeEnabled(DebugDrawMode mode) const;
    
    // Visual toggles
    void ToggleGrid() { showGrid = !showGrid; }
    void ToggleAxes() { showAxes = !showAxes; }
    void ToggleStats() { showStats = !showStats; }
    void ToggleConsole() { showConsole = !showConsole; }
    
    bool ShowGrid() const { return showGrid; }
    bool ShowAxes() const { return showAxes; }
    bool ShowStats() const { return showStats; }
    bool ShowConsole() const { return showConsole; }
    
    // Drawing functions (would use OpenGL/Vulkan)
    void DrawLine(const Vec3& start, const Vec3& end, const Vec3& color);
    void DrawBox(const Vec3& min, const Vec3& max, const Vec3& color);
    void DrawSphere(const Vec3& center, float radius, const Vec3& color);
    void DrawCross(const Vec3& position, float size, const Vec3& color);
    void DrawGrid(float size, float step, const Vec3& color);
    void DrawAxes(const Vec3& position, float size);
};

// ============================================
// Debug Menu / Inspector
// ============================================
class DebugMenu {
private:
    bool visible;
    int selectedTab;
    
    // Tab states
    bool showProfiler;
    bool showMetrics;
    bool showSettings;
    bool showEntities;
    bool showChunks;
    bool showNetwork;
    
public:
    DebugMenu();
    
    void Toggle() { visible = !visible; }
    bool IsVisible() const { return visible; }
    
    // Tab selection
    void SelectTab(int tab) { selectedTab = tab; }
    int GetSelectedTab() const { return selectedTab; }
    
    // Tab toggles
    void ToggleProfiler() { showProfiler = !showProfiler; }
    void ToggleMetrics() { showMetrics = !showMetrics; }
    void ToggleSettings() { showSettings = !showSettings; }
    void ToggleEntities() { showEntities = !showEntities; }
    void ToggleChunks() { showChunks = !showChunks; }
    void ToggleNetwork() { showNetwork = !showNetwork; }
    
    bool ShowProfiler() const { return showProfiler; }
    bool ShowMetrics() const { return showMetrics; }
    bool ShowSettings() const { return showSettings; }
    bool ShowEntities() const { return showEntities; }
    bool ShowChunks() const { return showChunks; }
    bool ShowNetwork() const { return showNetwork; }
};

// ============================================
// Main Debug System
// ============================================
class DebugSystem {
private:
    Profiler profiler;
    DebugMetrics metrics;
    DebugVisualizer visualizer;
    DebugMenu menu;
    
    bool enabled;
    bool pauseOnError;
    bool logToFile;
    std::string logPath;
    
public:
    DebugSystem();
    ~DebugSystem();
    
    // Lifecycle
    void Initialize();
    void Shutdown();
    void Update(float deltaTime);
    
    // Enable/disable
    void SetEnabled(bool enable) { enabled = enable; }
    bool IsEnabled() const { return enabled; }
    void Toggle() { enabled = !enabled; }
    
    // Access to subsystems
    Profiler& GetProfiler() { return profiler; }
    DebugMetrics& GetMetrics() { return metrics; }
    DebugVisualizer& GetVisualizer() { return visualizer; }
    DebugMenu& GetMenu() { return menu; }
    
    // Convenience methods
    void BeginProfile(const std::string& name) { profiler.BeginSample(name); }
    void EndProfile(const std::string& name) { profiler.EndSample(name); }
    
    // Error handling
    void SetPauseOnError(bool pause) { pauseOnError = pause; }
    bool PauseOnError() const { return pauseOnError; }
    
    // Logging
    void SetLogToFile(bool log) { logToFile = log; }
    bool LogToFile() const { return logToFile; }
    void SetLogPath(const std::string& path) { logPath = path; }
    
    // Snapshot - save current state for analysis
    void SaveSnapshot(const std::string& filename) const;
    
    // Quick checks
    void CheckMemoryLeaks();
    void ValidateWorldState();
    void CheckPerformanceBudget(float targetFrameTimeMs);
};

// Global accessor
DebugSystem& GetDebugSystem();

// Scoped profiler helper
class ScopedProfile {
private:
    std::string name;
    Profiler& profiler;
    
public:
    ScopedProfile(const std::string& name, Profiler& prof)
        : name(name), profiler(prof) {
        profiler.BeginSample(name);
    }
    
    ~ScopedProfile() {
        profiler.EndSample(name);
    }
};

// Macros for easy profiling
#define VGE_PROFILE(name) ScopedProfile _vge_profile_##__LINE__(name, GetDebugSystem().GetProfiler())
#define VGE_PROFILE_FUNC() VGE_PROFILE(__FUNCTION__)

} // namespace vge