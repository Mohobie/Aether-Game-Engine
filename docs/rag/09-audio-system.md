# Audio System

The engine includes a 3D positional audio system for immersive sound.

## Audio Engine

```cpp
#include "audio/audio_engine.h"

// Initialize audio
vge::AudioEngine audio;
audio.Initialize();
audio.SetMasterVolume(1.0f);

// Update listener position (usually camera/player)
void Update(float dt) {
    audio.SetListenerPosition(camera.GetPosition());
    audio.SetListenerOrientation(
        camera.GetForward(),  // Look direction
        camera.GetUp()        // Up vector
    );
}
```

## Sound Manager

```cpp
#include "audio/sound_manager.h"

// Create sound manager
vge::SoundManager sounds;
sounds.Initialize();

// Load sounds
sounds.LoadSound("jump", "audio/jump.wav");
sounds.LoadSound("land", "audio/land.wav");
sounds.LoadSound("step", "audio/footstep.wav");
sounds.LoadSound("break", "audio/block_break.wav");
sounds.LoadSound("place", "audio/block_place.wav");
sounds.LoadSound("music", "audio/ambient_music.ogg");

// Play sound
sounds.Play("jump");

// Play with volume
sounds.Play("step", 0.5f);  // 50% volume

// Play 3D positioned sound
vge::Vec3 explosionPos(10, 5, 10);
sounds.Play3D("explosion", explosionPos, 1.0f);

// Play looping sound
uint32_t musicId = sounds.PlayLooping("music", 0.3f);

// Stop looping sound
sounds.Stop(musicId);

// Fade out
sounds.FadeOut(musicId, 2.0f);  // Fade over 2 seconds

// Check if playing
if (sounds.IsPlaying(musicId)) {
    // ...
}
```

## Sound Properties

```cpp
// Create sound with properties
vge::SoundProperties props;
props.volume = 1.0f;
props.pitch = 1.0f;       // 1.0 = normal, 2.0 = double speed/pitch
props.loop = false;
props.spatial = true;     // 3D positional audio
props.minDistance = 1.0f; // Distance where volume starts attenuating
props.maxDistance = 50.0f; // Distance where volume reaches 0
props.rolloff = 1.0f;     // How quickly sound fades with distance

uint32_t soundId = sounds.PlayWithProperties("explosion", props);
```

## Audio Sources

```cpp
// Attach sound to entity (follows entity position)
vge::AudioSource* source = entities.AddComponent<vge::AudioSource>(entity);
source->SetSound("engine");
source->SetLooping(true);
source->SetVolume(0.5f);
source->SetSpatial(true);
source->SetRange(5.0f, 100.0f);
source->Play();

// Update in system
class AudioSystem : public vge::System {
public:
    void Update(float dt) override {
        auto entities = GetEntities<TransformComponent, AudioSource>();
        
        for (auto entity : entities) {
            auto* transform = GetComponent<TransformComponent>(entity);
            auto* source = GetComponent<AudioSource>(entity);
            
            // Update 3D position
            source->SetPosition(transform->position);
        }
    }
};
```

## Music System

```cpp
class MusicManager {
    vge::SoundManager* sounds;
    std::vector<std::string> playlist;
    int currentTrack = 0;
    uint32_t currentMusic = 0;
    bool isPlaying = false;
    
public:
    void Initialize(vge::SoundManager* soundManager) {
        sounds = soundManager;
        
        // Load playlist
        playlist.push_back("music/morning.ogg");
        playlist.push_back("music/exploration.ogg");
        playlist.push_back("music/danger.ogg");
        playlist.push_back("music/night.ogg");
    }
    
    void Play(int trackIndex) {
        if (currentMusic != 0) {
            sounds->FadeOut(currentMusic, 1.0f);
        }
        
        currentTrack = trackIndex % playlist.size();
        currentMusic = sounds->Play(playlist[currentTrack], 0.3f);
        isPlaying = true;
    }
    
    void PlayForTime(float timeOfDay) {
        // timeOfDay: 0.0 = midnight, 0.25 = sunrise, 0.5 = noon, 0.75 = sunset
        if (timeOfDay < 0.2f || timeOfDay > 0.8f) {
            Play(3); // Night music
        } else if (timeOfDay < 0.3f || timeOfDay > 0.7f) {
            Play(0); // Morning/evening music
        } else {
            Play(1); // Day exploration
        }
    }
    
    void Next() {
        Play(currentTrack + 1);
    }
    
    void Stop() {
        if (currentMusic != 0) {
            sounds->FadeOut(currentMusic, 2.0f);
            currentMusic = 0;
        }
        isPlaying = false;
    }
    
    void SetVolume(float volume) {
        if (currentMusic != 0) {
            sounds->SetVolume(currentMusic, volume);
        }
    }
};
```

