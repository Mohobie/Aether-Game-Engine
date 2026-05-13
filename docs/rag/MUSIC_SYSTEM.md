# Music System

## Quick Reference

```cpp
vge::AudioEngine audio;
audio.Initialize();

// Load music tracks
int ambientDay = audio.LoadSound("music/ambient_day.mp3");
int ambientNight = audio.LoadSound("music/ambient_night.mp3");
int combatMusic = audio.LoadSound("music/combat.mp3");

// Play ambient music (looping)
audio.Play2D(ambientDay, vge::VolumeGroup::Music, 0.5f, 1.0f, true);

// Switch to night music
audio.StopGroup(vge::VolumeGroup::Music);
audio.Play2D(ambientNight, vge::VolumeGroup::Music, 0.5f, 1.0f, true);
```

## Features

### Music Types
| Type | Trigger | Volume |
|------|---------|--------|
| Ambient Day | Daytime | 0.5 |
| Ambient Night | Nighttime | 0.5 |
| Combat | Near hostile mobs | 0.6 |
| Exploration | Walking/exploring | 0.4 |
| Menu | Main menu | 0.7 |

### Transitions
- **Fade in/out:** Smooth volume transitions
- **Crossfade:** Blend between tracks
- **Looping:** Continuous playback
- **Conditional:** Based on game state

## Implementation

```cpp
class MusicManager {
    vge::AudioEngine& audio;
    int currentTrack = -1;
    
public:
    void Update(float deltaTime, bool isNight, bool inCombat) {
        if (inCombat) {
            PlayTrack("combat");
        } else if (isNight) {
            PlayTrack("ambient_night");
        } else {
            PlayTrack("ambient_day");
        }
    }
    
    void PlayTrack(const std::string& track) {
        // Stop current, start new
        audio.StopGroup(vge::VolumeGroup::Music);
        int trackId = audio.LoadSound("music/" + track + ".mp3");
        audio.Play2D(trackId, vge::VolumeGroup::Music, 0.5f, 1.0f, true);
    }
};
```

## Files
- `src/audio/audio_engine.h`
- `src/audio/sound_manager.cpp`
