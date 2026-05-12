#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include "math/vec3.h"

namespace vge {

// ============================================================
// Audio Backend Interface
// ============================================================
// Abstract base for audio backends (OpenAL, SDL2_mixer, mock, etc.)

class AudioBackend {
public:
    virtual ~AudioBackend() = default;

    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual bool IsInitialized() const = 0;

    // Load a sound file into memory (returns handle)
    virtual int LoadSound(const std::string& path) = 0;
    virtual void UnloadSound(int handle) = 0;

    // Source management
    virtual int CreateSource() = 0;
    virtual void DestroySource(int sourceId) = 0;
    virtual void SetSourceSound(int sourceId, int soundHandle) = 0;
    virtual void PlaySource(int sourceId) = 0;
    virtual void StopSource(int sourceId) = 0;
    virtual void PauseSource(int sourceId) = 0;
    virtual bool IsSourcePlaying(int sourceId) const = 0;
    virtual void SetSourceLooping(int sourceId, bool loop) = 0;

    // 3D positioning
    virtual void SetSourcePosition(int sourceId, const Vec3& pos) = 0;
    virtual void SetSourceVelocity(int sourceId, const Vec3& vel) = 0;
    virtual void SetSourceVolume(int sourceId, float volume) = 0;
    virtual void SetSourcePitch(int sourceId, float pitch) = 0;
    virtual void SetSourceMaxDistance(int sourceId, float distance) = 0;
    virtual void SetSourceRolloff(int sourceId, float rolloff) = 0;

    // Listener
    virtual void SetListenerPosition(const Vec3& pos) = 0;
    virtual void SetListenerOrientation(const Vec3& forward, const Vec3& up) = 0;
    virtual void SetListenerVolume(float volume) = 0;

    // Master volume
    virtual void SetMasterVolume(float volume) = 0;
};

// ============================================================
// Volume Groups
// ============================================================

enum class VolumeGroup {
    Master,
    Music,
    SFX,
    Voice,
    Ambient,
    UI,
    Count
};

// ============================================================
// Pooled Audio Source
// ============================================================

struct PooledSource {
    int backendId = -1;         // Backend source handle
    int soundHandle = -1;       // Loaded sound handle
    bool inUse = false;
    bool is3D = false;
    bool loop = false;
    float volume = 1.0f;
    float pitch = 1.0f;
    int priority = 0;           // Higher = more important
    VolumeGroup group = VolumeGroup::SFX;
    Vec3 position;
    float maxDistance = 100.0f;
    float rolloff = 1.0f;
    float age = 0.0f;           // Time since playback started

    void Reset();
};

// ============================================================
// Audio Engine
// ============================================================

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    // Initialization
    bool Initialize(std::unique_ptr<AudioBackend> backend = nullptr);
    void Shutdown();
    bool IsInitialized() const;

    // Volume groups
    void SetVolume(VolumeGroup group, float volume);
    float GetVolume(VolumeGroup group) const;
    void SetMasterVolume(float volume);
    float GetMasterVolume() const;

    // Listener
    void SetListenerPosition(const Vec3& pos);
    void SetListenerOrientation(const Vec3& forward, const Vec3& up);

    // Sound loading
    int LoadSound(const std::string& path);
    void UnloadSound(int handle);

    // Playback
    // Returns pooled source index, or -1 if no source available
    int Play2D(int soundHandle, VolumeGroup group = VolumeGroup::SFX,
               float volume = 1.0f, float pitch = 1.0f, bool loop = false, int priority = 0);

    int Play3D(int soundHandle, const Vec3& position,
               VolumeGroup group = VolumeGroup::SFX,
               float volume = 1.0f, float pitch = 1.0f,
               bool loop = false, int priority = 0,
               float maxDistance = 100.0f, float rolloff = 1.0f);

    void Stop(int sourceIndex);
    void StopAll();
    void StopGroup(VolumeGroup group);
    void Pause(int sourceIndex);
    void Resume(int sourceIndex);
    bool IsPlaying(int sourceIndex) const;

    // Source control
    void SetSourcePosition(int sourceIndex, const Vec3& pos);
    void SetSourceVolume(int sourceIndex, float volume);
    void SetSourcePitch(int sourceIndex, float pitch);

    // Update (call once per frame)
    void Update(float deltaTime);

    // Stats
    size_t GetActiveSourceCount() const;
    size_t GetTotalSourceCount() const;
    size_t GetLoadedSoundCount() const;

    // Backend access (for advanced use)
    AudioBackend* GetBackend() const { return backend.get(); }

private:
    std::unique_ptr<AudioBackend> backend;
    bool initialized = false;

    // Volume groups (0.0 - 1.0)
    float volumes[static_cast<int>(VolumeGroup::Count)];

    // Source pool
    std::vector<PooledSource> sourcePool;
    static constexpr size_t DEFAULT_POOL_SIZE = 32;
    static constexpr size_t MAX_POOL_SIZE = 64;

    // Loaded sounds
    std::unordered_map<int, std::string> loadedSounds;
    int nextSoundHandle = 1;

    // Internal helpers
    int AcquireSource(int priority);
    void ReleaseSource(int index);
    float CalculateAttenuation(const Vec3& sourcePos, float maxDist, float rolloff) const;
    void ApplyVolumeToSource(int index);
};

} // namespace vge
