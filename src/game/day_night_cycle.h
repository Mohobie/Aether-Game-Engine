#pragma once
#include "math/vec3.h"
#include <string>

namespace vge {

// ============================================
// Moon Phase System
// ============================================
enum class MoonPhase {
    NewMoon,          // No visible moon
    WaxingCrescent,
    FirstQuarter,
    WaxingGibbous,
    FullMoon,         // Fully visible
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
// Day/Night Cycle System (Game Integration)
// ============================================
// Time is stored as 0.0-1.0 where:
//   0.0 / 1.0 = dawn (6:00 AM)
//   0.25      = noon (12:00 PM)
//   0.5       = dusk (6:00 PM)
//   0.75      = midnight (12:00 AM)
//
// 1 game day = 20 real minutes (1200 seconds)
// ============================================
class DayNightCycle {
private:
    // Time (0.0-1.0, wraps)
    float timeOfDay;
    float dayLength;    // Real seconds per game day (default: 1200)
    int dayCount;
    float timeScale;
    bool paused;

    // Celestial bodies
    CelestialBody sun;
    CelestialBody moon;

    // Moon phase
    MoonPhase currentMoonPhase;
    float moonPhaseProgress;

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

    // Color interpolation helpers
    Vec3 LerpColor(const Vec3& a, const Vec3& b, float t) const;
    SkyColorSet GetSkyColorsForTime(float normalizedTime) const;
    float EaseInOut(float t) const;

public:
    DayNightCycle();

    // Update
    void Update(float deltaTime);

    // Time control (0.0-1.0 cycle)
    void SetTime(float t);
    float GetTimeOfDay() const { return timeOfDay; }
    int GetDayCount() const { return dayCount; }
    float GetNormalizedTime() const { return timeOfDay; }

    // Time in hours (0-24) for display
    float GetTimeInHours() const;
    std::string GetTimeString() const;
    std::string GetDayTimeString() const;

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
    float GetMoonPhaseBrightness() const;
    bool IsMoonVisible() const { return moon.visible; }

    // Sky
    const SkyColorSet& GetCurrentSky() const { return currentSky; }
    Vec3 GetSkyTopColor() const { return currentSky.topColor; }
    Vec3 GetSkyHorizonColor() const { return currentSky.horizonColor; }
    Vec3 GetSkyBottomColor() const { return currentSky.bottomColor; }
    Vec3 GetFogColor() const { return currentSky.fogColor; }
    float GetFogDensity() const { return currentSky.fogDensity; }

    // Ambient light
    Vec3 GetAmbientLightColor() const { return ambientLightColor; }
    float GetAmbientIntensity() const { return ambientIntensity; }
    Vec3 GetTotalAmbientLight() const;

    // Day/Night queries
    bool IsDay() const;
    bool IsNight() const;
    bool IsDawn() const;
    bool IsDusk() const;
    float GetDayNightBlend() const; // 0.0 = full night, 1.0 = full day

    // Star visibility
    float GetStarVisibility() const;

    // Fast-forward
    void SkipToDawn();
    void SkipToNoon();
    void SkipToDusk();
    void SkipToMidnight();
};

} // namespace vge
