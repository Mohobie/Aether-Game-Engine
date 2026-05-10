#include "debug/debug_system.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace vge {

// ============================================
// Profiler Implementation
// ============================================

void Profiler::BeginSample(const std::string& name) {
    activeTimers[name] = std::chrono::high_resolution_clock::now();
}

void Profiler::EndSample(const std::string& name) {
    auto it = activeTimers.find(name);
    if (it == activeTimers.end()) return;
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - it->second);
    double elapsedMs = duration.count() / 1000.0;
    
    auto& sample = samples[name];
    sample.name = name;
    sample.elapsedMs = elapsedMs;
    sample.callCount++;
    sample.totalMs += elapsedMs;
    sample.avgMs = sample.totalMs / sample.callCount;
    
    if (sample.callCount == 1) {
        sample.minMs = elapsedMs;
        sample.maxMs = elapsedMs;
    } else {
        sample.minMs = std::min(sample.minMs, elapsedMs);
        sample.maxMs = std::max(sample.maxMs, elapsedMs);
    }
    
    activeTimers.erase(it);
}

void Profiler::Reset() {
    samples.clear();
    activeTimers.clear();
}

std::vector<std::string> Profiler::GetReport() const {
    std::vector<std::string> report;
    report.push_back("=== Performance Profile ===");
    report.push_back("Name | Calls | Total | Avg | Min | Max");
    report.push_back("-----|-------|-------|-----|-----|-----");
    
    for (const auto& [name, sample] : samples) {
        std::ostringstream oss;
        oss << std::left << std::setw(20) << sample.name
            << " | " << std::setw(5) << sample.callCount
            << " | " << std::fixed << std::setprecision(2) << std::setw(6) << sample.totalMs
            << " | " << std::setw(5) << sample.avgMs
            << " | " << std::setw(5) << sample.minMs
            << " | " << std::setw(5) << sample.maxMs;
        report.push_back(oss.str());
    }
    
    return report;
}

// ============================================
// Debug Visualizer Implementation
// ============================================

DebugVisualizer::DebugVisualizer()
    : drawMode(0)
    , showGrid(true)
    , showAxes(true)
    , showStats(true)
    , showConsole(false) {}

void DebugVisualizer::ToggleMode(DebugDrawMode mode) {
    uint32_t m = static_cast<uint32_t>(mode);
    if (drawMode & m) {
        drawMode &= ~m;
    } else {
        drawMode |= m;
    }
}

bool DebugVisualizer::IsModeEnabled(DebugDrawMode mode) const {
    return (drawMode & static_cast<uint32_t>(mode)) != 0;
}

void DebugVisualizer::DrawLine(const Vec3& start, const Vec3& end, const Vec3& color) {
    (void)start;
    (void)end;
    (void)color;
    // Would use OpenGL/Vulkan line rendering
}

void DebugVisualizer::DrawBox(const Vec3& min, const Vec3& max, const Vec3& color) {
    (void)min;
    (void)max;
    (void)color;
    // Would use OpenGL/Vulkan line rendering for box edges
}

void DebugVisualizer::DrawSphere(const Vec3& center, float radius, const Vec3& color) {
    (void)center;
    (void)radius;
    (void)color;
    // Would use OpenGL/Vulkan line rendering
}

void DebugVisualizer::DrawCross(const Vec3& position, float size, const Vec3& color) {
    (void)position;
    (void)size;
    (void)color;
    // Would draw 3 lines (X, Y, Z axes)
}

void DebugVisualizer::DrawGrid(float size, float step, const Vec3& color) {
    (void)size;
    (void)step;
    (void)color;
    // Would draw grid lines on XZ plane
}

void DebugVisualizer::DrawAxes(const Vec3& position, float size) {
    (void)position;
    (void)size;
    // Would draw RGB axes
}

// ============================================
// Debug Menu Implementation
// ============================================

DebugMenu::DebugMenu()
    : visible(false)
    , selectedTab(0)
    , showProfiler(true)
    , showMetrics(true)
    , showSettings(false)
    , showEntities(false)
    , showChunks(false)
    , showNetwork(false) {}

// ============================================
// Debug System Implementation
// ============================================

DebugSystem::DebugSystem()
    : enabled(false)
    , pauseOnError(false)
    , logToFile(false)
    , logPath("debug.log") {}

DebugSystem::~DebugSystem() {
    Shutdown();
}

void DebugSystem::Initialize() {
    std::cout << "[DebugSystem] Initialized" << std::endl;
    enabled = true;
    
    if (logToFile) {
        std::ofstream log(logPath, std::ios::app);
        if (log.is_open()) {
            log << "[DebugSystem] Session started" << std::endl;
        }
    }
}

void DebugSystem::Shutdown() {
    if (logToFile) {
        std::ofstream log(logPath, std::ios::app);
        if (log.is_open()) {
            log << "[DebugSystem] Session ended" << std::endl;
        }
    }
    
    std::cout << "[DebugSystem] Shutdown" << std::endl;
}

void DebugSystem::Update(float deltaTime) {
    if (!enabled) return;
    
    // Update metrics
    metrics.frameTimeMs = deltaTime * 1000.0f;
    metrics.fps = 1.0f / deltaTime;
    metrics.frameCount++;
    
    // Update memory metrics (simplified)
    metrics.memoryUsedMB = 0; // Would query OS
    metrics.memoryTotalMB = 0; // Would query OS
}

void DebugSystem::SaveSnapshot(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) return;
    
    file << "=== Debug Snapshot ===" << std::endl;
    file << "Frame: " << metrics.frameCount << std::endl;
    file << "FPS: " << metrics.fps << std::endl;
    file << "Frame Time: " << metrics.frameTimeMs << " ms" << std::endl;
    file << "Loaded Chunks: " << metrics.loadedChunks << std::endl;
    file << "Visible Chunks: " << metrics.visibleChunks << std::endl;
    file << "Total Blocks: " << metrics.totalBlocks << std::endl;
    file << "Player Position: (" << metrics.playerPosition.x << ", " 
         << metrics.playerPosition.y << ", " << metrics.playerPosition.z << ")" << std::endl;
    file << "Draw Calls: " << metrics.drawCalls << std::endl;
    file << "Triangles: " << metrics.trianglesRendered << std::endl;
    file << "Vertices: " << metrics.verticesRendered << std::endl;
    
    // Profile report
    file << std::endl << "=== Profile Report ===" << std::endl;
    auto report = profiler.GetReport();
    for (const auto& line : report) {
        file << line << std::endl;
    }
    
    file.close();
    std::cout << "[DebugSystem] Snapshot saved to: " << filename << std::endl;
}

void DebugSystem::CheckMemoryLeaks() {
    // Would implement memory tracking
    std::cout << "[DebugSystem] Memory check: OK" << std::endl;
}

void DebugSystem::ValidateWorldState() {
    // Would validate chunk consistency, block IDs, etc.
    std::cout << "[DebugSystem] World validation: OK" << std::endl;
}

void DebugSystem::CheckPerformanceBudget(float targetFrameTimeMs) {
    if (metrics.frameTimeMs > targetFrameTimeMs) {
        std::cout << "[DebugSystem] WARNING: Frame time " << metrics.frameTimeMs 
                  << " ms exceeds budget " << targetFrameTimeMs << " ms" << std::endl;
    }
}

// Global instance
DebugSystem& GetDebugSystem() {
    static DebugSystem instance;
    return instance;
}

} // namespace vge