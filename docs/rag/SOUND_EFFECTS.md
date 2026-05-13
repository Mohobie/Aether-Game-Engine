# Sound Effects

## Quick Reference

```cpp
vge::AudioEngine audio;
audio.Initialize();

// Load sounds
int breakSound = audio.LoadSound("sounds/stone_break.wav");
int placeSound = audio.LoadSound("sounds/stone_place.wav");
int footstepSound = audio.LoadSound("sounds/grass_footstep.wav");

// Play sounds
audio.Play2D(breakSound, vge::VolumeGroup::SFX, 0.8f);
audio.Play2D(placeSound, vge::VolumeGroup::SFX, 0.6f);
audio.Play2D(footstepSound, vge::VolumeGroup::SFX, 0.4f);
```

## Features

### Block Sounds
| Sound | Trigger | Volume |
|-------|---------|--------|
| Break | Block destroyed | 0.8 |
| Place | Block placed | 0.6 |
| Step | Walking on block | 0.4 |

### Mob Sounds
| Sound | Trigger | Volume |
|-------|---------|--------|
| Ambient | Random idle | 0.5 |
| Hurt | Taking damage | 0.7 |
| Death | Dying | 0.8 |
| Attack | Attacking | 0.6 |

### Tool Sounds
| Sound | Trigger | Volume |
|-------|---------|--------|
| Swing | Tool swing | 0.5 |
| Hit | Hitting block/entity | 0.6 |
| Break | Tool breaking | 0.9 |

## Implementation

```cpp
// Initialize audio
vge::AudioEngine audio;
audio.Initialize();

// Load sound effects
std::unordered_map<std::string, int> sounds;
sounds["stone_break"] = audio.LoadSound("sounds/stone_break.wav");
sounds["grass_step"] = audio.LoadSound("sounds/grass_step.wav");
sounds["zombie_ambient"] = audio.LoadSound("sounds/zombie_ambient.wav");

// Play on events
void OnBlockBreak(const std::string& blockType) {
    std::string soundName = blockType + "_break";
    if (sounds.count(soundName)) {
        audio.Play2D(sounds[soundName], vge::VolumeGroup::SFX, 0.8f);
    }
}

void OnFootstep(const std::string& groundType) {
    std::string soundName = groundType + "_step";
    if (sounds.count(soundName)) {
        audio.Play2D(sounds[soundName], vge::VolumeGroup::SFX, 0.4f);
    }
}
```

## Files
- `src/audio/audio_engine.h`
- `src/audio/sound_manager.cpp`
