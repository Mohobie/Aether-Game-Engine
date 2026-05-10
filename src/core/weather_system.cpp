#include "core/weather_system.h"
#include <cstdlib>
#include <ctime>

namespace vge {

WeatherSystem::WeatherSystem()
    : currentWeather(WeatherType::Clear)
    , targetWeather(WeatherType::Clear)
    , transitionProgress(1.0f)
    , transitionDuration(10.0f)
    , weatherDuration(300.0f)
    , timer(0.0f)
    , cloudDensity(0.0f)
    , precipitationIntensity(0.0f)
    , windSpeed(1.0f)
    , fogDensity(0.0f)
    , fogColor(0.7f, 0.8f, 0.9f)
    , thunderIntensity(0.0f)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

void WeatherSystem::Update(float deltaTime) {
    timer += deltaTime;
    
    // Handle weather transition
    if (transitionProgress < 1.0f) {
        transitionProgress += deltaTime / transitionDuration;
        if (transitionProgress > 1.0f) {
            transitionProgress = 1.0f;
            currentWeather = targetWeather;
        }
    }
    
    // Auto-change weather after duration
    if (timer >= weatherDuration && transitionProgress >= 1.0f) {
        RandomizeWeather();
        timer = 0.0f;
    }
    
    UpdateParameters(deltaTime);
}

void WeatherSystem::UpdateParameters(float deltaTime) {
    (void)deltaTime;
    
    // Interpolate parameters toward target
    float t = transitionProgress;
    
    cloudDensity = cloudDensity + (GetTargetCloudDensity(targetWeather) - cloudDensity) * t;
    precipitationIntensity = precipitationIntensity + (GetTargetPrecipitation(targetWeather) - precipitationIntensity) * t;
    windSpeed = windSpeed + (GetTargetWindSpeed(targetWeather) - windSpeed) * t;
    fogDensity = fogDensity + (GetTargetFogDensity(targetWeather) - fogDensity) * t;
    thunderIntensity = thunderIntensity + (GetTargetThunder(targetWeather) - thunderIntensity) * t;
    
    // Fog color interpolation
    Vec3 targetFog = GetTargetFogColor(targetWeather);
    fogColor = Vec3(
        fogColor.x + (targetFog.x - fogColor.x) * t,
        fogColor.y + (targetFog.y - fogColor.y) * t,
        fogColor.z + (targetFog.z - fogColor.z) * t
    );
}

void WeatherSystem::StartTransition(WeatherType newWeather) {
    targetWeather = newWeather;
    transitionProgress = 0.0f;
}

void WeatherSystem::SetWeather(WeatherType weather, float duration) {
    if (weather != currentWeather) {
        StartTransition(weather);
    }
    weatherDuration = duration;
    timer = 0.0f;
}

float WeatherSystem::GetTargetCloudDensity(WeatherType weather) const {
    switch (weather) {
        case WeatherType::Clear: return 0.0f;
        case WeatherType::Rain: return 0.7f;
        case WeatherType::Storm: return 0.9f;
        case WeatherType::Snow: return 0.8f;
        case WeatherType::Fog: return 0.6f;
    }
    return 0.0f;
}

float WeatherSystem::GetTargetPrecipitation(WeatherType weather) const {
    switch (weather) {
        case WeatherType::Clear: return 0.0f;
        case WeatherType::Rain: return 0.6f;
        case WeatherType::Storm: return 1.0f;
        case WeatherType::Snow: return 0.5f;
        case WeatherType::Fog: return 0.0f;
    }
    return 0.0f;
}

float WeatherSystem::GetTargetWindSpeed(WeatherType weather) const {
    switch (weather) {
        case WeatherType::Clear: return 1.0f;
        case WeatherType::Rain: return 3.0f;
        case WeatherType::Storm: return 8.0f;
        case WeatherType::Snow: return 2.0f;
        case WeatherType::Fog: return 0.5f;
    }
    return 1.0f;
}

float WeatherSystem::GetTargetFogDensity(WeatherType weather) const {
    switch (weather) {
        case WeatherType::Clear: return 0.0f;
        case WeatherType::Rain: return 0.02f;
        case WeatherType::Storm: return 0.05f;
        case WeatherType::Snow: return 0.04f;
        case WeatherType::Fog: return 0.1f;
    }
    return 0.0f;
}

float WeatherSystem::GetTargetThunder(WeatherType weather) const {
    switch (weather) {
        case WeatherType::Clear: return 0.0f;
        case WeatherType::Rain: return 0.0f;
        case WeatherType::Storm: return 0.8f;
        case WeatherType::Snow: return 0.0f;
        case WeatherType::Fog: return 0.0f;
    }
    return 0.0f;
}

Vec3 WeatherSystem::GetTargetFogColor(WeatherType weather) const {
    switch (weather) {
        case WeatherType::Clear: return Vec3(0.7f, 0.8f, 0.9f);
        case WeatherType::Rain: return Vec3(0.5f, 0.55f, 0.6f);
        case WeatherType::Storm: return Vec3(0.3f, 0.35f, 0.4f);
        case WeatherType::Snow: return Vec3(0.8f, 0.85f, 0.9f);
        case WeatherType::Fog: return Vec3(0.6f, 0.65f, 0.7f);
    }
    return Vec3(0.7f, 0.8f, 0.9f);
}

void WeatherSystem::RandomizeWeather() {
    float r = static_cast<float>(std::rand()) / RAND_MAX;
    
    WeatherType newWeather;
    float duration;
    
    if (r < 0.4f) {
        newWeather = WeatherType::Clear;
        duration = 300.0f + static_cast<float>(std::rand() % 600); // 5-15 min
    } else if (r < 0.6f) {
        newWeather = WeatherType::Rain;
        duration = 180.0f + static_cast<float>(std::rand() % 300); // 3-8 min
    } else if (r < 0.75f) {
        newWeather = WeatherType::Storm;
        duration = 120.0f + static_cast<float>(std::rand() % 240); // 2-6 min
    } else if (r < 0.9f) {
        newWeather = WeatherType::Snow;
        duration = 180.0f + static_cast<float>(std::rand() % 300); // 3-8 min
    } else {
        newWeather = WeatherType::Fog;
        duration = 120.0f + static_cast<float>(std::rand() % 180); // 2-5 min
    }
    
    SetWeather(newWeather, duration);
}

std::string WeatherSystem::GetWeatherName() const {
    switch (currentWeather) {
        case WeatherType::Clear: return "Clear";
        case WeatherType::Rain: return "Rain";
        case WeatherType::Storm: return "Storm";
        case WeatherType::Snow: return "Snow";
        case WeatherType::Fog: return "Fog";
    }
    return "Unknown";
}

std::string WeatherSystem::GetWeatherIcon() const {
    switch (currentWeather) {
        case WeatherType::Clear: return "☀";
        case WeatherType::Rain: return "🌧";
        case WeatherType::Storm: return "⛈";
        case WeatherType::Snow: return "🌨";
        case WeatherType::Fog: return "🌫";
    }
    return "?";
}

} // namespace vge
