#include "core/time_system.h"
#include "voxel/world.h"
#include "voxel/biome.h"
#include <cmath>

namespace vge {

TimeSystem::TimeSystem() 
    : timeOfDay(6000), // Start at noon
      dayLength(24000), // 20 minutes per day (like Minecraft)
      dayCount(0),
      weather(WeatherType::Clear),
      weatherDuration(0) {}

void TimeSystem::Update(float deltaTime) {
    // Advance time
    timeOfDay += deltaTime * 20.0f; // 20 ticks per second
    
    if (timeOfDay >= dayLength) {
        timeOfDay -= dayLength;
        dayCount++;
        
        // Random weather change at dawn
        if (dayCount % 3 == 0) {
            ChangeWeather();
        }
    }
    
    // Update weather duration
    if (weatherDuration > 0) {
        weatherDuration -= deltaTime;
        if (weatherDuration <= 0) {
            weather = WeatherType::Clear;
        }
    }
}

float TimeSystem::GetSunIntensity() const {
    // Calculate sun intensity based on time of day
    // Noon = 1.0, midnight = 0.0
    float normalizedTime = timeOfDay / dayLength;
    float sunHeight = sinf(normalizedTime * 3.14159f * 2.0f - 3.14159f / 2.0f);
    
    if (sunHeight < 0) sunHeight = 0;
    
    // Weather affects sun
    switch (weather) {
        case WeatherType::Rain:
            sunHeight *= 0.3f;
            break;
        case WeatherType::Storm:
            sunHeight *= 0.1f;
            break;
        case WeatherType::Snow:
            sunHeight *= 0.5f;
            break;
        default:
            break;
    }
    
    return sunHeight;
}

Vec3 TimeSystem::GetSunColor() const {
    float normalizedTime = timeOfDay / dayLength;
    
    // Dawn (orange)
    if (normalizedTime < 0.1f) {
        return Vec3(1.0f, 0.6f, 0.3f);
    }
    // Day (white/yellow)
    else if (normalizedTime < 0.45f) {
        return Vec3(1.0f, 1.0f, 0.9f);
    }
    // Dusk (orange/red)
    else if (normalizedTime < 0.55f) {
        return Vec3(1.0f, 0.5f, 0.2f);
    }
    // Night (blue)
    else {
        return Vec3(0.2f, 0.3f, 0.6f);
    }
}

std::string TimeSystem::GetTimeString() const {
    int hours = (int)(timeOfDay / 1000);
    int minutes = (int)((timeOfDay - hours * 1000) * 60 / 1000);
    
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%02d:%02d", hours, minutes);
    return std::string(buffer);
}

bool TimeSystem::IsDay() const {
    float normalizedTime = timeOfDay / dayLength;
    return normalizedTime > 0.1f && normalizedTime < 0.9f;
}

bool TimeSystem::IsNight() const {
    return !IsDay();
}

void TimeSystem::SetTime(int time) {
    timeOfDay = time % dayLength;
}

void TimeSystem::SetWeather(WeatherType type, float transitionDuration) {
    weather = type;
    weatherDuration = transitionDuration;
}

void TimeSystem::ChangeWeather() {
    // Random weather
    float r = (float)rand() / RAND_MAX;
    
    if (r < 0.5f) {
        weather = WeatherType::Clear;
        weatherDuration = 0;
    } else if (r < 0.7f) {
        weather = WeatherType::Rain;
        weatherDuration = 300.0f + (r * 600.0f); // 5-15 minutes
    } else if (r < 0.85f) {
        weather = WeatherType::Storm;
        weatherDuration = 120.0f + (r * 300.0f); // 2-7 minutes
    } else {
        weather = WeatherType::Snow;
        weatherDuration = 300.0f + (r * 600.0f); // 5-15 minutes
    }
}

} // namespace vge