## Ambient Sounds

```cpp
class AmbientAudio {
    vge::SoundManager* sounds;
    
    // Ambient loops
    uint32_t windSound = 0;
    uint32_t rainSound = 0;
    uint32_t caveSound = 0;
    
public:
    void Initialize(vge::SoundManager* soundManager) {
        sounds = soundManager;
        
        windSound = sounds->PlayLooping("ambient/wind.ogg", 0.0f);
        rainSound = sounds->PlayLooping("ambient/rain.ogg", 0.0f);
        caveSound = sounds->PlayLooping("ambient/cave.ogg", 0.0f);
    }
    
    void Update(const WeatherSystem& weather, const Player& player) {
        // Wind based on weather
        float windVolume = 0.1f;
        if (weather.IsStormy()) windVolume = 0.6f;
        else if (weather.IsWindy()) windVolume = 0.3f;
        sounds->SetVolume(windSound, windVolume);
        
        // Rain based on weather
        float rainVolume = weather.IsRaining() ? 0.5f : 0.0f;
        if (weather.IsStormy()) rainVolume = 0.8f;
        sounds->SetVolume(rainSound, rainVolume);
        
        // Cave ambience when underground
        float caveVolume = 0.0f;
        if (player.GetPosition().y < 30) {
            caveVolume = 1.0f - (player.GetPosition().y / 30.0f);
            caveVolume = std::clamp(caveVolume, 0.0f, 0.5f);
        }
        sounds->SetVolume(caveSound, caveVolume);
    }
};
```

## Footstep System

```cpp
class FootstepSystem {
    vge::SoundManager* sounds;
    float stepTimer = 0.0f;
    float stepInterval = 0.5f;  // Seconds between steps
    
public:
    void Initialize(vge::SoundManager* soundManager) {
        sounds = soundManager;
        
        // Load footstep sounds for different surfaces
        sounds->LoadSound("step_grass", "audio/footstep_grass.wav");
        sounds->LoadSound("step_stone", "audio/footstep_stone.wav");
        sounds->LoadSound("step_wood", "audio/footstep_wood.wav");
        sounds->LoadSound("step_metal", "audio/footstep_metal.wav");
        sounds->LoadSound("step_water", "audio/footstep_water.wav");
    }
    
    void Update(float dt, const Player& player) {
        if (!player.IsMoving() || !player.IsGrounded()) {
            return;
        }
        
        // Update timer
        stepTimer -= dt;
        if (stepTimer <= 0) {
            // Play footstep
            PlayFootstep(player);
            
            // Reset timer based on movement speed
            float speed = player.GetVelocity().Length();
            stepTimer = stepInterval / (speed / player.GetWalkSpeed());
        }
    }
    
private:
    void PlayFootstep(const Player& player) {
        // Determine surface type
        vge::Vec3 footPos = player.GetPosition() - vge::Vec3(0, 0.1f, 0);
        vge::BlockType block = world.GetBlock(footPos);
        
        std::string soundName;
        switch (block) {
            case vge::BlockType::Grass:
            case vge::BlockType::Dirt:
                soundName = "step_grass";
                break;
            case vge::BlockType::Stone:
            case vge::BlockType::Cobblestone:
                soundName = "step_stone";
                break;
            case vge::BlockType::Wood:
            case vge::BlockType::Planks:
                soundName = "step_wood";
                break;
            case vge::BlockType::Metal:
                soundName = "step_metal";
                break;
            default:
                soundName = "step_grass";
        }
        
        // Randomize pitch slightly for variety
        float pitch = 0.9f + (rand() / (float)RAND_MAX) * 0.2f;
        sounds->Play(soundName, 0.3f, pitch);
    }
};
```

## Sound Effects

