#pragma once
#include "math/vec3.h"
#include <vector>
#include <cstdint>
#include <random>

namespace vge {

// ============================================
// Vec4 helper (RGBA color)
// ============================================
struct Vec4 {
    float x, y, z, w;
    Vec4() : x(0), y(0), z(0), w(0) {}
    Vec4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
    Vec4 operator*(float s) const { return Vec4(x*s, y*s, z*s, w*s); }
    Vec4 operator+(const Vec4& o) const { return Vec4(x+o.x, y+o.y, z+o.z, w+o.w); }
};

// ============================================
// Particle - Single particle instance
// ============================================
struct Particle {
    Vec3 position;
    Vec3 velocity;
    Vec3 acceleration;
    float life;           // Remaining life in seconds
    float maxLife;        // Initial life
    float size;           // Particle size
    float rotation;       // Rotation angle
    float rotationSpeed;  // Angular velocity
    Vec4 color;           // RGBA color
    float alpha;          // Transparency (0-1)
    bool active;          // Is particle alive?
    
    Particle()
        : position(0, 0, 0)
        , velocity(0, 0, 0)
        , acceleration(0, -9.8f, 0) // Gravity default
        , life(0.0f)
        , maxLife(1.0f)
        , size(1.0f)
        , rotation(0.0f)
        , rotationSpeed(0.0f)
        , color(1, 1, 1, 1)
        , alpha(1.0f)
        , active(false)
    {}
};

// ============================================
// Particle Emitter Configuration
// ============================================
struct EmitterConfig {
    Vec3 spawnPosition;        // World-space spawn point
    Vec3 spawnArea;            // Box area for random spawn (x,y,z half-extents)
    Vec3 spawnVelocity;        // Base velocity
    Vec3 velocityRandomness;   // Random velocity variation
    float spawnRate;           // Particles per second
    float particleLife;        // Base particle lifetime
    float lifeRandomness;      // Lifetime variation
    float particleSize;        // Base size
    float sizeRandomness;      // Size variation
    float sizeOverLife;        // Size multiplier at end of life (0 = shrink to nothing)
    Vec4 startColor;           // Color at birth
    Vec4 endColor;             // Color at death
    bool fadeOverLife;         // Fade alpha over lifetime
    bool shrinkOverLife;       // Shrink size over lifetime
    bool rotate;               // Enable rotation
    float rotationSpeed;       // Base rotation speed
    float rotationRandomness;  // Rotation variation
    Vec3 gravity;              // Gravity/acceleration
    float drag;                // Air resistance (0 = no drag, 1 = instant stop)
    int maxParticles;          // Maximum particles alive at once
    bool burstMode;            // Emit all at once then die
    int burstCount;            // Particles in burst
    bool loop;                 // Continuous emission
    bool worldSpace;           // Particles move in world space (true) or local (false)
    
    EmitterConfig()
        : spawnPosition(0, 10, 0)
        , spawnArea(5.0f, 0.0f, 5.0f)
        , spawnVelocity(0, -5.0f, 0)
        , velocityRandomness(1.0f, 1.0f, 1.0f)
        , spawnRate(100.0f)
        , particleLife(2.0f)
        , lifeRandomness(0.5f)
        , particleSize(0.1f)
        , sizeRandomness(0.05f)
        , sizeOverLife(0.0f)
        , startColor(0.6f, 0.7f, 0.9f, 0.8f)
        , endColor(0.6f, 0.7f, 0.9f, 0.0f)
        , fadeOverLife(true)
        , shrinkOverLife(true)
        , rotate(false)
        , rotationSpeed(0.0f)
        , rotationRandomness(0.0f)
        , gravity(0, -9.8f, 0)
        , drag(0.1f)
        , maxParticles(1000)
        , burstMode(false)
        , burstCount(50)
        , loop(true)
        , worldSpace(true)
    {}
};

// ============================================
// Particle System
// CPU-based particle simulation for weather
// effects (rain, snow, etc.)
// ============================================
class ParticleSystem {
private:
    std::vector<Particle> particles;
    EmitterConfig config;
    
    // Emission state
    float emissionAccumulator;
    bool emitting;
    int activeCount;
    
    // Random number generation
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist01;
    
    // Internal methods
    void EmitParticle();
    void EmitBurst();
    int FindDeadParticle();
    float RandomFloat(float min, float max);
    Vec3 RandomVec3(const Vec3& base, const Vec3& variance);
    
public:
    ParticleSystem();
    explicit ParticleSystem(const EmitterConfig& cfg);
    
    // Update all particles
    void Update(float deltaTime);
    
    // Emission control
    void StartEmission();
    void StopEmission();
    void EmitBurst(int count);
    bool IsEmitting() const { return emitting; }
    
    // Configuration
    void SetConfig(const EmitterConfig& cfg);
    const EmitterConfig& GetConfig() const { return config; }
    void SetSpawnPosition(const Vec3& pos) { config.spawnPosition = pos; }
    void SetSpawnArea(const Vec3& area) { config.spawnArea = area; }
    
    // Particle access
    const std::vector<Particle>& GetParticles() const { return particles; }
    int GetActiveCount() const { return activeCount; }
    int GetMaxParticles() const { return config.maxParticles; }
    
    // Reset all particles
    void Clear();
    
    // Render helpers (ASCII/framebuffer)
    void RenderASCII(int screenW, int screenH, const class Camera& camera) const;
    void RenderFB(class FramebufferRenderer* fb, const class Camera& camera) const;
};

} // namespace vge
