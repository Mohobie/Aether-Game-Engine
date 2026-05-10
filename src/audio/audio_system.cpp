#include "audio_system.h"
#include "platform/file_system.h"
#include <iostream>
#include <cmath>
#include <cstring>
#include <algorithm>

namespace vge {

// ============================================
// Audio Clip Implementation
// ============================================

bool AudioClip::LoadFromFile(const std::string& path) {
    std::cout << "[Audio] Loading clip: " << path << std::endl;
    // In a real implementation, this would load WAV/OGG using a library
    // For now, generate a simple placeholder sound
    *this = CreateSineWave(440.0f, 1.0f);
    name = path;
    return true;
}

bool AudioClip::LoadFromMemory(const float* data, int sampleCount, int rate, int ch) {
    sampleRate = rate;
    channels = ch;
    samples.resize(sampleCount);
    std::memcpy(samples.data(), data, sampleCount * sizeof(float));
    duration = static_cast<float>(sampleCount / ch) / rate;
    return true;
}

AudioClip AudioClip::CreateSineWave(float frequency, float duration, float amplitude) {
    AudioClip clip;
    clip.sampleRate = 44100;
    clip.channels = 1;
    clip.duration = duration;
    
    int sampleCount = static_cast<int>(duration * clip.sampleRate);
    clip.samples.resize(sampleCount);
    
    for (int i = 0; i < sampleCount; ++i) {
        float t = static_cast<float>(i) / clip.sampleRate;
        clip.samples[i] = std::sin(2.0f * 3.14159f * frequency * t) * amplitude;
    }
    
    return clip;
}

AudioClip AudioClip::CreateSquareWave(float frequency, float duration, float amplitude) {
    AudioClip clip;
    clip.sampleRate = 44100;
    clip.channels = 1;
    clip.duration = duration;
    
    int sampleCount = static_cast<int>(duration * clip.sampleRate);
    clip.samples.resize(sampleCount);
    
    float period = 1.0f / frequency;
    for (int i = 0; i < sampleCount; ++i) {
        float t = static_cast<float>(i) / clip.sampleRate;
        float phase = t / period;
        clip.samples[i] = (phase - std::floor(phase) < 0.5f ? 1.0f : -1.0f) * amplitude;
    }
    
    return clip;
}

AudioClip AudioClip::CreateNoise(float duration, float amplitude) {
    AudioClip clip;
    clip.sampleRate = 44100;
    clip.channels = 1;
    clip.duration = duration;
    
    int sampleCount = static_cast<int>(duration * clip.sampleRate);
    clip.samples.resize(sampleCount);
    
    // Simple pseudo-random noise
    unsigned int seed = 12345;
    for (int i = 0; i < sampleCount; ++i) {
        seed = seed * 1103515245 + 12345;
        float value = static_cast<float>(seed & 0x7FFF) / 16384.0f - 0.5f;
        clip.samples[i] = value * amplitude;
    }
    
    return clip;
}

// ============================================
// Audio Source Implementation
// ============================================

void AudioSource::Play() {
    isPlaying = true;
    std::cout << "[Audio] Playing source at (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
}

void AudioSource::Stop() {
    isPlaying = false;
}

void AudioSource::Pause() {
    isPlaying = false;
}

void AudioSource::Resume() {
    isPlaying = true;
}

void AudioSource::SetClip(AudioClip* newClip) {
    clip = newClip;
}

float AudioSource::GetAttenuatedVolume(const Vec3& listenerPos) const {
    if (!is3D) return volume;
    
    Vec3 diff = position - listenerPos;
    float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
    
    if (dist >= maxDistance) return 0.0f;
    
    // Inverse square law with rolloff
    float attenuation = 1.0f / (1.0f + dist * rolloff);
    return volume * attenuation;
}

// ============================================
// Audio Listener Implementation
// ============================================

void AudioListener::SetOrientation(const Vec3& fwd, const Vec3& upVec) {
    forward = fwd.normalize();
    up = upVec.normalize();
}

// ============================================
// Audio System Implementation
// ============================================

AudioSystem::AudioSystem() : initialized(false), masterVolume(1.0f) {}

AudioSystem::~AudioSystem() {
    Shutdown();
}

bool AudioSystem::Initialize() {
    std::cout << "[Audio] Initializing audio system" << std::endl;
    initialized = true;
    return true;
}

void AudioSystem::Shutdown() {
    Clear();
    initialized = false;
}

AudioClip* AudioSystem::LoadClip(const std::string& name, const std::string& path) {
    std::string fullPath = "assets/audio/" + path;
    
    AudioClip clip;
    if (clip.LoadFromFile(fullPath)) {
        clip.name = name;
        clips[name] = std::move(clip);
        return &clips[name];
    }
    
    return nullptr;
}

AudioClip* AudioSystem::GetClip(const std::string& name) {
    auto it = clips.find(name);
    if (it != clips.end()) {
        return &it->second;
    }
    return nullptr;
}

void AudioSystem::UnloadClip(const std::string& name) {
    clips.erase(name);
}

AudioSource* AudioSystem::CreateSource() {
    auto source = std::make_unique<AudioSource>();
    AudioSource* ptr = source.get();
    sources.push_back(std::move(source));
    return ptr;
}

void AudioSystem::DestroySource(AudioSource* source) {
    auto it = std::find_if(sources.begin(), sources.end(),
        [source](const std::unique_ptr<AudioSource>& s) { return s.get() == source; });
    if (it != sources.end()) {
        sources.erase(it);
    }
}

void AudioSystem::PlayOneShot(const std::string& clipName, const Vec3& position) {
    AudioClip* clip = GetClip(clipName);
    if (clip) {
        PlayOneShot(clip, position);
    }
}

void AudioSystem::PlayOneShot(AudioClip* clip, const Vec3& position) {
    if (!clip) return;
    
    AudioSource* source = CreateSource();
    source->SetClip(clip);
    source->position = position;
    source->is3D = true;
    source->loop = false;
    source->Play();
    
    // Auto-destroy after playback (in a real system, this would be handled by the audio thread)
    std::cout << "[Audio] One-shot: " << clip->name << " at (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
}

void AudioSystem::SetListenerOrientation(const Vec3& forward, const Vec3& up) {
    listener.SetOrientation(forward, up);
}

void AudioSystem::Update(float deltaTime) {
    // In a real implementation, this would:
    // 1. Mix all active sources
    // 2. Apply 3D positioning
    // 3. Send to audio hardware
    
    // For now, just clean up finished one-shot sources
    auto newEnd = std::remove_if(sources.begin(), sources.end(),
        [](const std::unique_ptr<AudioSource>& source) {
            return !source->isPlaying && !source->loop;
        });
    sources.erase(newEnd, sources.end());
}

size_t AudioSystem::GetActiveSourceCount() const {
    size_t count = 0;
    for (const auto& source : sources) {
        if (source->isPlaying) {
            ++count;
        }
    }
    return count;
}

void AudioSystem::Clear() {
    clips.clear();
    sources.clear();
}

} // namespace vge