```cpp
class SFXManager {
    vge::SoundManager* sounds;
    
public:
    void Initialize(vge::SoundManager* soundManager) {
        sounds = soundManager;
        
        // UI sounds
        sounds->LoadSound("ui_click", "audio/ui_click.wav");
        sounds->LoadSound("ui_hover", "audio/ui_hover.wav");
        sounds->LoadSound("ui_open", "audio/ui_open.wav");
        sounds->LoadSound("ui_close", "audio/ui_close.wav");
        
        // Game sounds
        sounds->LoadSound("pickup", "audio/pickup.wav");
        sounds->LoadSound("drop", "audio/drop.wav");
        sounds->LoadSound("craft", "audio/craft.wav");
        sounds->LoadSound("levelup", "audio/levelup.wav");
        sounds->LoadSound("damage", "audio/damage.wav");
        sounds->LoadSound("death", "audio/death.wav");
        sounds->LoadSound("explosion", "audio/explosion.wav");
        sounds->LoadSound("shoot", "audio/shoot.wav");
        sounds->LoadSound("reload", "audio/reload.wav");
        sounds->LoadSound("hit", "audio/hit.wav");
        sounds->LoadSound("break", "audio/block_break.wav");
        sounds->LoadSound("place", "audio/block_place.wav");
    }
    
    void PlayUI(const std::string& sound) {
        sounds->Play(sound, 0.5f);
    }
    
    void PlayGame(const std::string& sound, const vge::Vec3& pos) {
        sounds->Play3D(sound, pos, 1.0f);
    }
    
    void PlayHit(const vge::Vec3& pos) {
        sounds->Play3D("hit", pos, 0.7f);
    }
    
    void PlayExplosion(const vge::Vec3& pos, float intensity) {
        sounds->Play3D("explosion", pos, intensity);
        
        // Screen shake
        camera.Shake(intensity * 0.5f, 0.5f);
    }
};
```

## Audio Mixer

```cpp
class AudioMixer {
    struct Channel {
        float volume = 1.0f;
        bool muted = false;
    };
    
    std::unordered_map<std::string, Channel> channels;
    
public:
    void Initialize() {
        channels["Master"] = {1.0f, false};
        channels["Music"] = {0.5f, false};
        channels["SFX"] = {0.8f, false};
        channels["UI"] = {0.6f, false};
        channels["Ambient"] = {0.4f, false};
        channels["Voice"] = {1.0f, false};
    }
    
    void SetVolume(const std::string& channel, float volume) {
        if (channels.find(channel) != channels.end()) {
            channels[channel].volume = std::clamp(volume, 0.0f, 1.0f);
        }
    }
    
    float GetVolume(const std::string& channel) {
        if (channels.find(channel) != channels.end()) {
            return channels[channel].muted ? 0.0f : channels[channel].volume;
        }
        return 0.0f;
    }
    
    void Mute(const std::string& channel) {
        if (channels.find(channel) != channels.end()) {
            channels[channel].muted = true;
        }
    }
    
    void Unmute(const std::string& channel) {
        if (channels.find(channel) != channels.end()) {
            channels[channel].muted = false;
        }
    }
    
    void ToggleMute(const std::string& channel) {
        if (channels.find(channel) != channels.end()) {
            channels[channel].muted = !channels[channel].muted;
        }
    }
    
    // Calculate final volume for a sound
    float GetFinalVolume(const std::string& channel, float soundVolume) {
        float masterVol = GetVolume("Master");
        float channelVol = GetVolume(channel);
        return masterVol * channelVol * soundVolume;
    }
};
```

## Loading and Streaming

```cpp
// Load sound (kept in memory)
sounds->LoadSound("explosion", "audio/explosion.wav");

// Stream music (loaded in chunks, good for long files)
sounds->StreamSound("music", "audio/music.ogg");

// Preload common sounds
void PreloadSounds() {
    std::vector<std::string> commonSounds = {
        "step_grass", "step_stone", "jump", "land",
        "break", "place", "ui_click", "ui_hover"
    };
    
    for (const auto& sound : commonSounds) {
        sounds->LoadSound(sound, "audio/" + sound + ".wav");
    }
}

// Unload unused sounds to free memory
void UnloadRareSounds() {
    sounds->UnloadSound("boss_music");
    sounds->UnloadSound("rare_item");
}
```
