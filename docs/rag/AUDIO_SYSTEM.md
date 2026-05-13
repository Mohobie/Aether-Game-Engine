# Audio System

## Quick Reference

```cpp
vge::AudioEngine audio;
audio.Initialize();

// Load sounds
int breakSound = audio.LoadSound("sounds/block_break.wav");
int bgMusic = audio.LoadSound("music/ambient.mp3");

// Play 2D sound
audio.Play2D(breakSound, vge::VolumeGroup::SFX, 0.8f);

// Play 3D positioned sound
audio.Play3D(breakSound, blockPos, vge::VolumeGroup::SFX, 1.0f, 1.0f, false, 0, 50.0f);

// Background music
audio.Play2D(bgMusic, vge::VolumeGroup::Music, 0.5f, 1.0f, true);

// Update listener position
audio.SetListenerPosition(playerPos);
audio.SetListenerOrientation(playerForward, playerUp);

// Update in game loop
audio.Update(deltaTime);
```

## Volume Groups

| Group | Purpose | Default |
|-------|---------|---------|
| Master | Overall volume | 1.0 |
| Music | Background music | 0.7 |
| SFX | Sound effects | 1.0 |
| Voice | Voice/chat | 1.0 |
| Ambient | Environment | 0.8 |
| UI | Interface sounds | 0.9 |

## Sound Loading

```cpp
// Load from file
int sound = audio.LoadSound("sounds/explosion.wav");

// Unload when done
audio.UnloadSound(sound);
```

## Playback Options

```cpp
// 2D sound (UI, music, global effects)
int source = audio.Play2D(
    soundHandle,
    vge::VolumeGroup::SFX,  // Volume group
    1.0f,                   // Volume (0-1)
    1.0f,                   // Pitch (0.5-2)
    false,                  // Loop
    5                       // Priority (higher = more important)
);

// 3D sound (positional)
int source = audio.Play3D(
    soundHandle,
    vge::Vec3(10, 5, 10),   // Position
    vge::VolumeGroup::SFX,
    1.0f,                   // Volume
    1.0f,                   // Pitch
    false,                  // Loop
    5,                      // Priority
    50.0f,                  // Max distance
    1.0f                    // Rolloff
);
```

## Source Control

```cpp
// Stop/pause/resume
audio.Stop(source);
audio.Pause(source);
audio.Resume(source);

// Check if playing
bool playing = audio.IsPlaying(source);

// Update 3D position
audio.SetSourcePosition(source, newPos);

// Adjust volume/pitch
audio.SetSourceVolume(source, 0.5f);
audio.SetSourcePitch(source, 1.2f);
```

## Volume Management

```cpp
// Set group volumes
audio.SetVolume(vge::VolumeGroup::Music, 0.5f);
audio.SetVolume(vge::VolumeGroup::SFX, 0.8f);

// Master volume
audio.SetMasterVolume(0.9f);

// Get current volume
float musicVol = audio.GetVolume(vge::VolumeGroup::Music);
```

## Files
- `src/audio/audio_engine.h`
- `src/audio/audio_engine.cpp`
- `src/audio/audio_system.cpp`
- `src/audio/sound_manager.cpp`
