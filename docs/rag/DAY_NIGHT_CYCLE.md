# Day/Night Cycle

## Quick Reference

```cpp
vge::DayNightCycle cycle;
cycle.SetDayLength(1200.0f); // 20 minutes per day

// In game loop
cycle.Update(deltaTime);

// Get sky color for rendering
Vec3 skyColor = cycle.GetSkyTopColor();
renderer.SetClearColor(skyColor.x, skyColor.y, skyColor.z, 1.0f);

// Check time
if (cycle.IsDay()) { /* ... */ }
if (cycle.IsNight()) { /* ... */ }
```

## Features

### Time System
- **Day Length:** Configurable (default 20 minutes real-time)
- **Time Scale:** Speed up/slow down
- **Day Count:** Tracks total days passed

### Sky Colors
| Time | Top Color | Horizon Color |
|------|-----------|---------------|
| Night | Dark blue/black | Deep blue |
| Dawn | Blue → Orange | Orange/pink |
| Day | Bright blue | Light blue |
| Dusk | Blue → Purple | Orange/red |

### Celestial Bodies
- **Sun:** Arcs across sky, intensity based on height
- **Moon:** Opposite to sun, 8 phases
- **Stars:** 500 stars with twinkling, visible at night

### Ambient Light
- Day: Bright white (intensity 0.6)
- Night: Dim blue (intensity 0.1)
- Dawn/Dusk: Warm orange transition

## Controls

```cpp
cycle.SetTime(6.0f);   // Set to 6:00 AM
cycle.SkipToDawn();    // Jump to sunrise
cycle.SkipToNoon();    // Jump to midday
cycle.SkipToDusk();    // Jump to sunset
cycle.SkipToMidnight();// Jump to midnight

cycle.Pause();         // Stop time
cycle.Resume();        // Resume time
cycle.SetTimeScale(2.0f); // 2x speed
```

## Queries

```cpp
float time = cycle.GetTimeOfDay();      // 0-24 hours
int ticks = cycle.GetTimeOfDayTicks();  // 0-24000 (Minecraft style)
bool isDay = cycle.IsDay();
bool isNight = cycle.IsNight();
bool isDawn = cycle.IsDawn();   // 6:00-9:00
bool isDusk = cycle.IsDusk();   // 18:00-21:00
float blend = cycle.GetDayNightBlend(); // 0.0=night, 1.0=day

// Sun info
Vec3 sunDir = cycle.GetSunDirection();
float sunIntensity = cycle.GetSunIntensity();
bool sunVisible = cycle.IsSunVisible();

// Moon info
MoonPhase phase = cycle.GetMoonPhase();
std::string phaseName = cycle.GetMoonPhaseName();
float moonBrightness = cycle.GetMoonPhaseBrightness();
```

## Files
- `src/rendering/sky/day_night_cycle.h`
- `src/rendering/sky/day_night_cycle.cpp`
- `src/rendering/sky/sky_renderer.h`
- `src/rendering/sky/sky_renderer.cpp`
