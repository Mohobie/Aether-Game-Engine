# Audio Event System

The engine provides a decoupled, game-agnostic audio event system. Games register their own sound files for each event type.

## Core Principle

**No hardcoded sounds.** The engine defines event types, but games provide the actual sound files.

## Audio Event Types

### Movement
- `Footstep` - Walking/running steps
- `Jump` - Jumping
- `Land` - Landing from jump/fall
- `Swim` - Swimming in water
- `Climb` - Climbing ladders/walls

### Combat
- `Attack` - Generic attack
- `AttackSwing` - Weapon swing (miss)
- `AttackHit` - Weapon hit target
- `Hurt` - Taking damage
- `Die` - Death
- `Block` - Blocking attack
- `Parry` - Parrying attack

### World Interaction
- `Interact` - Interacting with objects
- `Use` - Using items
- `Pickup` - Picking up items
- `Drop` - Dropping items
- `Open` - Opening doors/chests
- `Close` - Closing doors/chests
- `Craft` - Crafting items

### Explosions / Effects
- `Explosion` - Explosions
- `Fire` - Fire burning
- `Splash` - Water splash
- `WeatherRain` - Rain
- `WeatherThunder` - Thunder

### UI
- `UIClick` - Button click
- `UIHover` - Hover over element
- `UIConfirm` - Confirm action
- `UICancel` - Cancel action
- `UIError` - Error sound
- `UISlider` - Slider movement

### Ambient (loops / zones)
- `AmbientForest` - Forest ambience
- `AmbientCave` - Cave ambience
- `AmbientOcean` - Ocean ambience
- `AmbientWind` - Wind
- `AmbientCity` - City ambience
- `AmbientDungeon` - Dungeon ambience

### Music Triggers
- `MusicExplore` - Exploration music
- `MusicCombat` - Combat music
- `MusicBoss` - Boss fight music
- `MusicVictory` - Victory music
- `MusicDefeat` - Defeat music
- `MusicMenu` - Menu music

### Custom
- `Custom` - Game-specific events

## Sound Packs

### Creating a Sound Pack

```cpp
#include "audio/audio_events.h"

// Create a sound pack
vge::SoundPack pack("my_game_sounds");

// Register sounds for events
pack.RegisterEvent(vge::AudioEventType::Footstep, "sfx/footstep.wav", 0.4f);
pack.RegisterEvent(vge::AudioEventType::Jump, "sfx/jump.wav", 0.5f);
pack.RegisterEvent(vge::AudioEventType::AttackHit, "sfx/attack_hit.wav", 0.7f);
pack.RegisterEvent(vge::AudioEventType::Hurt, "sfx/hurt.wav", 0.6f);

// Register multiple sounds for randomization
vge::EventSoundSet footstepSet(vge::AudioEventType::Footstep);
footstepSet.AddSound("sfx/footstep1.wav", 0.4f, 0.9f, 1.1f);
footstepSet.AddSound("sfx/footstep2.wav", 0.4f, 0.9f, 1.1f);
footstepSet.AddSound("sfx/footstep3.wav", 0.4f, 0.9f, 1.1f);
footstepSet.randomizePitch = true;
footstepSet.pitchVariation = 0.1f;
pack.RegisterEvent(vge::AudioEventType::Footstep, footstepSet);
```

### Loading from JSON

```json
{
  "name": "my_game_sounds",
  "events": {
    "footstep": {
      "default_volume": 0.4,
      "randomize_pitch": true,
      "pitch_variation": 0.1,
      "sounds": [
        {"file": "sfx/footstep1.wav", "volume": 0.4, "pitch_min": 0.9, "pitch_max": 1.1},
        {"file": "sfx/footstep2.wav", "volume": 0.4, "pitch_min": 0.9, "pitch_max": 1.1},
        {"file": "sfx/footstep3.wav", "volume": 0.4, "pitch_min": 0.9, "pitch_max": 1.1}
      ]
    },
    "jump": {
      "sounds": [
        {"file": "sfx/jump.wav", "volume": 0.5}
      ]
    },
    "attack_hit": {
      "sounds": [
        {"file": "sfx/attack_hit.wav", "volume": 0.7}
      ]
    }
  },
  "custom_events": {
    "spaceship_engine": {
      "sounds": [
        {"file": "sfx/engine_loop.wav", "volume": 0.6, "loop": true}
      ]
    }
  }
}
```

```cpp
// Load from file
vge::SoundPack pack("my_game_sounds");
pack.LoadFromFile("assets/audio/packs/my_game_sounds.json");

// Save to file
pack.SaveToFile("assets/audio/packs/my_game_sounds.json");
```

## Using Sound Packs

```cpp
// Get sounds for an event
const vge::EventSoundSet* sounds = pack.GetEventSounds(vge::AudioEventType::Footstep);
if (sounds) {
    const vge::SoundEntry* sound = sounds->PickRandom();
    if (sound) {
        audioEngine.Play(sound->filePath, sound->volume);
    }
}

// Check if event is registered
if (pack.HasEvent(vge::AudioEventType::Jump)) {
    // Play jump sound
}

// Get all registered events
auto events = pack.GetRegisteredEvents();
```

## Custom Events

```cpp
// Register custom events for game-specific sounds
pack.RegisterCustomEvent("spaceship_engine", "sfx/engine_loop.wav", 0.6f);
pack.RegisterCustomEvent("laser_shot", "sfx/laser.wav", 0.8f);

// Use custom events
const vge::EventSoundSet* engineSounds = pack.GetCustomEventSounds("spaceship_engine");
if (engineSounds) {
    // Play engine sound
}
```

## Default Sound Pack

The engine provides a default sound pack with sensible fallback entries:

```cpp
vge::SoundPack defaultPack = vge::CreateDefaultSoundPack();
```

This includes basic sounds for:
- Movement (footstep, jump, land)
- Combat (attack_swing, attack_hit, hurt, die, explosion)
- Interaction (interact, pickup, open, close)
- UI (click, hover, confirm, cancel, error)
- Ambient (forest, cave, ocean, wind)

## Sound Entry Properties

```cpp
struct SoundEntry {
    std::string filePath;       // Relative to assets/audio/
    float volume = 1.0f;        // Per-sound volume multiplier
    float pitchMin = 1.0f;      // Random pitch range
    float pitchMax = 1.0f;
    float probability = 1.0f;   // For weighted random selection
    int priority = 0;           // Higher = more important
};
```

## Event Sound Set Properties

```cpp
struct EventSoundSet {
    AudioEventType eventType;
    std::vector<SoundEntry> sounds;
    float defaultVolume = 1.0f;
    bool randomizePitch = false;
    float pitchVariation = 0.1f;  // +/- variation when randomizePitch is true
};
```

## Best Practices

1. **Use sound packs** - Organize sounds by game/theme
2. **Randomize footsteps** - Use multiple sounds with pitch variation
3. **Set appropriate volumes** - UI: 0.3-0.5, SFX: 0.5-0.8, Ambient: 0.2-0.4
4. **Use custom events** - For game-specific sounds not covered by standard events
5. **Load asynchronously** - Sound packs can be large, load in background
6. **Provide fallbacks** - Always have a default sound pack as backup
