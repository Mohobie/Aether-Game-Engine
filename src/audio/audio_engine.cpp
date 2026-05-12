#include "audio_engine.h"
#include "core/logger.h"
#include <iostream>
#include <cmath>
#include <algorithm>

namespace vge {

// ============================================================
// Mock Audio Backend (for testing without hardware)
// ============================================================

class MockAudioBackend : public AudioBackend {
public:
    bool Initialize() override {
        std::cout << "[MockAudio] Initialized" << std::endl;
        initialized = true;
        return true;
    }

    void Shutdown() override {
        sounds.clear();
        sources.clear();
        initialized = false;
        std::cout << "[MockAudio] Shutdown" << std::endl;
    }

    bool IsInitialized() const override { return initialized; }

    int LoadSound(const std::string& path) override {
        int handle = nextHandle++;
        sounds[handle] = path;
        return handle;
    }

    void UnloadSound(int handle) override {
        sounds.erase(handle);
    }

    int CreateSource() override {
        int id = nextSourceId++;
        SourceState s;
        s.id = id;
        sources[id] = s;
        return id;
    }

    void DestroySource(int sourceId) override {
        sources.erase(sourceId);
    }

    void SetSourceSound(int sourceId, int soundHandle) override {
        auto it = sources.find(sourceId);
        if (it != sources.end()) {
            it->second.soundHandle = soundHandle;
        }
    }

    void PlaySource(int sourceId) override {
        auto it = sources.find(sourceId);
        if (it != sources.end()) {
            it->second.playing = true;
        }
    }

    void StopSource(int sourceId) override {
        auto it = sources.find(sourceId);
        if (it != sources.end()) {
            it->second.playing = false;
        }
    }

    void PauseSource(int sourceId) override {
        auto it = sources.find(sourceId);
        if (it != sources.end()) {
            it->second.playing = false;
        }
    }

    bool IsSourcePlaying(int sourceId) const override {
        auto it = sources.find(sourceId);
        if (it != sources.end()) {
            return it->second.playing;
        }
        return false;
    }

    void SetSourceLooping(int sourceId, bool loop) override {
        auto it = sources.find(sourceId);
        if (it != sources.end()) {
            it->second.looping = loop;
        }
    }

    void SetSourcePosition(int sourceId, const Vec3& pos) override {
        auto it = sources.find(sourceId);
        if (it != sources.end()) {
            it->second.position = pos;
        }
    }

    void SetSourceVelocity(int sourceId, const Vec3& vel) override {
        (void)vel;
        (void)sourceId;
    }

    void SetSourceVolume(int sourceId, float volume) override {
        auto it = sources.find(sourceId);
        if (it != sources.end()) {
            it->second.volume = volume;
        }
    }

    void SetSourcePitch(int sourceId, float pitch) override {
        auto it = sources.find(sourceId);
        if (it != sources.end()) {
            it->second.pitch = pitch;
        }
    }

    void SetSourceMaxDistance(int sourceId, float distance) override {
        auto it = sources.find(sourceId);
        if (it != sources.end()) {
            it->second.maxDistance = distance;
        }
    }

    void SetSourceRolloff(int sourceId, float rolloff) override {
        auto it = sources.find(sourceId);
        if (it != sources.end()) {
            it->second.rolloff = rolloff;
        }
    }

    void SetListenerPosition(const Vec3& pos) override {
        listenerPos = pos;
    }

    void SetListenerOrientation(const Vec3& forward, const Vec3& up) override {
        listenerForward = forward;
        listenerUp = up;
    }

    void SetListenerVolume(float volume) override {
        listenerVolume = volume;
    }

    void SetMasterVolume(float volume) override {
        masterVolume = volume;
    }

private:
    struct SourceState {
        int id = -1;
        int soundHandle = -1;
        bool playing = false;
        bool looping = false;
        Vec3 position;
        float volume = 1.0f;
        float pitch = 1.0f;
        float maxDistance = 100.0f;
        float rolloff = 1.0f;
    };

