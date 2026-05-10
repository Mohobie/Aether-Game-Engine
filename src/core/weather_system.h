#pragma once
#include "math/vec3.h"
#include <string>

namespace vge {

// ============================================
// Weather Types
// ============================================
enum class WeatherType {
    Clear,
    Rain,
    Storm,
    Snow,
    Fog
};

// ============================================
// Weather System
// Integrates with DayNightCycle for dynamic
// sky and lighting based on weather
// ============================================
class WeatherSystem {
private:
    WeatherType currentWeather;
    WeatherType targetWeather;
    float transitionProgress;   // 0.0 = current, 1.0 = target
    float transitionDuration;   // Seconds to transition
    float weatherDuration;      // How long current weather lasts
    float timer;                // Time in current state
    
    // Weather parameters (interpolated)
    float cloudDensity;
    float precipitationIntensity;
    float windSpeed;
    float fogDensity;
    Vec3 fogColor;
    float thunderIntensity;
    
    // Internal
    void UpdateParameters(float deltaTime);
    void StartTransition(WeatherType newWeather);
    float GetTargetCloudDensity(WeatherType weather) const;
    float GetTargetPrecipitation(WeatherType weather) const;
    float GetTargetWindSpeed(WeatherType weather) const;
    float GetTargetFogDensity(WeatherType weather) const;
    float GetTargetThunder(WeatherType weather) const;
    Vec3 GetTargetFogColor(WeatherType weather) const;
    
public:
    WeatherSystem();
    
    void Update(float deltaTime);
    
    // Weather control
    void SetWeather(WeatherType weather, float duration);
    WeatherType GetCurrentWeather() const { return currentWeather; }
    WeatherType GetTargetWeather() const { return targetWeather; }
    bool IsTransitioning() const { return transitionProgress < 1.0f; }
    
    // Parameters
    float GetCloudDensity() const { return cloudDensity; }
    float GetPrecipitationIntensity() const { return precipitationIntensity; }
    float GetWindSpeed() const { return windSpeed; }
    float GetFogDensity() const { return fogDensity; }
    Vec3 GetFogColor() const { return fogColor; }
    float GetThunderIntensity() const { return thunderIntensity; }
    
    // Queries
    bool IsRaining() const { return currentWeather == WeatherType::Rain || currentWeather == WeatherType::Storm; }
    bool IsStorming() const { return currentWeather == WeatherType::Storm; }
    bool IsSnowing() const { return currentWeather == WeatherType::Snow; }
    bool IsFoggy() const { return currentWeather == WeatherType::Fog; }
    
    // Random weather
    void RandomizeWeather();
    
    // String helpers
    std::string GetWeatherName() const;
    std::string GetWeatherIcon() const;
};

} // namespace vge
