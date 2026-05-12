#include "game/day_night_cycle.h"
#include <cmath>
#include <cstdlib>
#include <ctime>

namespace vge {

// ============================================
// Constructor
// ============================================
DayNightCycle::DayNightCycle()
    : timeOfDay(0.0f)       // Start at dawn
    , dayLength(1200.0f)    // 20 minutes per day
    , dayCount(0)
    , timeScale(1.0f)
    , paused(false)
    , currentMoonPhase(MoonPhase::FullMoon)
    , moonPhaseProgress(0.0f)
    , skyTransitionSpeed(2.0f)
    , ambientIntensity(0.5f)
{
    // Initialize celestial positions
    UpdateSunPosition();
    UpdateMoonPosition();
    UpdateMoonPhase();
    UpdateSkyColors();
    UpdateAmbientLight();
}

// ============================================
// Update
// ============================================
void DayNightCycle::Update(float deltaTime) {
    if (paused) return;

    // Advance time: 1.0 = full day cycle
    float timeAdvance = (deltaTime / dayLength) * timeScale;
    timeOfDay += timeAdvance;

    // Wrap around
    while (timeOfDay >= 1.0f) {
        timeOfDay -= 1.0f;
        dayCount++;

        // Advance moon phase every 3 days
        if (dayCount % 3 == 0) {
            int phaseIdx = static_cast<int>(currentMoonPhase);
            phaseIdx = (phaseIdx + 1) % 8;
            currentMoonPhase = static_cast<MoonPhase>(phaseIdx);
            moonPhaseProgress = 0.0f;
        }
    }

    // Update celestial bodies
    UpdateSunPosition();
    UpdateMoonPosition();
    UpdateMoonPhase();
    UpdateSkyColors();
    UpdateAmbientLight();
}

// ============================================
// Sun Position
// ============================================
void DayNightCycle::UpdateSunPosition() {
    float t = timeOfDay; // 0.0 = dawn, 0.25 = noon, 0.5 = dusk, 0.75 = midnight

    // Sun angle: 0 at dawn, PI at dusk, below horizon at night
    // Map timeOfDay to sun angle:
    //   dawn (0.0)   -> -PI/2 (rising)
    //   noon (0.25)  -> 0 (zenith)
    //   dusk (0.5)   -> PI/2 (setting)
    //   night        -> below horizon
    float sunAngle;
    if (t <= 0.5f) {
        // Daytime: dawn to dusk
        sunAngle = (t / 0.5f) * 3.14159f - 1.5708f; // -PI/2 to PI/2
    } else {
        // Night: continue below horizon
        sunAngle = ((t - 0.5f) / 0.5f) * 3.14159f + 1.5708f; // PI/2 to 3PI/2
    }

    // Position on hemisphere
    sun.position.x = std::cos(sunAngle);
    sun.position.y = std::sin(sunAngle);
    sun.position.z = 0.3f * std::cos(sunAngle * 0.5f);

    // Normalize
    float len = sun.position.length();
    if (len > 0.0f) {
        sun.position = sun.position / len;
    }

    // Visibility and intensity
    sun.visible = sun.position.y > -0.1f;

    // Intensity based on height
    float heightFactor = std::max(0.0f, sun.position.y);
    sun.intensity = heightFactor * heightFactor;

    // Sun color based on time
    if (t < 0.05f || t > 0.55f) {
        // Night - no sun
        sun.color = Vec3(0.0f, 0.0f, 0.0f);
        sun.intensity = 0.0f;
    } else if (t < 0.1f) {
        // Dawn - warm orange
        float blend = (t - 0.05f) / 0.05f;
        sun.color = LerpColor(Vec3(1.0f, 0.4f, 0.1f), Vec3(1.0f, 0.9f, 0.7f), blend);
    } else if (t < 0.15f) {
        // Morning - yellow-white
        float blend = (t - 0.1f) / 0.05f;
        sun.color = LerpColor(Vec3(1.0f, 0.9f, 0.7f), Vec3(1.0f, 1.0f, 0.95f), blend);
    } else if (t < 0.35f) {
        // Day - bright white
        sun.color = Vec3(1.0f, 1.0f, 0.98f);
    } else if (t < 0.4f) {
        // Late afternoon - warm
        float blend = (t - 0.35f) / 0.05f;
        sun.color = LerpColor(Vec3(1.0f, 1.0f, 0.98f), Vec3(1.0f, 0.85f, 0.6f), blend);
    } else if (t < 0.5f) {
        // Dusk - orange/red
        float blend = (t - 0.4f) / 0.1f;
        sun.color = LerpColor(Vec3(1.0f, 0.85f, 0.6f), Vec3(1.0f, 0.4f, 0.1f), blend);
    } else {
        sun.color = Vec3(0.0f, 0.0f, 0.0f);
        sun.intensity = 0.0f;
    }

    sun.size = 1.0f;
}

// ============================================
// Moon Position
// ============================================
void DayNightCycle::UpdateMoonPosition() {
    float t = timeOfDay;

    // Moon is opposite to sun (roughly)
    float moonAngle = (t + 0.5f) * 3.14159f * 2.0f;
    if (moonAngle > 6.28318f) moonAngle -= 6.28318f;

    moon.position.x = std::cos(moonAngle);
    moon.position.y = std::sin(moonAngle);
    moon.position.z = -0.2f * std::cos(moonAngle * 0.3f);

    // Normalize
    float len = moon.position.length();
    if (len > 0.0f) {
        moon.position = moon.position / len;
    }

    // Visibility
    moon.visible = moon.position.y > -0.15f;

    // Intensity based on phase
    float phaseBrightness = GetMoonPhaseBrightness();
    float heightFactor = std::max(0.0f, moon.position.y);
    moon.intensity = phaseBrightness * heightFactor * 0.3f;

    // Moon color - slightly blue-white
    moon.color = Vec3(0.8f, 0.85f, 1.0f) * phaseBrightness;
    moon.size = 0.8f;
}

// ============================================
// Moon Phase
// ============================================
void DayNightCycle::UpdateMoonPhase() {
    float daysSincePhaseChange = static_cast<float>(dayCount % 3) + timeOfDay;
    moonPhaseProgress = daysSincePhaseChange / 3.0f;
    if (moonPhaseProgress > 1.0f) moonPhaseProgress = 1.0f;
}

// ============================================
// Sky Colors
// ============================================
SkyColorSet DayNightCycle::GetSkyColorsForTime(float t) const {
    SkyColorSet colors;

    if (t < 0.05f || t > 0.55f) {
        // Night - dark blue/black
        colors.topColor = Vec3(0.02f, 0.03f, 0.08f);
        colors.horizonColor = Vec3(0.05f, 0.06f, 0.12f);
        colors.bottomColor = Vec3(0.01f, 0.02f, 0.05f);
        colors.fogColor = Vec3(0.05f, 0.06f, 0.12f);
        colors.fogDensity = 0.02f;
    } else if (t < 0.1f) {
        // Dawn
        float blend = EaseInOut((t - 0.05f) / 0.05f);
        colors.topColor = LerpColor(Vec3(0.02f, 0.03f, 0.08f), Vec3(0.4f, 0.5f, 0.7f), blend);
        colors.horizonColor = LerpColor(Vec3(0.05f, 0.06f, 0.12f), Vec3(1.0f, 0.5f, 0.2f), blend);
        colors.bottomColor = LerpColor(Vec3(0.01f, 0.02f, 0.05f), Vec3(0.3f, 0.3f, 0.4f), blend);
        colors.fogColor = colors.horizonColor;
        colors.fogDensity = 0.01f + blend * 0.01f;
    } else if (t < 0.15f) {
        // Morning
        float blend = EaseInOut((t - 0.1f) / 0.05f);
        colors.topColor = LerpColor(Vec3(0.4f, 0.5f, 0.7f), Vec3(0.5f, 0.7f, 1.0f), blend);
        colors.horizonColor = LerpColor(Vec3(1.0f, 0.5f, 0.2f), Vec3(0.8f, 0.9f, 1.0f), blend);
        colors.bottomColor = LerpColor(Vec3(0.3f, 0.3f, 0.4f), Vec3(0.5f, 0.6f, 0.7f), blend);
        colors.fogColor = colors.horizonColor;
        colors.fogDensity = 0.01f;
    } else if (t < 0.35f) {
        // Day
        colors.topColor = Vec3(0.5f, 0.7f, 1.0f);
        colors.horizonColor = Vec3(0.8f, 0.9f, 1.0f);
        colors.bottomColor = Vec3(0.5f, 0.6f, 0.7f);
        colors.fogColor = Vec3(0.8f, 0.9f, 1.0f);
        colors.fogDensity = 0.005f;
    } else if (t < 0.4f) {
        // Late afternoon
        float blend = EaseInOut((t - 0.35f) / 0.05f);
        colors.topColor = LerpColor(Vec3(0.5f, 0.7f, 1.0f), Vec3(0.4f, 0.5f, 0.7f), blend);
        colors.horizonColor = LerpColor(Vec3(0.8f, 0.9f, 1.0f), Vec3(1.0f, 0.6f, 0.3f), blend);
        colors.bottomColor = LerpColor(Vec3(0.5f, 0.6f, 0.7f), Vec3(0.4f, 0.4f, 0.5f), blend);
        colors.fogColor = colors.horizonColor;
        colors.fogDensity = 0.01f + blend * 0.01f;
    } else if (t < 0.55f) {
        // Dusk
        float blend = EaseInOut((t - 0.4f) / 0.15f);
        colors.topColor = LerpColor(Vec3(0.4f, 0.5f, 0.7f), Vec3(0.02f, 0.03f, 0.08f), blend);
        colors.horizonColor = LerpColor(Vec3(1.0f, 0.6f, 0.3f), Vec3(0.1f, 0.1f, 0.2f), blend);
        colors.bottomColor = LerpColor(Vec3(0.4f, 0.4f, 0.5f), Vec3(0.01f, 0.02f, 0.05f), blend);
        colors.fogColor = colors.horizonColor;
        colors.fogDensity = 0.02f + blend * 0.01f;
    }

    return colors;
}

void DayNightCycle::UpdateSkyColors() {
    float t = timeOfDay;
    targetSky = GetSkyColorsForTime(t);

    // Smooth transition
    float dt = 1.0f / skyTransitionSpeed;
    currentSky.topColor = LerpColor(currentSky.topColor, targetSky.topColor, dt);
    currentSky.horizonColor = LerpColor(currentSky.horizonColor, targetSky.horizonColor, dt);
    currentSky.bottomColor = LerpColor(currentSky.bottomColor, targetSky.bottomColor, dt);
    currentSky.fogColor = LerpColor(currentSky.fogColor, targetSky.fogColor, dt);
    currentSky.fogDensity = currentSky.fogDensity + (targetSky.fogDensity - currentSky.fogDensity) * dt;
}

// ============================================
// Ambient Light
// ============================================
void DayNightCycle::UpdateAmbientLight() {
    float t = timeOfDay;

    if (t < 0.05f || t > 0.55f) {
        // Night - very dim, blue-tinted
        ambientLightColor = Vec3(0.1f, 0.15f, 0.25f);
        ambientIntensity = 0.1f;
    } else if (t < 0.1f) {
        // Dawn - increasing warm light
        float blend = (t - 0.05f) / 0.05f;
        ambientLightColor = LerpColor(Vec3(0.1f, 0.15f, 0.25f), Vec3(0.5f, 0.45f, 0.35f), blend);
        ambientIntensity = 0.1f + blend * 0.3f;
    } else if (t < 0.15f) {
        // Morning - brightening
        float blend = (t - 0.1f) / 0.05f;
        ambientLightColor = LerpColor(Vec3(0.5f, 0.45f, 0.35f), Vec3(0.7f, 0.7f, 0.65f), blend);
        ambientIntensity = 0.4f + blend * 0.2f;
    } else if (t < 0.35f) {
        // Day - full ambient
        ambientLightColor = Vec3(0.7f, 0.7f, 0.65f);
        ambientIntensity = 0.6f;
    } else if (t < 0.4f) {
        // Late afternoon - dimming
        float blend = (t - 0.35f) / 0.05f;
        ambientLightColor = LerpColor(Vec3(0.7f, 0.7f, 0.65f), Vec3(0.5f, 0.45f, 0.35f), blend);
        ambientIntensity = 0.6f - blend * 0.2f;
    } else if (t < 0.55f) {
        // Dusk - fading to night
        float blend = (t - 0.4f) / 0.15f;
        ambientLightColor = LerpColor(Vec3(0.5f, 0.45f, 0.35f), Vec3(0.1f, 0.15f, 0.25f), blend);
        ambientIntensity = 0.4f - blend * 0.3f;
    }

    // Add moonlight contribution at night
    if (moon.visible && moon.intensity > 0.0f) {
        Vec3 moonAmbient = moon.color * moon.intensity * 0.3f;
        ambientLightColor = ambientLightColor + moonAmbient;
        ambientIntensity += moon.intensity * 0.1f;
    }
}

// ============================================
// Helpers
// ============================================
Vec3 DayNightCycle::LerpColor(const Vec3& a, const Vec3& b, float t) const {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return Vec3(
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t
    );
}

float DayNightCycle::EaseInOut(float t) const {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return t * t * (3.0f - 2.0f * t);
}

// ============================================
// Time Control
// ============================================
void DayNightCycle::SetTime(float t) {
    while (t >= 1.0f) {
        t -= 1.0f;
        dayCount++;
    }
    while (t < 0.0f) {
        t += 1.0f;
        dayCount--;
    }
    timeOfDay = t;

    // Force immediate updates
    UpdateSunPosition();
    UpdateMoonPosition();
    UpdateSkyColors();
    UpdateAmbientLight();
}

float DayNightCycle::GetTimeInHours() const {
    // Map 0.0-1.0 to 0-24 hours
    // 0.0 = 6:00, 0.25 = 12:00, 0.5 = 18:00, 0.75 = 0:00
    float hours = timeOfDay * 24.0f;
    if (hours >= 6.0f) {
        return hours - 6.0f;
    }
    return hours + 18.0f;
}

void DayNightCycle::SetDayLength(float realSeconds) {
    dayLength = realSeconds;
}

// ============================================
// Moon Phase Helpers
// ============================================
float DayNightCycle::GetMoonPhaseBrightness() const {
    switch (currentMoonPhase) {
        case MoonPhase::NewMoon:       return 0.0f;
        case MoonPhase::WaxingCrescent:  return 0.25f;
        case MoonPhase::FirstQuarter:   return 0.5f;
        case MoonPhase::WaxingGibbous:  return 0.75f;
        case MoonPhase::FullMoon:       return 1.0f;
        case MoonPhase::WaningGibbous:  return 0.75f;
        case MoonPhase::LastQuarter:    return 0.5f;
        case MoonPhase::WaningCrescent:  return 0.25f;
    }
    return 0.0f;
}

std::string DayNightCycle::GetMoonPhaseName() const {
    switch (currentMoonPhase) {
        case MoonPhase::NewMoon:        return "New Moon";
        case MoonPhase::WaxingCrescent:  return "Waxing Crescent";
        case MoonPhase::FirstQuarter:   return "First Quarter";
        case MoonPhase::WaxingGibbous:  return "Waxing Gibbous";
        case MoonPhase::FullMoon:       return "Full Moon";
        case MoonPhase::WaningGibbous:  return "Waning Gibbous";
        case MoonPhase::LastQuarter:    return "Last Quarter";
        case MoonPhase::WaningCrescent:  return "Waning Crescent";
    }
    return "Unknown";
}

// ============================================
// Ambient Light
// ============================================
Vec3 DayNightCycle::GetTotalAmbientLight() const {
    return Vec3(
        ambientLightColor.x * ambientIntensity,
        ambientLightColor.y * ambientIntensity,
        ambientLightColor.z * ambientIntensity
    );
}

// ============================================
// Day/Night Queries
// ============================================
bool DayNightCycle::IsDay() const {
    float t = timeOfDay;
    return t > 0.05f && t < 0.55f;
}

bool DayNightCycle::IsNight() const {
    return !IsDay();
}

bool DayNightCycle::IsDawn() const {
    float t = timeOfDay;
    return t >= 0.05f && t <= 0.15f;
}

bool DayNightCycle::IsDusk() const {
    float t = timeOfDay;
    return t >= 0.4f && t <= 0.55f;
}

float DayNightCycle::GetDayNightBlend() const {
    float t = timeOfDay;

    if (t < 0.05f || t > 0.55f) {
        return 0.0f; // Full night
    } else if (t >= 0.15f && t <= 0.4f) {
        return 1.0f; // Full day
    } else if (t < 0.15f) {
        return (t - 0.05f) / 0.1f; // Dawn transition
    } else {
        return 1.0f - (t - 0.4f) / 0.15f; // Dusk transition
    }
}

// ============================================
// Time Strings
// ============================================
std::string DayNightCycle::GetTimeString() const {
    float hours24 = timeOfDay * 24.0f;
    int hours = static_cast<int>(hours24) % 24;
    int minutes = static_cast<int>((hours24 - hours) * 60.0f) % 60;

    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%02d:%02d", hours, minutes);
    return std::string(buffer);
}

std::string DayNightCycle::GetDayTimeString() const {
    return "Day " + std::to_string(dayCount + 1) + ", " + GetTimeString();
}

// ============================================
// Fast-forward
// ============================================
void DayNightCycle::SkipToDawn() {
    SetTime(0.0f);
}

void DayNightCycle::SkipToNoon() {
    SetTime(0.25f);
}

void DayNightCycle::SkipToDusk() {
    SetTime(0.5f);
}

void DayNightCycle::SkipToMidnight() {
    SetTime(0.75f);
}

float DayNightCycle::GetStarVisibility() const {
    float t = timeOfDay;
    // Stars visible at night
    if (t < 0.05f || t > 0.55f) {
        return 1.0f;
    } else if (t < 0.15f) {
        return 1.0f - (t - 0.05f) / 0.1f;
    } else if (t > 0.45f) {
        return (t - 0.45f) / 0.1f;
    }
    return 0.0f;
}

} // namespace vge
