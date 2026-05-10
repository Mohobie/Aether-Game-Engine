# Audio Module API

**Files:** `audio/audio_engine.h`, `audio/sound.h`, `audio/music.h`, `audio/audio_listener.h`, `audio/audio_source.h`

---

## `audio/audio_engine.h`

```cpp
namespace vge {
```

### `class AudioEngine`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `GetInstance` | `AudioEngine&` | `void` | Singleton |
| `Initialize` | `bool` | `void` | Initialize audio |
| `Shutdown` | `void` | `void` | Shutdown audio |
| `SetMasterVolume` | `void` | `float volume` | Set master volume (0-1) |
| `GetMasterVolume` | `float` | `void` | Get master volume |
| `SetSFXVolume` | `void` | `float volume` | Set SFX volume |
| `GetSFXVolume` | `float` | `void` | Get SFX volume |
| `SetMusicVolume` | `void` | `float volume` | Set music volume |
| `GetMusicVolume` | `float` | `void` | Get music volume |
| `Mute` | `void` | `void` | Mute all |
| `Unmute` | `void` | `void` | Unmute |
| `IsMuted` | `bool` | `void` | Check muted |
| `PlaySound` | `void` | `const std::string& path, float volume = 1.0f, bool loop = false` | Play sound |
| `PlayMusic` | `void` | `const std::string& path, float volume = 1.0f, bool loop = true` | Play music |
| `StopAll` | `void` | `void` | Stop all sounds |
| `PauseAll` | `void` | `void` | Pause all |
| `ResumeAll` | `void` | `void` | Resume all |
| `GetActiveSounds` | `size_t` | `void` | Count active sounds |
| `SetListenerPosition` | `void` | `const Vec3& pos` | Set listener pos |
| `SetListenerOrientation` | `void` | `const Vec3& forward, const Vec3& up` | Set listener orientation |
| `GetListenerPosition` | `Vec3` | `void` | Get listener pos |
| `GetListenerForward` | `Vec3` | `void` | Get listener forward |
| `GetListenerUp` | `Vec3` | `void` | Get listener up |

---

## `audio/sound.h`

### `enum class SoundState`
| Value | Description |
|-------|-------------|
| `Stopped` | Not playing |
| `Playing` | Currently playing |
| `Paused` | Paused |

### `class Sound`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Sound` | (ctor) | `void` | Constructor |
| `Load` | `bool` | `const std::string& path` | Load from file |
| `Play` | `void` | `void` | Play sound |
| `Stop` | `void` | `void` | Stop sound |
| `Pause` | `void` | `void` | Pause sound |
| `Resume` | `void` | `void` | Resume sound |
| `SetVolume` | `void` | `float volume` | Set volume (0-1) |
| `GetVolume` | `float` | `void` | Get volume |
| `SetPitch` | `void` | `float pitch` | Set pitch (0.5-2.0) |
| `GetPitch` | `float` | `void` | Get pitch |
| `SetLoop` | `void` | `bool loop` | Set looping |
| `IsLooping` | `bool` | `void` | Check looping |
| `SetPosition` | `void` | `const Vec3& pos` | Set 3D position |
| `GetPosition` | `Vec3` | `void` | Get position |
| `SetMinDistance` | `void` | `float distance` | Set min 3D distance |
| `GetMinDistance` | `float` | `void` | Get min distance |
| `SetMaxDistance` | `void` | `float distance` | Set max 3D distance |
| `GetMaxDistance` | `float` | `void` | Get max distance |
| `GetState` | `SoundState` | `void` | Get state |
| `IsPlaying` | `bool` | `void` | Check playing |
| `GetDuration` | `float` | `void` | Get duration (seconds) |
| `GetPlaybackPosition` | `float` | `void` | Get current position |
| `SetPlaybackPosition` | `void` | `float position` | Seek to position |
| `IsLoaded` | `bool` | `void` | Check loaded |

---

## `audio/music.h`

### `class Music`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Music` | (ctor) | `void` | Constructor |
| `Load` | `bool` | `const std::string& path` | Load music file |
| `Play` | `void` | `void` | Start playback |
| `Stop` | `void` | `void` | Stop playback |
| `Pause` | `void` | `void` | Pause |
| `Resume` | `void` | `void` | Resume |
| `SetVolume` | `void` | `float volume` | Set volume (0-1) |
| `GetVolume` | `float` | `void` | Get volume |
| `SetPitch` | `void` | `float pitch` | Set pitch |
| `GetPitch` | `float` | `void` | Get pitch |
| `SetLoop` | `void` | `bool loop` | Set looping |
| `IsLooping` | `bool` | `void` | Check looping |
| `FadeIn` | `void` | `float duration` | Fade in over duration |
| `FadeOut` | `void` | `float duration` | Fade out over duration |
| `Crossfade` | `void` | `Music* other, float duration` | Crossfade to other track |
| `GetState` | `SoundState` | `void` | Get state |
| `IsPlaying` | `bool` | `void` | Check playing |
| `GetDuration` | `float` | `void` | Get total duration |
| `GetPlaybackPosition` | `float` | `void` | Get current position |
| `SetPlaybackPosition` | `void` | `float position` | Seek to position |
| `IsLoaded` | `bool` | `void` | Check loaded |

---

## `audio/audio_listener.h`

### `class AudioListener`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `AudioListener` | (ctor) | `void` | Constructor |
| `SetPosition` | `void` | `const Vec3& pos` | Set position |
| `GetPosition` | `Vec3` | `void` | Get position |
| `SetVelocity` | `void` | `const Vec3& vel` | Set velocity |
| `GetVelocity` | `Vec3` | `void` | Get velocity |
| `SetOrientation` | `void` | `const Vec3& forward, const Vec3& up` | Set orientation |
| `GetForward` | `Vec3` | `void` | Get forward vector |
| `GetUp` | `Vec3` | `void` | Get up vector |
| `SetGain` | `void` | `float gain` | Set master gain |
| `GetGain` | `float` | `void` | Get gain |
| `Enable` | `void` | `void` | Enable listener |
| `Disable` | `void` | `void` | Disable listener |
| `IsEnabled` | `bool` | `void` | Check enabled |

---

## `audio/audio_source.h`

### `class AudioSource`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `AudioSource` | (ctor) | `void` | Constructor |
| `SetSound` | `void` | `Sound* sound` | Set sound |
| `GetSound` | `Sound*` | `void` | Get sound |
| `Play` | `void` | `void` | Play |
| `Stop` | `void` | `void` | Stop |
| `Pause` | `void` | `void` | Pause |
| `Resume` | `void` | `void` | Resume |
| `SetVolume` | `void` | `float volume` | Set volume |
| `GetVolume` | `float` | `void` | Get volume |
| `SetPitch` | `void` | `float pitch` | Set pitch |
| `GetPitch` | `float` | `void` | Get pitch |
| `SetLoop` | `void` | `bool loop` | Set loop |
| `IsLooping` | `bool` | `void` | Check looping |
| `SetPosition` | `void` | `const Vec3& pos` | Set position |
| `GetPosition` | `Vec3` | `void` | Get position |
| `SetVelocity` | `void` | `const Vec3& vel` | Set velocity |
| `GetVelocity` | `Vec3` | `void` | Get velocity |
| `SetAttenuation` | `void` | `float min, float max` | Set attenuation range |
| `GetMinDistance` | `float` | `void` | Get min distance |
| `GetMaxDistance` | `float` | `void` | Get max distance |
| `IsPlaying` | `bool` | `void` | Check playing |
| `GetState` | `SoundState` | `void` | Get state |
