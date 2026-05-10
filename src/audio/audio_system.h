#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include "math/vec3.h"

namespace vge {

// Audio clip data
struct AudioClip {
    std::string name;
    std::vector<float> samples;
    int sampleRate;
    int channels;
    float duration;
    
    AudioClip() : sampleRate(44100), channels(2), duration(0) {}
    
    bool LoadFromFile(const std::string& path);
    bool LoadFromMemory(const float* data, int sampleCount, int rate, int ch);
    
    // Generate simple sounds
    static AudioClip CreateSineWave(float frequency, float duration, float amplitude = 0.5f);
    static AudioClip CreateSquareWave(float frequency, float duration, float amplitude = 0.5f);
    static AudioClip CreateNoise(float duration, float amplitude = 0.5f);
};

// Audio source (plays sounds in 3D space)
class AudioSource {
public:
    Vec3 position;
    float volume;
    float pitch;
    bool loop;
    bool isPlaying;
    bool is3D;
    float maxDistance;
    float rolloff;
    
    AudioClip* clip;
    
    AudioSource() 
        : position(0, 0, 0)
        , volume(1.0f)
        , pitch(1.0f)
        , loop(false)
        , isPlaying(false)
        , is3D(true)
        , maxDistance(100.0f)
        , rolloff(1.0f)
        , clip(nullptr)
    {}
    
    void Play();
    void Stop();
    void Pause();
    void Resume();
    
    // Set clip to play
    void SetClip(AudioClip* newClip);
    
    // Update 3D positioning
    void UpdatePosition(const Vec3& pos) { position = pos; }
    
    // Get volume with 3D attenuation
    float GetAttenuatedVolume(const Vec3& listenerPos) const;
};

// Audio listener (usually attached to camera/player)
class AudioListener {
public:
    Vec3 position;
    Vec3 forward;
    Vec3 up;
    float globalVolume;
    
    AudioListener() 
        : position(0, 0, 0)
        , forward(0, 0, 1)
        , up(0, 1, 0)
        , globalVolume(1.0f)
    {}
    
    void SetPosition(const Vec3& pos) { position = pos; }
    void SetOrientation(const Vec3& fwd, const Vec3& upVec);
};

// Audio manager
class AudioSystem {
private:
    std::unordered_map<std::string, AudioClip> clips;
    std::vector<std::unique_ptr<AudioSource>> sources;
    AudioListener listener;
    
    bool initialized;
    float masterVolume;
    
public:
    AudioSystem();
    ~AudioSystem();
    
    // Initialization
    bool Initialize();
    void Shutdown();
    
    // Clip management
    AudioClip* LoadClip(const std::string& name, const std::string& path);
    AudioClip* GetClip(const std::string& name);
    void UnloadClip(const std::string& name);
    
    // Source management
    AudioSource* CreateSource();
    void DestroySource(AudioSource* source);
    
    // Playback
    void PlayOneShot(const std::string& clipName, const Vec3& position = Vec3(0,0,0));
    void PlayOneShot(AudioClip* clip, const Vec3& position = Vec3(0,0,0));
    
    // Listener
    void SetListenerPosition(const Vec3& pos) { listener.SetPosition(pos); }
    void SetListenerOrientation(const Vec3& forward, const Vec3& up);
    AudioListener* GetListener() { return &listener; }
    
    // Volume
    void SetMasterVolume(float vol) { masterVolume = vol; }
    float GetMasterVolume() const { return masterVolume; }
    
    // Update (process audio)
    void Update(float deltaTime);
    
    // Stats
    size_t GetClipCount() const { return clips.size(); }
    size_t GetActiveSourceCount() const;
    
    void Clear();
};

} // namespace vge