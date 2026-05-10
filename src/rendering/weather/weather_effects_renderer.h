#pragma once
#include "rendering/particles/particle_system.h"
#include "core/weather_system.h"
#include "math/vec3.h"
#include <vector>
#include <memory>

namespace vge {

// Forward declarations
class DayNightCycle;
class FramebufferRenderer;
class Camera;

// ============================================
// Weather Effect Types
// ============================================
enum class PrecipitationType {
    Rain,       // Fast falling, blue-tinted
    Snow,       // Slow falling, white, drifting
    Hail        // Fast falling, white, bouncing
};

// ============================================
// Lightning Flash
// ============================================
struct LightningFlash {
    float intensity;      // 0-1 brightness
    float duration;       // Flash duration in seconds
    float timer;          // Time remaining
    bool active;
    
    LightningFlash() : intensity(0.0f), duration(0.0f), timer(0.0f), active(false) {}
};

// ============================================
// Volumetric Fog Layer
// ============================================
struct FogLayer {
    float baseHeight;     // Fog starts at this Y
    float thickness;      // Fog thickness in world units
    float density;        // 0-1 opacity
    Vec3 color;           // Fog color
    float turbulence;     // Fog movement amount
    float turbulenceSpeed;// Movement speed
    float phase;          // Animation phase
    
    FogLayer()
        : baseHeight(0.0f)
        , thickness(10.0f)
        , density(0.5f)
        , color(0.7f, 0.75f, 0.8f)
        , turbulence(0.5f)
        , turbulenceSpeed(0.3f)
        , phase(0.0f)
    {}
};

// ============================================
// Wind Field
// ============================================
struct WindField {
    Vec3 direction;       // Primary wind direction
    float baseSpeed;      // Base wind speed
    float gustiness;      // How much speed varies
    float turbulence;     // Direction variation
    float currentSpeed;   // Current speed (animated)
    Vec3 currentDirection; // Current direction (animated)
    float phase;          // Animation phase
    
    WindField()
        : direction(1.0f, 0.0f, 0.0f)
        , baseSpeed(2.0f)
        , gustiness(0.5f)
        , turbulence(0.2f)
        , currentSpeed(2.0f)
        , currentDirection(1.0f, 0.0f, 0.0f)
        , phase(0.0f)
    {}
};

// ============================================
// Sky Overlay
// Dynamic sky color modifications based on weather
// ============================================
struct SkyOverlay {
    Vec3 overlayColor;    // Color to blend with sky
    float overlayStrength; // 0-1 blend factor
    float cloudDarkening;  // How much clouds darken the sky
    float stormDarkness;   // Additional darkness during storms
    
    SkyOverlay()
        : overlayColor(0.5f, 0.55f, 0.6f)
        , overlayStrength(0.0f)
        , cloudDarkening(0.0f)
        , stormDarkness(0.0f)
    {}
};

// ============================================
// Weather Effects Renderer
// Integrates particle systems, fog, wind, and
// sky overlays with the weather system
// ============================================
class WeatherEffectsRenderer {
private:
    // Particle systems for precipitation
    std::unique_ptr<ParticleSystem> rainParticles;
    std::unique_ptr<ParticleSystem> snowParticles;
    std::unique_ptr<ParticleSystem> stormParticles;
    
    // Weather state
    WeatherSystem* weatherSystem;
    DayNightCycle* dayNightCycle;
    
    // Effects
    LightningFlash lightning;
    FogLayer fogLayer;
    WindField wind;
    SkyOverlay skyOverlay;
    
    // Internal state
    float stormTimer;
    float nextLightningTime;
    float windPhase;
    
    // Screen-space rain/snow rendering (for ASCII)
    struct ScreenDrop {
        int x, y;
        float speed;
        float length;
        char character;
        float life;
    };
    std::vector<ScreenDrop> screenDrops;
    
    // Configuration
    int maxScreenDrops;
    float screenDropSpawnRate;
    float screenDropAccumulator;
    
    // Internal methods
    void UpdateRain(float deltaTime, const Camera& camera);
    void UpdateSnow(float deltaTime, const Camera& camera);
    void UpdateStorm(float deltaTime, const Camera& camera);
    void UpdateFog(float deltaTime);
    void UpdateWind(float deltaTime);
    void UpdateSkyOverlay(float deltaTime);
    void UpdateLightning(float deltaTime);
    void SpawnLightning();
    
    void ConfigureRainEmitter(const Camera& camera);
    void ConfigureSnowEmitter(const Camera& camera);
    void ConfigureStormEmitter(const Camera& camera);
    
    EmitterConfig GetRainConfig(const Camera& camera) const;
    EmitterConfig GetSnowConfig(const Camera& camera) const;
    EmitterConfig GetStormConfig(const Camera& camera) const;
    
    char GetRainChar(float intensity) const;
    char GetSnowChar() const;
    uint32_t GetRainColor(float intensity) const;
    uint32_t GetSnowColor(float intensity) const;
    
public:
    WeatherEffectsRenderer(WeatherSystem* weather, DayNightCycle* dayNight);
    
    // Update all weather effects
    void Update(float deltaTime, const Camera& camera);
    
    // Render to ASCII/console
    void RenderASCII(int screenW, int screenH);
    
    // Render to framebuffer
    void RenderFB(FramebufferRenderer* fb, const Camera& camera);
    
    // Accessors
    const WindField& GetWind() const { return wind; }
    const FogLayer& GetFog() const { return fogLayer; }
    const SkyOverlay& GetSkyOverlay() const { return skyOverlay; }
    bool IsLightningActive() const { return lightning.active; }
    float GetLightningIntensity() const { return lightning.active ? lightning.intensity : 0.0f; }
    
    // Manual control
    void TriggerLightning();
    void SetFogDensity(float density) { fogLayer.density = density; }
    void SetWindSpeed(float speed) { wind.baseSpeed = speed; }
    void SetWindDirection(const Vec3& dir) { wind.direction = dir; }
    
    // Get modified sky colors (blends weather overlay with day/night)
    Vec3 GetModifiedSkyTopColor() const;
    Vec3 GetModifiedSkyHorizonColor() const;
    Vec3 GetModifiedFogColor() const;
    float GetModifiedFogDensity() const;
};

} // namespace vge
