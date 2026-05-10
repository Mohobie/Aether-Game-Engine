#include "rendering/particles/particle_system.h"
#include "rendering/camera.h"
#include "rendering/framebuffer_renderer.h"
#include <cmath>
#include <algorithm>

namespace vge {

// ============================================
// Constructor
// ============================================
ParticleSystem::ParticleSystem()
    : emissionAccumulator(0.0f)
    , emitting(false)
    , activeCount(0)
    , rng(std::random_device{}())
    , dist01(0.0f, 1.0f)
{
    particles.reserve(config.maxParticles);
    for (int i = 0; i < config.maxParticles; ++i) {
        particles.emplace_back();
    }
}

ParticleSystem::ParticleSystem(const EmitterConfig& cfg)
    : config(cfg)
    , emissionAccumulator(0.0f)
    , emitting(false)
    , activeCount(0)
    , rng(std::random_device{}())
    , dist01(0.0f, 1.0f)
{
    particles.reserve(config.maxParticles);
    for (int i = 0; i < config.maxParticles; ++i) {
        particles.emplace_back();
    }
}

// ============================================
// Random Helpers
// ============================================
float ParticleSystem::RandomFloat(float min, float max) {
    return min + dist01(rng) * (max - min);
}

Vec3 ParticleSystem::RandomVec3(const Vec3& base, const Vec3& variance) {
    return Vec3(
        base.x + RandomFloat(-variance.x, variance.x),
        base.y + RandomFloat(-variance.y, variance.y),
        base.z + RandomFloat(-variance.z, variance.z)
    );
}

// ============================================
// Find Dead Particle
// ============================================
int ParticleSystem::FindDeadParticle() {
    for (size_t i = 0; i < particles.size(); ++i) {
        if (!particles[i].active) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// ============================================
// Emit Single Particle
// ============================================
void ParticleSystem::EmitParticle() {
    int index = FindDeadParticle();
    if (index < 0) return;
    
    Particle& p = particles[index];
    p.active = true;
    
    // Position
    p.position = RandomVec3(config.spawnPosition, config.spawnArea);
    
    // Velocity
    p.velocity = RandomVec3(config.spawnVelocity, config.velocityRandomness);
    
    // Life
    p.maxLife = config.particleLife + RandomFloat(-config.lifeRandomness, config.lifeRandomness);
    p.life = p.maxLife;
    if (p.life <= 0.0f) p.life = 0.1f;
    
    // Size
    p.size = config.particleSize + RandomFloat(-config.sizeRandomness, config.sizeRandomness);
    if (p.size <= 0.0f) p.size = 0.01f;
    
    // Rotation
    p.rotation = 0.0f;
    p.rotationSpeed = config.rotationSpeed + RandomFloat(-config.rotationRandomness, config.rotationRandomness);
    
    // Alpha
    p.alpha = 0.8f;
    
    // Physics
    p.acceleration = config.gravity;
    
    activeCount++;
}

// ============================================
// Emit Burst
// ============================================
void ParticleSystem::EmitBurst(int count) {
    int emitCount = std::min(count, config.maxParticles);
    for (int i = 0; i < emitCount; ++i) {
        EmitParticle();
    }
}

// ============================================
// Update
// ============================================
void ParticleSystem::Update(float deltaTime) {
    // Emit new particles
    if (emitting && !config.burstMode) {
        emissionAccumulator += deltaTime * config.spawnRate;
        while (emissionAccumulator >= 1.0f) {
            EmitParticle();
            emissionAccumulator -= 1.0f;
        }
    }
    
    // Update existing particles
    activeCount = 0;
    for (auto& p : particles) {
        if (!p.active) continue;
        
        // Update life
        p.life -= deltaTime;
        if (p.life <= 0.0f) {
            p.active = false;
            continue;
        }
        
        // Update physics
        p.velocity = p.velocity + p.acceleration * deltaTime;
        p.velocity = p.velocity * (1.0f - config.drag * deltaTime);
        p.position = p.position + p.velocity * deltaTime;
        
        // Update rotation
        if (config.rotate) {
            p.rotation += p.rotationSpeed * deltaTime;
        }
        
        // Life ratio (0 = birth, 1 = death)
        float lifeRatio = 1.0f - (p.life / p.maxLife);
        
        // Update size
        if (config.shrinkOverLife) {
            p.size = p.size * (1.0f - lifeRatio * (1.0f - config.sizeOverLife));
            // Note: actual size is computed at render time
        }
        
        // Update alpha
        if (config.fadeOverLife) {
            float alphaMultiplier = 1.0f - lifeRatio;
            p.alpha = alphaMultiplier;
        }
        
        activeCount++;
    }
}

// ============================================
// Emission Control
// ============================================
void ParticleSystem::StartEmission() {
    emitting = true;
    if (config.burstMode) {
        EmitBurst(config.burstCount);
        if (!config.loop) {
            emitting = false;
        }
    }
}

void ParticleSystem::StopEmission() {
    emitting = false;
}

// ============================================
// Configuration
// ============================================
void ParticleSystem::SetConfig(const EmitterConfig& cfg) {
    config = cfg;
    // Resize particle pool if needed
    if (static_cast<int>(particles.size()) < config.maxParticles) {
        int oldSize = static_cast<int>(particles.size());
        particles.reserve(config.maxParticles);
        for (int i = oldSize; i < config.maxParticles; ++i) {
            particles.emplace_back();
        }
    }
}

// ============================================
// Clear
// ============================================
void ParticleSystem::Clear() {
    for (auto& p : particles) {
        p.active = false;
    }
    activeCount = 0;
    emissionAccumulator = 0.0f;
}

// ============================================
// ASCII Rendering
// ============================================
void ParticleSystem::RenderASCII(int screenW, int screenH, const Camera& camera) const {
    (void)screenW;
    (void)screenH;
    (void)camera;
    // Simplified ASCII rendering - just show particle count
    // In full implementation, project particles to screen
}

// ============================================
// Framebuffer Rendering
// ============================================
void ParticleSystem::RenderFB(FramebufferRenderer* fb, const Camera& camera) const {
    if (!fb || !fb->IsInitialized()) return;
    
    int fbW = fb->GetWidth();
    int fbH = fb->GetHeight();
    
    for (const auto& p : particles) {
        if (!p.active) continue;
        
        // Simple projection to screen
        Vec3 relative = p.position - camera.GetPosition();
        
        // Skip if behind camera
        if (relative.z <= 0.1f) continue;
        
        // Simple perspective projection
        float screenX = (relative.x / relative.z) * fbW * 0.5f + fbW * 0.5f;
        float screenY = (-relative.y / relative.z) * fbH * 0.5f + fbH * 0.5f;
        
        int px = static_cast<int>(screenX);
        int py = static_cast<int>(screenY);
        
        if (px < 0 || px >= fbW || py < 0 || py >= fbH) continue;
        
        // Particle size in screen pixels
        float distance = relative.length();
        float screenSize = std::max(1.0f, p.size * 50.0f / distance);
        int size = static_cast<int>(screenSize);
        
        // Draw particle as small square
        uint8_t r = static_cast<uint8_t>(std::min(255.0f, 255.0f * p.alpha));
        uint8_t g = static_cast<uint8_t>(std::min(255.0f, 255.0f * p.alpha));
        uint8_t b = static_cast<uint8_t>(std::min(255.0f, 255.0f * p.alpha));
        uint32_t color = RGB(r, g, b);
        
        for (int dy = -size/2; dy <= size/2; dy++) {
            for (int dx = -size/2; dx <= size/2; dx++) {
                fb->SetPixel(px + dx, py + dy, color);
            }
        }
    }
}

} // namespace vge
