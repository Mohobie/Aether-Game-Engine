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

## Bed/Sleep System

The bed system allows players to sleep through the night and set respawn points.

### Usage

```cpp
vge::BedSystem bedSystem(&world, &dayNightCycle, &lightSystem);

// Place a bed (direction: 0=+X, 1=+Z, 2=-X, 3=-Z)
bedSystem.PlaceBed(x, y, z, direction);

// Try to sleep (returns SleepResult)
vge::SleepResult result = bedSystem.TrySleep(playerPosition);
// Results: Success, NotNight, TooFar, Obstructed, MonstersNearby, AlreadySkipping

// Wake up manually
bedSystem.WakeUp();

// Check state
bool sleeping = bedSystem.IsSleeping();
bool skipping = bedSystem.IsSkippingNight();

// Set callbacks
bedSystem.SetOnSleepStarted([]() { /* ... */ });
bedSystem.SetOnSleepEnded([]() { /* ... */ });
bedSystem.SetOnRespawnPointSet([]() { /* ... */ });
```

### Sleep Requirements
- Must be night time (time > 18:00 or time < 6:00)
- Player must be within 2.5 blocks of the bed
- No monsters within 8 blocks (Manhattan distance)
- Bed must have clear space above it

### Night Skip
- After 3 seconds of lying in bed, time fast-forwards to dawn
- Skip speed: 50x normal time speed
- All players must be in bed to skip (single-player: immediate)

### Files
- `src/game/bed_system.h`
- `src/game/bed_system.cpp`

## Light System Integration

The LightSystem connects to the DayNightCycle for dynamic sky lighting:

```cpp
vge::LightSystem lightSystem(&world);
lightSystem.UpdateSkyLightForTime(dayNightCycle.GetDayNightBlend());

// Check if dark enough for mob spawning
bool dark = lightSystem.IsDarkEnoughForMobs(x, y, z);

// Get total light level (sky + block)
uint8_t light = lightSystem.GetTotalLightLevel(x, y, z);
```

### Files
- `src/rendering/light_system.h`
- `src/rendering/light_system.cpp`

## Mob Spawning Integration

Hostile mobs only spawn at night or in dark areas:

```cpp
// MobSystem uses dayNightBlend for spawn rates
mobSystem.Update(deltaTime, playerPosition, dayNightCycle.GetDayNightBlend());

// MobSpawner checks light levels
MobSpawner spawner(&world, &dayNightCycle, &lightSystem);
spawner.Update(deltaTime, playerPosition);
```

### Spawn Rules
- Hostile mobs: Only spawn when `dayNightBlend < 0.3` (night) AND light level < 7
- Passive mobs: Only spawn during day
- All mobs burn in daylight (hostile mobs take damage when `dayNightBlend > 0.7`)

### Files
- `src/ai/mob_spawner.h`
- `src/ai/mob_spawner.cpp`
- `src/game/mob_system.h`
- `src/game/mob_system.cpp`

## Game Integration

In `my_game.cpp`, the day/night cycle is integrated as follows:

```cpp
// 1. Create systems
vge::DayNightCycle dayNightCycle;
dayNightCycle.SetDayLength(1200.0f); // 20 minutes
renderer.SetDayNightCycle(&dayNightCycle);

vge::LightSystem lightSystem(&world);
lightSystem.UpdateSkyLightForTime(dayNightCycle.GetDayNightBlend());

vge::BedSystem bedSystem(&world, &dayNightCycle, &lightSystem);

// 2. In game loop - update systems
dayNightCycle.Update(deltaTime);
if (!bedSystem.IsSkippingNight()) {
    lightSystem.UpdateSkyLightForTime(dayNightCycle.GetDayNightBlend());
}
bedSystem.Update(deltaTime, playerPosition);
mobSystem.Update(deltaTime, playerPosition, dayNightCycle.GetDayNightBlend());

// 3. Controls
F key - Sleep in bed
E key - Place bed (when selected in hotbar)
0 key - Skip to dawn
8 key - Skip to dusk  
9 key - Skip to midnight
```

## Files
- `src/rendering/sky/day_night_cycle.h`
- `src/rendering/sky/day_night_cycle.cpp`
- `src/rendering/sky/sky_renderer.h`
- `src/rendering/sky/sky_renderer.cpp`
- `src/game/bed_system.h`
- `src/game/bed_system.cpp`
- `src/rendering/light_system.h`
- `src/rendering/light_system.cpp`
