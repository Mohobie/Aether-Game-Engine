#include "rendering/sky/day_night_cycle.h"
#include <cmath>
#include <cstdlib>
#include <ctime>

namespace vge {

// ============================================
// Constructor
// ============================================
DayNightCycle::DayNightCycle()
    : timeOfDay(6.0f)       // Start at dawn
    , dayLength(1200.0f)    // 20 minutes per day (like Minecraft)
    , dayCount(0)
    , timeScale(1.0f)
    , paused(false)
    , starTwinkleSpeed(2.0f)
    , currentMoonPhase(MoonPhase::FullMoon)
    , moonPhaseProgress(0.0f)
    , skyTransitionSpeed(2.0f)
    , ambientIntensity(0.5f)
{
    // Seed random for star generation
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    
    GenerateStars();
    
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
    
    // Advance time
    float timeAdvance = (deltaTime / dayLength) * 24.0f * timeScale;
    timeOfDay += timeAdvance;
    
    // Handle day rollover
    while (timeOfDay >= 24.0f) {
        timeOfDay -= 24.0f;
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
    
    // Update star twinkling
    for (auto& star : stars) {
        star.twinklePhase += deltaTime * starTwinkleSpeed * (0.5f + star.brightness * 0.5f);
        if (star.twinklePhase > 6.28318f) {
            star.twinklePhase -= 6.28318f;
        }
    }
}

// ============================================
// Sun Position
// ============================================
void DayNightCycle::UpdateSunPosition() {
    float normalizedTime = GetNormalizedTime(); // 0.0 = midnight, 0.5 = noon
    
    // Sun moves in an arc: east -> up -> west
    // Angle from -PI/2 (sunrise) to PI/2 (sunset) to 3PI/2 (next sunrise)
    float sunAngle = (normalizedTime - 0.25f) * 6.28318f; // -PI/2 at 6:00, PI/2 at 18:00
    
    // Position on a hemisphere
    sun.position.x = std::cos(sunAngle);          // East-West
    sun.position.y = std::sin(sunAngle);          // Up-Down
    sun.position.z = 0.3f * std::cos(sunAngle * 0.5f); // Slight north-south variation
    
    // Normalize
    float len = sun.position.length();
    if (len > 0.0f) {
        sun.position = sun.position / len;
    }
    
    // Visibility and intensity
    sun.visible = sun.position.y > -0.1f; // Visible until slightly below horizon
    
    // Intensity based on height
    float heightFactor = std::max(0.0f, sun.position.y);
    sun.intensity = heightFactor * heightFactor; // Quadratic falloff for smoother transition
    
    // Sun color based on time
    if (normalizedTime < 0.2f || normalizedTime > 0.8f) {
        // Night - no sun
        sun.color = Vec3(0.0f, 0.0f, 0.0f);
        sun.intensity = 0.0f;
    } else if (normalizedTime < 0.25f) {
        // Dawn - warm orange
        float t = (normalizedTime - 0.2f) / 0.05f;
        sun.color = LerpColor(Vec3(1.0f, 0.4f, 0.1f), Vec3(1.0f, 0.9f, 0.7f), t);
    } else if (normalizedTime < 0.3f) {
        // Morning - yellow-white
        float t = (normalizedTime - 0.25f) / 0.05f;
        sun.color = LerpColor(Vec3(1.0f, 0.9f, 0.7f), Vec3(1.0f, 1.0f, 0.95f), t);
    } else if (normalizedTime < 0.7f) {
        // Day - bright white
        sun.color = Vec3(1.0f, 1.0f, 0.98f);
    } else if (normalizedTime < 0.75f) {
        // Late afternoon - warm
        float t = (normalizedTime - 0.7f) / 0.05f;
        sun.color = LerpColor(Vec3(1.0f, 1.0f, 0.98f), Vec3(1.0f, 0.85f, 0.6f), t);
    } else if (normalizedTime < 0.8f) {
        // Dusk - orange/red
        float t = (normalizedTime - 0.75f) / 0.05f;
        sun.color = LerpColor(Vec3(1.0f, 0.85f, 0.6f), Vec3(1.0f, 0.4f, 0.1f), t);
    }
    
    sun.size = 1.0f;
}

// ============================================
// Moon Position
// ============================================
void DayNightCycle::UpdateMoonPosition() {
    float normalizedTime = GetNormalizedTime();
    
    // Moon is opposite to sun (roughly)
    float moonAngle = (normalizedTime + 0.25f) * 6.28318f;
    
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
    moon.intensity = phaseBrightness * heightFactor * 0.3f; // Moon is dimmer than sun
    
    // Moon color - slightly blue-white
    moon.color = Vec3(0.8f, 0.85f, 1.0f) * phaseBrightness;
    
    moon.size = 0.8f; // Slightly smaller than sun
}

// ============================================
// Moon Phase
// ============================================
void DayNightCycle::UpdateMoonPhase() {
    // Moon phase progresses over 8 days (simplified)
    float daysSincePhaseChange = static_cast<float>(dayCount % 3) + (timeOfDay / 24.0f);
    moonPhaseProgress = daysSincePhaseChange / 3.0f;
    if (moonPhaseProgress > 1.0f) moonPhaseProgress = 1.0f;
}

// ============================================
// Sky Colors
// ============================================
SkyColorSet DayNightCycle::GetSkyColorsForTime(float normalizedTime) const {
    SkyColorSet colors;
    
    if (normalizedTime < 0.2f || normalizedTime > 0.8f) {
        // Night - dark blue/black
        colors.topColor = Vec3(0.02f, 0.03f, 0.08f);
        colors.horizonColor = Vec3(0.05f, 0.06f, 0.12f);
        colors.bottomColor = Vec3(0.01f, 0.02f, 0.05f);
        colors.fogColor = Vec3(0.05f, 0.06f, 0.12f);
        colors.fogDensity = 0.02f;
    } else if (normalizedTime < 0.25f) {
        // Dawn
        float t = EaseInOut((normalizedTime - 0.2f) / 0.05f);
        colors.topColor = LerpColor(Vec3(0.02f, 0.03f, 0.08f), Vec3(0.4f, 0.5f, 0.7f), t);
        colors.horizonColor = LerpColor(Vec3(0.05f, 0.06f, 0.12f), Vec3(1.0f, 0.5f, 0.2f), t);
        colors.bottomColor = LerpColor(Vec3(0.01f, 0.02f, 0.05f), Vec3(0.3f, 0.3f, 0.4f), t);
        colors.fogColor = colors.horizonColor;
        colors.fogDensity = 0.01f + t * 0.01f;
    } else if (normalizedTime < 0.3f) {
        // Morning
        float t = EaseInOut((normalizedTime - 0.25f) / 0.05f);
        colors.topColor = LerpColor(Vec3(0.4f, 0.5f, 0.7f), Vec3(0.5f, 0.7f, 1.0f), t);
        colors.horizonColor = LerpColor(Vec3(1.0f, 0.5f, 0.2f), Vec3(0.8f, 0.9f, 1.0f), t);
        colors.bottomColor = LerpColor(Vec3(0.3f, 0.3f, 0.4f), Vec3(0.5f, 0.6f, 0.7f), t);
        colors.fogColor = colors.horizonColor;
        colors.fogDensity = 0.01f;
    } else if (normalizedTime < 0.7f) {
        // Day
        colors.topColor = Vec3(0.5f, 0.7f, 1.0f);
        colors.horizonColor = Vec3(0.8f, 0.9f, 1.0f);
        colors.bottomColor = Vec3(0.5f, 0.6f, 0.7f);
        colors.fogColor = Vec3(0.8f, 0.9f, 1.0f);
        colors.fogDensity = 0.005f;
    } else if (normalizedTime < 0.75f) {
        // Late afternoon
        float t = EaseInOut((normalizedTime - 0.7f) / 0.05f);
        colors.topColor = LerpColor(Vec3(0.5f, 0.7f, 1.0f), Vec3(0.4f, 0.5f, 0.7f), t);
        colors.horizonColor = LerpColor(Vec3(0.8f, 0.9f, 1.0f), Vec3(1.0f, 0.6f, 0.3f), t);
        colors.bottomColor = LerpColor(Vec3(0.5f, 0.6f, 0.7f), Vec3(0.4f, 0.4f, 0.5f), t);
        colors.fogColor = colors.horizonColor;
        colors.fogDensity = 0.01f + t * 0.01f;
    } else if (normalizedTime < 0.8f) {
        // Dusk
        float t = EaseInOut((normalizedTime - 0.75f) / 0.05f);
        colors.topColor = LerpColor(Vec3(0.4f, 0.5f, 0.7f), Vec3(0.02f, 0.03f, 0.08f), t);
        colors.horizonColor = LerpColor(Vec3(1.0f, 0.6f, 0.3f), Vec3(0.1f, 0.1f, 0.2f), t);
        colors.bottomColor = LerpColor(Vec3(0.4f, 0.4f, 0.5f), Vec3(0.01f, 0.02f, 0.05f), t);
        colors.fogColor = colors.horizonColor;
        colors.fogDensity = 0.02f + t * 0.01f;
    }
    
    return colors;
}

void DayNightCycle::UpdateSkyColors() {
    float normalizedTime = GetNormalizedTime();
    targetSky = GetSkyColorsForTime(normalizedTime);
    
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
    float normalizedTime = GetNormalizedTime();
    
    // Base ambient changes with time of day
    if (normalizedTime < 0.2f || normalizedTime > 0.8f) {
        // Night - very dim, blue-tinted
        ambientLightColor = Vec3(0.1f, 0.15f, 0.25f);
        ambientIntensity = 0.1f;
    } else if (normalizedTime < 0.25f) {
        // Dawn - increasing warm light
        float t = (normalizedTime - 0.2f) / 0.05f;
        ambientLightColor = LerpColor(Vec3(0.1f, 0.15f, 0.25f), Vec3(0.5f, 0.45f, 0.35f), t);
        ambientIntensity = 0.1f + t * 0.3f;
    } else if (normalizedTime < 0.3f) {
        // Morning - brightening
        float t = (normalizedTime - 0.25f) / 0.05f;
        ambientLightColor = LerpColor(Vec3(0.5f, 0.45f, 0.35f), Vec3(0.7f, 0.7f, 0.65f), t);
        ambientIntensity = 0.4f + t * 0.2f;
    } else if (normalizedTime < 0.7f) {
        // Day - full ambient
        ambientLightColor = Vec3(0.7f, 0.7f, 0.65f);
        ambientIntensity = 0.6f;
    } else if (normalizedTime < 0.75f) {
        // Late afternoon - dimming
        float t = (normalizedTime - 0.7f) / 0.05f;
        ambientLightColor = LerpColor(Vec3(0.7f, 0.7f, 0.65f), Vec3(0.5f, 0.45f, 0.35f), t);
        ambientIntensity = 0.6f - t * 0.2f;
    } else if (normalizedTime < 0.8f) {
        // Dusk - fading to night
        float t = (normalizedTime - 0.75f) / 0.05f;
        ambientLightColor = LerpColor(Vec3(0.5f, 0.45f, 0.35f), Vec3(0.1f, 0.15f, 0.25f), t);
        ambientIntensity = 0.4f - t * 0.3f;
    }
    
    // Add moonlight contribution at night
    if (moon.visible && moon.intensity > 0.0f) {
        Vec3 moonAmbient = moon.color * moon.intensity * 0.3f;
        ambientLightColor = ambientLightColor + moonAmbient;
        ambientIntensity += moon.intensity * 0.1f;
    }
}

// ============================================
// Star Generation
// ============================================
void DayNightCycle::GenerateStars() {
    stars.clear();
    stars.reserve(NUM_STARS);
    
    for (int i = 0; i < NUM_STARS; ++i) {
        Star star;
        
        // Random direction on upper hemisphere
        float theta = static_cast<float>(std::rand()) / RAND_MAX * 6.28318f; // 0 to 2PI
        float phi = static_cast<float>(std::rand()) / RAND_MAX * 1.5708f;     // 0 to PI/2 (upper half)
        
        star.direction.x = std::sin(phi) * std::cos(theta);
        star.direction.y = std::cos(phi);
        star.direction.z = std::sin(phi) * std::sin(theta);
        
        // Random brightness with some very bright stars
        float r = static_cast<float>(std::rand()) / RAND_MAX;
        if (r < 0.1f) {
            star.brightness = 0.8f + r * 2.0f; // Bright stars
        } else {
            star.brightness = 0.2f + r * 0.5f; // Dimmer stars
        }
        
        star.twinklePhase = static_cast<float>(std::rand()) / RAND_MAX * 6.28318f;
        star.colorIndex = static_cast<uint8_t>(std::rand() % 4);
        
        stars.push_back(star);
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
void DayNightCycle::SetTime(float hour) {
    while (hour >= 24.0f) {
        hour -= 24.0f;
        dayCount++;
    }
    while (hour < 0.0f) {
        hour += 24.0f;
        dayCount--;
    }
    timeOfDay = hour;
    
    // Force immediate updates
    UpdateSunPosition();
    UpdateMoonPosition();
    UpdateSkyColors();
    UpdateAmbientLight();
}

void DayNightCycle::SetTimeOfDayTicks(int ticks) {
    SetTime(static_cast<float>(ticks) / 1000.0f);
}

int DayNightCycle::GetTimeOfDayTicks() const {
    return static_cast<int>(timeOfDay * 1000.0f);
}

float DayNightCycle::GetNormalizedTime() const {
    return timeOfDay / 24.0f;
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
// Star Visibility
// ============================================
float DayNightCycle::GetStarVisibility() const {
    float normalizedTime = GetNormalizedTime();
    
    // Stars visible at night
    if (normalizedTime < 0.2f || normalizedTime > 0.8f) {
        return 1.0f;
    } else if (normalizedTime < 0.25f) {
        return 1.0f - (normalizedTime - 0.2f) / 0.05f;
    } else if (normalizedTime > 0.75f) {
        return (normalizedTime - 0.75f) / 0.05f;
    }
    
    return 0.0f;
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
    float normalizedTime = GetNormalizedTime();
    return normalizedTime > 0.2f && normalizedTime < 0.8f;
}

bool DayNightCycle::IsNight() const {
    return !IsDay();
}

bool DayNightCycle::IsDawn() const {
    float normalizedTime = GetNormalizedTime();
    return normalizedTime >= 0.2f && normalizedTime <= 0.3f;
}

bool DayNightCycle::IsDusk() const {
    float normalizedTime = GetNormalizedTime();
    return normalizedTime >= 0.7f && normalizedTime <= 0.8f;
}

float DayNightCycle::GetDayNightBlend() const {
    float normalizedTime = GetNormalizedTime();
    
    if (normalizedTime < 0.2f || normalizedTime > 0.8f) {
        return 0.0f; // Full night
    } else if (normalizedTime >= 0.3f && normalizedTime <= 0.7f) {
        return 1.0f; // Full day
    } else if (normalizedTime < 0.3f) {
        return (normalizedTime - 0.2f) / 0.1f; // Dawn transition
    } else {
        return 1.0f - (normalizedTime - 0.7f) / 0.1f; // Dusk transition
    }
}

// ============================================
// Time Strings
// ============================================
std::string DayNightCycle::GetTimeString() const {
    int hours = static_cast<int>(timeOfDay);
    int minutes = static_cast<int>((timeOfDay - hours) * 60.0f);
    
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
    SetTime(6.0f);
}

void DayNightCycle::SkipToDusk() {
    SetTime(18.0f);
}

void DayNightCycle::SkipToNoon() {
    SetTime(12.0f);
}

void DayNightCycle::SkipToMidnight() {
    SetTime(0.0f);
}

} // namespace vge