    bool initialized = false;
    int nextHandle = 1;
    int nextSourceId = 1;
    std::unordered_map<int, std::string> sounds;
    std::unordered_map<int, SourceState> sources;
    Vec3 listenerPos;
    Vec3 listenerForward;
    Vec3 listenerUp;
    float listenerVolume = 1.0f;
    float masterVolume = 1.0f;
};

// ============================================================
// PooledSource
// ============================================================

void PooledSource::Reset() {
    backendId = -1;
    soundHandle = -1;
    inUse = false;
    is3D = false;
    loop = false;
    volume = 1.0f;
    pitch = 1.0f;
    priority = 0;
    group = VolumeGroup::SFX;
    position = Vec3(0, 0, 0);
    maxDistance = 100.0f;
    rolloff = 1.0f;
    age = 0.0f;
}

// ============================================================
// AudioEngine
// ============================================================

AudioEngine::AudioEngine() {
    // Default all volumes to 1.0
    for (int i = 0; i < static_cast<int>(VolumeGroup::Count); ++i) {
        volumes[i] = 1.0f;
    }
}

AudioEngine::~AudioEngine() {
    if (initialized) {
        Shutdown();
    }
}

bool AudioEngine::Initialize(std::unique_ptr<AudioBackend> customBackend) {
    if (initialized) {
        return true;
    }

    if (customBackend) {
        backend = std::move(customBackend);
    } else {
        // Default to mock backend for testing
        backend = std::make_unique<MockAudioBackend>();
    }

    if (!backend->Initialize()) {
        std::cerr << "[AudioEngine] Backend initialization failed" << std::endl;
        backend.reset();
        return false;
    }

    // Create source pool
    sourcePool.resize(DEFAULT_POOL_SIZE);
    for (size_t i = 0; i < DEFAULT_POOL_SIZE; ++i) {
        sourcePool[i].backendId = backend->CreateSource();
        sourcePool[i].Reset();
    }

    initialized = true;
    std::cout << "[AudioEngine] Initialized with " << DEFAULT_POOL_SIZE << " sources" << std::endl;
    return true;
}

void AudioEngine::Shutdown() {
    if (!initialized || !backend) return;

    // Stop all sources
    StopAll();

    // Destroy pooled sources
    for (auto& source : sourcePool) {
        if (source.backendId >= 0) {
            backend->DestroySource(source.backendId);
        }
    }
    sourcePool.clear();

    // Unload all sounds
    loadedSounds.clear();

    backend->Shutdown();
    backend.reset();
    initialized = false;

    std::cout << "[AudioEngine] Shutdown complete" << std::endl;
}

bool AudioEngine::IsInitialized() const {
    return initialized && backend && backend->IsInitialized();
}

// ============================================================
// Volume Groups
// ============================================================

void AudioEngine::SetVolume(VolumeGroup group, float volume) {
    int idx = static_cast<int>(group);
    if (idx < 0 || idx >= static_cast<int>(VolumeGroup::Count)) return;

    volumes[idx] = std::max(0.0f, std::min(1.0f, volume));

    // Update all active sources in this group
    for (size_t i = 0; i < sourcePool.size(); ++i) {
        if (sourcePool[i].inUse && sourcePool[i].group == group) {
            ApplyVolumeToSource(static_cast<int>(i));
        }
    }
}

float AudioEngine::GetVolume(VolumeGroup group) const {
    int idx = static_cast<int>(group);
    if (idx < 0 || idx >= static_cast<int>(VolumeGroup::Count)) return 1.0f;
    return volumes[idx];
}

void AudioEngine::SetMasterVolume(float volume) {
    volumes[static_cast<int>(VolumeGroup::Master)] = std::max(0.0f, std::min(1.0f, volume));

    // Update all active sources
    for (size_t i = 0; i < sourcePool.size(); ++i) {
        if (sourcePool[i].inUse) {
            ApplyVolumeToSource(static_cast<int>(i));
        }
    }
}

float AudioEngine::GetMasterVolume() const {
    return volumes[static_cast<int>(VolumeGroup::Master)];
}

// ============================================================
// Listener
// ============================================================

void AudioEngine::SetListenerPosition(const Vec3& pos) {
    if (backend) {
        backend->SetListenerPosition(pos);
    }
}

void AudioEngine::SetListenerOrientation(const Vec3& forward, const Vec3& up) {
    if (backend) {
        backend->SetListenerOrientation(forward, up);
    }
}

// ============================================================
// Sound Loading
// ============================================================

int AudioEngine::LoadSound(const std::string& path) {
    if (!initialized || !backend) return -1;

    int handle = backend->LoadSound(path);
    if (handle >= 0) {
        loadedSounds[handle] = path;
    }
    return handle;
}

void AudioEngine::UnloadSound(int handle) {
    if (!initialized || !backend) return;

    loadedSounds.erase(handle);
    backend->UnloadSound(handle);
}

// ============================================================
// Playback
// ============================================================

int AudioEngine::Play2D(int soundHandle, VolumeGroup group, float volume, float pitch,
                        bool loop, int priority) {
    if (!initialized || !backend) return -1;

    int index = AcquireSource(priority);
    if (index < 0) {
        std::cerr << "[AudioEngine] No available sources for 2D playback" << std::endl;
        return -1;
    }

    PooledSource& source = sourcePool[index];
    source.soundHandle = soundHandle;
    source.group = group;
    source.volume = volume;
    source.pitch = pitch;
    source.loop = loop;
    source.priority = priority;
    source.is3D = false;
    source.age = 0.0f;
    source.inUse = true;

    backend->SetSourceSound(source.backendId, soundHandle);
    backend->SetSourceLooping(source.backendId, loop);
    backend->SetSourcePitch(source.backendId, pitch);
    ApplyVolumeToSource(index);
    backend->PlaySource(source.backendId);

    return index;
}

int AudioEngine::Play3D(int soundHandle, const Vec3& position, VolumeGroup group,
                        float volume, float pitch, bool loop, int priority,
                        float maxDistance, float rolloff) {
    if (!initialized || !backend) return -1;

    int index = AcquireSource(priority);
    if (index < 0) {
        std::cerr << "[AudioEngine] No available sources for 3D playback" << std::endl;
        return -1;
    }

    PooledSource& source = sourcePool[index];
    source.soundHandle = soundHandle;
    source.group = group;
    source.volume = volume;
    source.pitch = pitch;
    source.loop = loop;
    source.priority = priority;
    source.is3D = true;
    source.position = position;
    source.maxDistance = maxDistance;
    source.rolloff = rolloff;
    source.age = 0.0f;
    source.inUse = true;

    backend->SetSourceSound(source.backendId, soundHandle);
    backend->SetSourceLooping(source.backendId, loop);
    backend->SetSourcePitch(source.backendId, pitch);
    backend->SetSourcePosition(source.backendId, position);
    backend->SetSourceMaxDistance(source.backendId, maxDistance);
    backend->SetSourceRolloff(source.backendId, rolloff);
    ApplyVolumeToSource(index);
    backend->PlaySource(source.backendId);

    return index;
}

void AudioEngine::Stop(int sourceIndex) {
    if (sourceIndex < 0 || sourceIndex >= static_cast<int>(sourcePool.size())) return;

    PooledSource& source = sourcePool[sourceIndex];
    if (source.inUse && backend) {
        backend->StopSource(source.backendId);
        ReleaseSource(sourceIndex);
    }
}

void AudioEngine::StopAll() {
    for (size_t i = 0; i < sourcePool.size(); ++i) {
        if (sourcePool[i].inUse) {
            if (backend) {
                backend->StopSource(sourcePool[i].backendId);
            }
            ReleaseSource(static_cast<int>(i));
        }
    }
}

void AudioEngine::StopGroup(VolumeGroup group) {
    for (size_t i = 0; i < sourcePool.size(); ++i) {
        if (sourcePool[i].inUse && sourcePool[i].group == group) {
            if (backend) {
                backend->StopSource(sourcePool[i].backendId);
            }
            ReleaseSource(static_cast<int>(i));
        }
    }
}

void AudioEngine::Pause(int sourceIndex) {
    if (sourceIndex < 0 || sourceIndex >= static_cast<int>(sourcePool.size())) return;
    if (sourcePool[sourceIndex].inUse && backend) {
        backend->PauseSource(sourcePool[sourceIndex].backendId);
    }
}

void AudioEngine::Resume(int sourceIndex) {
    if (sourceIndex < 0 || sourceIndex >= static_cast<int>(sourcePool.size())) return;
    if (sourcePool[sourceIndex].inUse && backend) {
        backend->PlaySource(sourcePool[sourceIndex].backendId);
    }
}

bool AudioEngine::IsPlaying(int sourceIndex) const {
    if (sourceIndex < 0 || sourceIndex >= static_cast<int>(sourcePool.size())) return false;
    if (!sourcePool[sourceIndex].inUse || !backend) return false;
    return backend->IsSourcePlaying(sourcePool[sourceIndex].backendId);
}

void AudioEngine::SetSourcePosition(int sourceIndex, const Vec3& pos) {
    if (sourceIndex < 0 || sourceIndex >= static_cast<int>(sourcePool.size())) return;
    PooledSource& source = sourcePool[sourceIndex];
    source.position = pos;
    if (backend) {
        backend->SetSourcePosition(source.backendId, pos);
        ApplyVolumeToSource(sourceIndex);
    }
}

void AudioEngine::SetSourceVolume(int sourceIndex, float volume) {
    if (sourceIndex < 0 || sourceIndex >= static_cast<int>(sourcePool.size())) return;
    sourcePool[sourceIndex].volume = volume;
    ApplyVolumeToSource(sourceIndex);
}

void AudioEngine::SetSourcePitch(int sourceIndex, float pitch) {
    if (sourceIndex < 0 || sourceIndex >= static_cast<int>(sourcePool.size())) return;
    sourcePool[sourceIndex].pitch = pitch;
    if (backend) {
        backend->SetSourcePitch(sourcePool[sourceIndex].backendId, pitch);
    }
}

// ============================================================
// Update
// ============================================================

void AudioEngine::Update(float deltaTime) {
    if (!initialized || !backend) return;

    for (size_t i = 0; i < sourcePool.size(); ++i) {
        PooledSource& source = sourcePool[i];
        if (!source.inUse) continue;

        source.age += deltaTime;

        // Check if non-looping source has finished
        if (!source.loop && !backend->IsSourcePlaying(source.backendId)) {
            ReleaseSource(static_cast<int>(i));
            continue;
        }

        // Update 3D attenuation
        if (source.is3D) {
            ApplyVolumeToSource(static_cast<int>(i));
        }
    }
}

// ============================================================
// Stats
// ============================================================

size_t AudioEngine::GetActiveSourceCount() const {
    size_t count = 0;
    for (const auto& source : sourcePool) {
        if (source.inUse) ++count;
    }
    return count;
}

size_t AudioEngine::GetTotalSourceCount() const {
    return sourcePool.size();
}

size_t AudioEngine::GetLoadedSoundCount() const {
    return loadedSounds.size();
}

// ============================================================
// Internal Helpers
// ============================================================

int AudioEngine::AcquireSource(int priority) {
    // First, try to find a free source
    for (size_t i = 0; i < sourcePool.size(); ++i) {
        if (!sourcePool[i].inUse) {
            return static_cast<int>(i);
        }
    }

    // If pool is full, try to steal from a lower-priority source
    int stealIndex = -1;
    int lowestPriority = priority;

    for (size_t i = 0; i < sourcePool.size(); ++i) {
        if (sourcePool[i].priority < lowestPriority) {
            lowestPriority = sourcePool[i].priority;
            stealIndex = static_cast<int>(i);
        }
    }

    if (stealIndex >= 0) {
        // Steal this source
        if (backend) {
            backend->StopSource(sourcePool[stealIndex].backendId);
        }
        sourcePool[stealIndex].Reset();
        return stealIndex;
    }

    // Expand pool if possible
    if (sourcePool.size() < MAX_POOL_SIZE) {
        size_t newIndex = sourcePool.size();
        sourcePool.emplace_back();
        sourcePool[newIndex].backendId = backend->CreateSource();
        sourcePool[newIndex].Reset();
        return static_cast<int>(newIndex);
    }

    return -1; // No source available
}

void AudioEngine::ReleaseSource(int index) {
    if (index < 0 || index >= static_cast<int>(sourcePool.size())) return;
    sourcePool[index].Reset();
}

float AudioEngine::CalculateAttenuation(const Vec3& sourcePos, float maxDist, float rolloff) const {
    // Get listener position from backend (simplified - using origin for mock)
    // In a real implementation, we'd query the backend for listener position
    Vec3 listenerPos(0, 0, 0);
    Vec3 diff = sourcePos - listenerPos;
    float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);

    if (dist >= maxDist) return 0.0f;
    if (dist <= 0.0f) return 1.0f;

    // Inverse square law with rolloff
    return 1.0f / (1.0f + dist * rolloff);
}

void AudioEngine::ApplyVolumeToSource(int index) {
    if (index < 0 || index >= static_cast<int>(sourcePool.size())) return;
    if (!backend) return;

    PooledSource& source = sourcePool[index];

    // Calculate final volume: master * group * source * attenuation
    float masterVol = volumes[static_cast<int>(VolumeGroup::Master)];
    float groupVol = volumes[static_cast<int>(source.group)];
    float attenuation = source.is3D ? CalculateAttenuation(source.position, source.maxDistance, source.rolloff) : 1.0f;

    float finalVolume = masterVol * groupVol * source.volume * attenuation;
    finalVolume = std::max(0.0f, std::min(1.0f, finalVolume));

    backend->SetSourceVolume(source.backendId, finalVolume);
}

} // namespace vge
