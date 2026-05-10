#pragma once
#include "math/vec3.h"
#include <vector>
#include <string>

namespace vge {

// ============================================
// Moon Phase System
// ============================================
enum class MoonPhase {
    NewMoon,      // No visible moon
    WaxingCrescent,
    FirstQuarter,
    WaxingGibbous,
    FullMoon,     // Fully visible
    WaningGibbous,
    LastQuarter,
    WaningCrescent
};

// ============================================
// Celestial Body (Sun / Moon)
// ============================================
struct CelestialBody {
    Vec3 position;      // Direction from world origin
    Vec3 color;         // Light color
    float intensity;    // 0.0 - 1.0
    float size;         // Angular size (for rendering)
    bool visible;       // Above horizon
    
    CelestialBody() 
        : position(0, 1, 0)
        , color(1, 1, 1)
        , intensity(1.0f)
        , size(1.0f)
        , visible(true) 
    {}
};

// ============================================
// Star
// ============================================
struct Star {
    Vec3 direction;     // Unit direction from observer
    float brightness;   // 0.0 - 1.0
    float twinklePhase; // For twinkling animation
    uint8_t colorIndex; // 0=bluish, 1=white, 2=yellowish, 3=reddish
    
    Star() 
        : direction(0, 1, 0)
        , brightness(1.0f)
        , twinklePhase(0.0f)
        , colorIndex(1) 
    {}
};

// ============================================
// Sky Color Presets
// ============================================
struct SkyColorSet {
    Vec3 topColor;
    Vec3 horizonColor;
    Vec3 bottomColor;
    Vec3 fogColor;
    float fogDensity;
    
    SkyColorSet() 
        : topColor(0.5f, 0.7f, 1.0f)
        , horizonColor(0.7f, 0.8f, 0.9f)
        , bottomColor(0.3f, 0.4f, 0.5f)
        , fogColor(0.7f, 0.8f, 0.9f)
        , fogDensity(0.01f) 
    {}
};

// ============================================
// Day/Night Cycle System
// ============================================
class DayNightCycle {
private:
    // Time
    float timeOfDay;        // 0.0 - 24.0 (hours)
    float dayLength;        // Real seconds per game day
    int dayCount;           // Days passed
    float timeScale;        // Speed multiplier
    bool paused;            // Pause time progression
    
    // Celestial bodies
    CelestialBody sun;
    CelestialBody moon;
    
    // Stars
    std::vector<Star> stars;
    static const int NUM_STARS = 500;
    float starTwinkleSpeed;
    
    // Moon phase
    MoonPhase currentMoonPhase;
    float moonPhaseProgress; // 0.0 - 1.0 within current phase
    
    // Sky colors
    SkyColorSet currentSky;
    SkyColorSet targetSky;
    float skyTransitionSpeed;
    
    // Ambient light
    Vec3 ambientLightColor;
    float ambientIntensity;
    
    // Internal methods
    void UpdateSunPosition();
    void UpdateMoonPosition();
    void UpdateMoonPhase();
    void UpdateSkyColors();
    void UpdateAmbientLight();
    void GenerateStars();
    
    // Color interpolation helpers
    Vec3 LerpColor(const Vec3& a, const Vec3& b, float t) const;
    SkyColorSet GetSkyColorsForTime(float normalizedTime) const;
    float EaseInOut(float t) const;
    
public:
    DayNightCycle();
    
    // Update
    void Update(float deltaTime);
    
    // Time control
    void SetTime(float hour);
    void SetTimeOfDayTicks(int ticks); // 0-24000 like Minecraft
    float GetTimeOfDay() const { return timeOfDay; }
    int GetTimeOfDayTicks() const;
    int GetDayCount() const { return dayCount; }
    float GetNormalizedTime() const; // 0.0 = midnight, 0.5 = noon
    
    void SetDayLength(float realSeconds);
    float GetDayLength() const { return dayLength; }
    
    void SetTimeScale(float scale) { timeScale = scale; }
    float GetTimeScale() const { return timeScale; }
    
    void Pause() { paused = true; }
    void Resume() { paused = false; }
    bool IsPaused() const { return paused; }
    
    // Celestial bodies
    const CelestialBody& GetSun() const { return sun; }
    const CelestialBody& GetMoon() const { return moon; }
    
    // Sun helpers
    Vec3 GetSunDirection() const { return sun.position; }
    float GetSunIntensity() const { return sun.intensity; }
    Vec3 GetSunColor() const { return sun.color; }
    bool IsSunVisible() const { return sun.visible; }
    
    // Moon helpers
    Vec3 GetMoonDirection() const { return moon.position; }
    float GetMoonIntensity() const { return moon.intensity; }
    Vec3 GetMoonColor() const { return moon.color; }
    MoonPhase GetMoonPhase() const { return currentMoonPhase; }
    std::string GetMoonPhaseName() const;
    float GetMoonPhaseBrightness() const; // 0.0 (new) to 1.0 (full)
    bool IsMoonVisible() const { return moon.visible; }
    
    // Stars
    const std::vector<Star>& GetStars() const { return stars; }
    float GetStarVisibility() const; // 0.0 (day) to 1.0 (night)
    
    // Sky
    const SkyColorSet& GetCurrentSky() const { return currentSky; }
    Vec3 GetSkyTopColor() const { return currentSky.topColor; }
    Vec3 GetSkyHorizonColor() const { return currentSky.horizonColor; }
    Vec3 GetFogColor() const { return currentSky.fogColor; }
    float GetFogDensity() const { return currentSky.fogDensity; }
    
    // Ambient light
    Vec3 GetAmbientLightColor() const { return ambientLightColor; }
    float GetAmbientIntensity() const { return ambientIntensity; }
    Vec3 GetTotalAmbientLight() const; // color * intensity
    
    // Day/Night queries
    bool IsDay() const;
    bool IsNight() const;
    bool IsDawn() const;   // Sunrise transition
    bool IsDusk() const;   // Sunset transition
    float GetDayNightBlend() const; // 0.0 = full night, 1.0 = full day
    
    // Time string
    std::string GetTimeString() const;
    std::string GetDayTimeString() const; // "Day 5, 14:30"
    
    // Fast-forward
    void SkipToDawn();
    void SkipToDusk();
    void SkipToNoon();
    void SkipToMidnight();
};

} // namespace vge
