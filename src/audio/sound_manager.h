#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <random>
#include "math/vec3.h"
#include "audio/audio_engine.h"
#include "audio/audio_events.h"

namespace vge {

// ============================================================
// Music Playlist
// ============================================================

enum class PlaylistMode {
    Sequential,
    Shuffle,
    LoopOne,
    Random
};

class MusicPlaylist {
public:
    MusicPlaylist();

    // Track management
    void AddTrack(const std::string& path, const std::string& name = "");
    void RemoveTrack(size_t index);
    void Clear();
    size_t GetTrackCount() const { return tracks.size(); }

    // Playback control
    void Play(size_t index = 0);
    void Stop();
    void Pause();
    void Resume();
    void Next();
    void Previous();

    // Settings
    void SetMode(PlaylistMode mode) { playMode = mode; }
    void SetVolume(float volume);
    void SetCrossfadeDuration(float seconds) { crossfadeDuration = seconds; }

    // State
    bool IsPlaying() const;
    size_t GetCurrentTrackIndex() const { return currentTrack; }
    std::string GetCurrentTrackName() const;

    // Update (call once per frame for crossfade)
    void Update(float deltaTime);

private:
    struct Track {
        std::string path;
        std::string name;
        int soundHandle = -1;
    };

    std::vector<Track> tracks;
    size_t currentTrack = 0;
    PlaylistMode playMode = PlaylistMode::Sequential;

    // Playback state
    int activeSource = -1;      // Currently playing source index
    int fadingSource = -1;      // Source being faded out
    float volume = 1.0f;
    float crossfadeDuration = 2.0f;
    float fadeTimer = 0.0f;
    bool isPlaying = false;
    bool isPaused = false;

    // Random
    std::mt19937 rng;

    // Reference to audio engine (set by SoundManager)
    AudioEngine* audioEngine = nullptr;

    void StartTrack(size_t index);
    void UpdateCrossfade(float deltaTime);

    friend class SoundManager;
};

// ============================================================
// Ambient Sound Zone
// ============================================================

struct AmbientZone {
    std::string name;
    Vec3 center;
    float radius;
    float fadeDistance;         // Distance over which to fade in/out
    AudioEventType ambientType;
    float volume = 1.0f;

    // Runtime
    int sourceIndex = -1;
    float currentVolume = 0.0f;
    bool isActive = false;

    AmbientZone() : center(0, 0, 0), radius(50.0f), fadeDistance(10.0f),
                    ambientType(AudioEventType::AmbientForest), volume(1.0f) {}
};

// ============================================================
// Sound Manager
// ============================================================

class SoundManager {
public:
    SoundManager();
    ~SoundManager();

    // Initialization
    bool Initialize(AudioEngine* engine);
    void Shutdown();
    bool IsInitialized() const;

    // Sound packs
    void LoadSoundPack(const SoundPack& pack);
    void LoadSoundPack(const std::string& packName);  // Load from file
    void SetActivePack(const std::string& packName);
    const SoundPack* GetActivePack() const;
    const SoundPack* GetPack(const std::string& packName) const;

    // Event-driven sound playback
    int PlayEvent(AudioEventType event, const Vec3& position = Vec3(0, 0, 0),
                  float volumeMultiplier = 1.0f, float pitchMultiplier = 1.0f);
    int PlayEvent2D(AudioEventType event, float volumeMultiplier = 1.0f);
    int PlayCustomEvent(const std::string& eventName, const Vec3& position = Vec3(0, 0, 0),
                        float volumeMultiplier = 1.0f);

    // Direct sound playback (for one-offs not in the event system)
    int PlaySound(const std::string& filePath, const Vec3& position = Vec3(0, 0, 0),
                  VolumeGroup group = VolumeGroup::SFX,
                  float volume = 1.0f, bool loop = false);
    int PlaySound2D(const std::string& filePath, VolumeGroup group = VolumeGroup::SFX,
                    float volume = 1.0f, bool loop = false);

    void StopEvent(int sourceIndex);
    void StopAllEvents();
    void StopAllByGroup(VolumeGroup group);

    // Music
    MusicPlaylist& GetMusicPlaylist() { return musicPlaylist; }
    void PlayMusic(const std::string& trackPath);
    void StopMusic();
    void SetMusicVolume(float volume);

    // Ambient zones
    AmbientZone* AddAmbientZone(const std::string& name, const Vec3& center,
                                 float radius, AudioEventType ambientType);
    void RemoveAmbientZone(const std::string& name);
    void UpdateAmbientZones(const Vec3& listenerPos);
    void ClearAmbientZones();

    // Volume
    void SetVolume(VolumeGroup group, float volume);
    float GetVolume(VolumeGroup group) const;
    void SetMasterVolume(float volume);

    // Listener
    void SetListenerPosition(const Vec3& pos);
    void SetListenerOrientation(const Vec3& forward, const Vec3& up);

    // Update (call once per frame)
    void Update(float deltaTime);

    // Stats
    size_t GetActiveSoundCount() const;
    size_t GetLoadedPackCount() const { return soundPacks.size(); }

private:
    AudioEngine* audioEngine = nullptr;
    bool initialized = false;

    // Sound packs
    std::unordered_map<std::string, SoundPack> soundPacks;
    std::string activePackName = "default";

    // Music
    MusicPlaylist musicPlaylist;

    // Ambient zones
    std::vector<std::unique_ptr<AmbientZone>> ambientZones;

    // Tracking active event sources
    std::vector<int> activeEventSources;

    // Internal helpers
    int LoadSoundForEvent(const SoundEntry& entry);
    float RandomPitch(float minPitch, float maxPitch);
    void CleanupFinishedSources();
};

} // namespace vge
