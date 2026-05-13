# Weather System

## Quick Reference

```cpp
vge::WeatherSystem weather;

// Set weather
weather.SetWeather(vge::WeatherType::Rain, 300.0f); // 5 minutes

// Update in game loop
weather.Update(deltaTime);

// Check current weather
if (weather.IsRaining()) {
    // Spawn rain particles
}

// Get parameters
float fog = weather.GetFogDensity();
Vec3 fogColor = weather.GetFogColor();
```

## Features

### Weather Types
| Type | Clouds | Precipitation | Fog | Thunder |
|------|--------|--------------|-----|---------|
| Clear | Low | None | None | None |
| Rain | High | Rain | Low | None |
| Storm | Max | Heavy | Medium | Yes |
| Snow | High | Snow | Low | None |
| Fog | Medium | None | Max | None |

### Transitions
- **Smooth:** Gradual transition between weather types
- **Duration:** Configurable weather duration
- **Random:** Randomize weather automatically

## Implementation

```cpp
vge::WeatherSystem weather;

// Set random weather
weather.RandomizeWeather();

// Or specific weather
weather.SetWeather(vge::WeatherType::Storm, 120.0f); // 2 minute storm

// Update
weather.Update(deltaTime);

// Query
if (weather.IsStorming()) {
    // Reduce visibility
    renderer.SetFogDensity(weather.GetFogDensity());
}
```

## Files
- `src/core/weather_system.h`
- `src/core/weather_system.cpp`
