#pragma once
#include "math/vec3.h"
#include <string>

namespace vge {

enum class WeatherType {
    Clear,
    Rain,
    Storm,
    Snow
};

class TimeSystem {
private:
    int timeOfDay;      // 0-24000
    int dayLength;      // Default 24000
    int dayCount;       // Days passed
    WeatherType weather;
    float weatherDuration;
    
    void ChangeWeather();
    
public:
    TimeSystem();
    
    void Update(float deltaTime);
    
    // Sun
    float GetSunIntensity() const;
    Vec3 GetSunColor() const;
    
    // Time
    int GetTimeOfDay() const { return timeOfDay; }
    int GetDayCount() const { return dayCount; }
    std::string GetTimeString() const;
    void SetTime(int time);
    
    // Day/Night
    bool IsDay() const;
    bool IsNight() const;
    
    // Weather
    WeatherType GetWeather() const { return weather; }
    float GetWeatherDuration() const { return weatherDuration; }
    void SetWeather(WeatherType type, float duration);
};

} // namespace vge