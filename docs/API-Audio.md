# Audio Module API
**Files:** src/audio/audio_engine.h, src/audio/music_player.h, src/audio/sound.h, src/audio/sound_manager.h

## `audio/audio_engine.h`
```cpp
namespace vge {
```

### `class AudioEngine`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Initialize` | `bool` | `` |
| `Shutdown` | `void` | `` |
| `PlaySound` | `void` | `const std::string& file` |
| `SetMasterVolume` | `void` | `float volume` |

## `audio/music_player.h`
```cpp
namespace aether {
```

### `class MusicPlayer`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `loadPlaylist` | `bool` | `const std::vector<std::string>& tracks` |
| `play` | `void` | `` |
| `pause` | `void` | `` |
| `stop` | `void` | `` |
| `next` | `void` | `` |
| `previous` | `void` | `` |
| `setVolume` | `void` | `float volume` |
| `isPlaying` | `bool` | `` |

## `audio/sound.h`
```cpp
namespace aether {
```

### `class Sound`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `loadFromFile` | `bool` | `const std::string& path` |
| `play` | `void` | `` |
| `stop` | `void` | `` |
| `setVolume` | `void` | `float volume` |
| `setLooping` | `void` | `bool loop` |
| `isPlaying` | `bool` | `` |

## `audio/sound_manager.h`
```cpp
namespace vge {
```

### `class SoundManager`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Initialize` | `bool` | `AudioEngine* engine` |
| `Shutdown` | `void` | `` |
| `LoadSound` | `bool` | `const std::string& name, const std::string& path` |
| `PlaySound` | `void` | `const std::string& name, float volume = 1.0f, bool loop = false` |
| `StopSound` | `void` | `const std::string& name` |
| `StopAllSounds` | `void` | `` |
| `SetMasterVolume` | `void` | `float volume` |
| `PlayBlockPlace` | `void` | `BlockType type` |
| `PlayBlockBreak` | `void` | `BlockType type` |
| `PlayFootstep` | `void` | `BlockType groundType` |
| `PlayJump` | `void` | `` |
| `PlayLand` | `void` | `` |
