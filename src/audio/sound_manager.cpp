#include "sound_manager.h"
#include <iostream>
#include <cmath>
#include <algorithm>

namespace vge {

// ============================================================
// MusicPlaylist
// ============================================================

MusicPlaylist::MusicPlaylist() : rng(std::random_device{}()) {}

void MusicPlaylist::AddTrack(const std::string& path, const std::string& name) {
    Track track;
    track.path = path;
    track.name = name.empty() ? path : name;
    tracks.push_back(track);
}

void MusicPlaylist::RemoveTrack(size_t index) {
    if (index < tracks.size()) {
        tracks.erase(tracks.begin() + index);
    }
}

void MusicPlaylist::Clear() {
    tracks.clear();
    currentTrack = 0;
    isPlaying = false;
}

void MusicPlaylist::Play(size_t index) {
    if (tracks.empty()) return;
    if (index >= tracks.size()) index = 0;

    // Crossfade if already playing
    if (isPlaying && activeSource >= 0 && audioEngine) {
        fadingSource = activeSource;
        fadeTimer = crossfadeDuration;
    }

    StartTrack(index);
    isPlaying = true;
    isPaused = false;
}

void MusicPlaylist::Stop() {
    if (audioEngine) {
        if (activeSource >= 0) {
            audioEngine->Stop(activeSource);
            activeSource = -1;
        }
        if (fadingSource >= 0) {
            audioEngine->Stop(fadingSource);
            fadingSource = -1;
        }
    }
    isPlaying = false;
    isPaused = false;
    fadeTimer = 0.0f;
}

void MusicPlaylist::Pause() {
    if (!isPlaying || isPaused) return;
    if (audioEngine && activeSource >= 0) {
        audioEngine->Pause(activeSource);
    }
    isPaused = true;
}

void MusicPlaylist::Resume() {
    if (!isPlaying || !isPaused) return;
    if (audioEngine && activeSource >= 0) {
        audioEngine->Resume(activeSource);
    }
    isPaused = false;
}

void MusicPlaylist::Next() {
    if (tracks.empty()) return;

    size_t nextTrack;
    switch (playMode) {
        case PlaylistMode::Sequential:
        case PlaylistMode::LoopOne:
            nextTrack = (currentTrack + 1) % tracks.size();
            break;
        case PlaylistMode::Shuffle:
        case PlaylistMode::Random:
            if (tracks.size() > 1) {
                std::uniform_int_distribution<size_t> dist(0, tracks.size() - 1);
                do {
                    nextTrack = dist(rng);
                } while (nextTrack == currentTrack);
            } else {
                nextTrack = 0;
            }
            break;
        default:
            nextTrack = (currentTrack + 1) % tracks.size();
    }

    Play(nextTrack);
}

void MusicPlaylist::Previous() {
    if (tracks.empty()) return;

    size_t prevTrack;
    if (currentTrack == 0) {
        prevTrack = tracks.size() - 1;
    } else {
        prevTrack = currentTrack - 1;
    }

    Play(prevTrack);
}

void MusicPlaylist::SetVolume(float vol) {
    volume = std::max(0.0f, std::min(1.0f, vol));
    if (audioEngine && activeSource >= 0) {
        audioEngine->SetSourceVolume(activeSource, volume);
    }
}

bool MusicPlaylist::IsPlaying() const {
    return isPlaying && !isPaused;
}

std::string MusicPlaylist::GetCurrentTrackName() const {
    if (currentTrack < tracks.size()) {
        return tracks[currentTrack].name;
    }
    return "";
}

void MusicPlaylist::Update(float deltaTime) {
    if (!isPlaying || tracks.empty()) return;

    // Update crossfade
    if (fadingSource >= 0) {
        UpdateCrossfade(deltaTime);
    }

    // Check if current track finished (non-looping)
    if (audioEngine && activeSource >= 0 && !isPaused) {
        if (!audioEngine->IsPlaying(activeSource) && playMode != PlaylistMode::LoopOne) {
            Next();
        }
    }
}

void MusicPlaylist::StartTrack(size_t index) {
    if (!audioEngine || index >= tracks.size()) return;

    Track& track = tracks[index];

    // Load sound if not already loaded
    if (track.soundHandle < 0) {
        track.soundHandle = audioEngine->LoadSound(track.path);
    }

    if (track.soundHandle >= 0) {
        activeSource = audioEngine->Play2D(track.soundHandle, VolumeGroup::Music,
                                           volume, 1.0f, true, 10);
        currentTrack = index;
        std::cout << "[MusicPlaylist] Now playing: " << track.name << std::endl;
    }
}

void MusicPlaylist::UpdateCrossfade(float deltaTime) {
    if (fadeTimer > 0.0f && fadingSource >= 0 && audioEngine) {
        fadeTimer -= deltaTime;
        float fadeVolume = (fadeTimer / crossfadeDuration) * volume;
        audioEngine->SetSourceVolume(fadingSource, std::max(0.0f, fadeVolume));

        if (fadeTimer <= 0.0f) {
            audioEngine->Stop(fadingSource);
            fadingSource = -1;
        }
    }
}

// ============================================================
// SoundManager
// ============================================================

SoundManager::SoundManager() {}

SoundManager::~SoundManager() {
    if (initialized) {
        Shutdown();
    }
}

bool SoundManager::Initialize(AudioEngine* engine) {
    if (!engine || !engine->IsInitialized()) {
        std::cerr << "[SoundManager] Audio engine not initialized" << std::endl;
        return false;
    }

    audioEngine = engine;
    musicPlaylist.audioEngine = engine;

    // Load default sound pack
    soundPacks["default"] = CreateDefaultSoundPack();
    activePackName = "default";

    initialized = true;
    std::cout << "[SoundManager] Initialized with default sound pack" << std::endl;
    return true;
}

void SoundManager::Shutdown() {
    if (!initialized) return;

    // Stop all sounds
    StopAllEvents();
    musicPlaylist.Stop();

    // Clear ambient zones
    ClearAmbientZones();

    // Clear packs
    soundPacks.clear();

    audioEngine = nullptr;
    initialized = false;

    std::cout << "[SoundManager] Shutdown complete" << std::endl;
}

bool SoundManager::IsInitialized() const {
    return initialized && audioEngine && audioEngine->IsInitialized();
}

// ============================================================
// Sound Packs
// ============================================================

void SoundManager::LoadSoundPack(const SoundPack& pack) {
    soundPacks[pack.GetName()] = pack;
    std::cout << "[SoundManager] Loaded sound pack: " << pack.GetName() << std::endl;
}

void SoundManager::LoadSoundPack(const std::string& packName) {
    SoundPack pack(packName);
    std::string path = "assets/audio/packs/" + packName + ".json";
    if (pack.LoadFromFile(path)) {
        soundPacks[packName] = std::move(pack);
        std::cout << "[SoundManager] Loaded sound pack from file: " << packName << std::endl;
    } else {
        std::cerr << "[SoundManager] Failed to load sound pack: " << packName << std::endl;
    }
}

void SoundManager::SetActivePack(const std::string& packName) {
    if (soundPacks.find(packName) != soundPacks.end()) {
        activePackName = packName;
        std::cout << "[SoundManager] Active pack set to: " << packName << std::endl;
    } else {
        std::cerr << "[SoundManager] Sound pack not found: " << packName << std::endl;
    }
}

const SoundPack* SoundManager::GetActivePack() const {
    auto it = soundPacks.find(activePackName);
    if (it != soundPacks.end()) {
        return &it->second;
    }
    return nullptr;
}

const SoundPack* SoundManager::GetPack(const std::string& packName) const {
    auto it = soundPacks.find(packName);
    if (it != soundPacks.end()) {
        return &it->second;
    }
    return nullptr;
}

// ============================================================
// Event-Driven Playback
// ============================================================

int SoundManager::PlayEvent(AudioEventType event, const Vec3& position,
                            float volumeMultiplier, float pitchMultiplier) {
    if (!initialized || !audioEngine) return -1;

    const SoundPack* pack = GetActivePack();
    if (!pack) return -1;

    const EventSoundSet* soundSet = pack->GetEventSounds(event);
    if (!soundSet || soundSet->sounds.empty()) {
        std::cerr << "[SoundManager] No sounds registered for event: "
                  << AudioEventTypeToString(event) << std::endl;
        return -1;
    }

    const SoundEntry* entry = soundSet->PickRandom();
    if (!entry) return -1;

    int soundHandle = LoadSoundForEvent(*entry);
    if (soundHandle < 0) return -1;

    // Calculate pitch with variation
    float pitch = pitchMultiplier;
    if (soundSet->randomizePitch) {
        pitch *= RandomPitch(1.0f - soundSet->pitchVariation, 1.0f + soundSet->pitchVariation);
    } else if (entry->pitchMin != entry->pitchMax) {
        pitch *= RandomPitch(entry->pitchMin, entry->pitchMax);
    }

    float volume = entry->volume * volumeMultiplier * soundSet->defaultVolume;

    // Determine volume group based on event type
    VolumeGroup group = VolumeGroup::SFX;
    if (event == AudioEventType::UIClick || event == AudioEventType::UIHover ||
        event == AudioEventType::UIConfirm || event == AudioEventType::UICancel ||
        event == AudioEventType::UIError || event == AudioEventType::UISlider) {
        group = VolumeGroup::UI;
    } else if (event == AudioEventType::AmbientForest || event == AudioEventType::AmbientCave ||
               event == AudioEventType::AmbientOcean || event == AudioEventType::AmbientWind ||
               event == AudioEventType::AmbientCity || event == AudioEventType::AmbientDungeon) {
        group = VolumeGroup::Ambient;
    }

    int sourceIndex = audioEngine->Play3D(soundHandle, position, group, volume, pitch,
                                          false, entry->priority);

    if (sourceIndex >= 0) {
        activeEventSources.push_back(sourceIndex);
        std::cout << "[SoundManager] Played event: " << AudioEventTypeToString(event)
                  << " at (" << position.x << ", " << position.y << ", " << position.z << ")"
                  << " vol=" << volume << " pitch=" << pitch << std::endl;
    }

    return sourceIndex;
}

int SoundManager::PlayEvent2D(AudioEventType event, float volumeMultiplier) {
    if (!initialized || !audioEngine) return -1;

    const SoundPack* pack = GetActivePack();
    if (!pack) return -1;

    const EventSoundSet* soundSet = pack->GetEventSounds(event);
    if (!soundSet || soundSet->sounds.empty()) {
        std::cerr << "[SoundManager] No sounds registered for event: "
                  << AudioEventTypeToString(event) << std::endl;
        return -1;
    }

    const SoundEntry* entry = soundSet->PickRandom();
    if (!entry) return -1;

    int soundHandle = LoadSoundForEvent(*entry);
    if (soundHandle < 0) return -1;

    float pitch = 1.0f;
    if (soundSet->randomizePitch) {
        pitch = RandomPitch(1.0f - soundSet->pitchVariation, 1.0f + soundSet->pitchVariation);
    } else if (entry->pitchMin != entry->pitchMax) {
        pitch = RandomPitch(entry->pitchMin, entry->pitchMax);
    }

    float volume = entry->volume * volumeMultiplier * soundSet->defaultVolume;

    VolumeGroup group = VolumeGroup::SFX;
    if (event == AudioEventType::UIClick || event == AudioEventType::UIHover ||
        event == AudioEventType::UIConfirm || event == AudioEventType::UICancel ||
        event == AudioEventType::UIError || event == AudioEventType::UISlider) {
        group = VolumeGroup::UI;
    }

    int sourceIndex = audioEngine->Play2D(soundHandle, group, volume, pitch, false, entry->priority);

    if (sourceIndex >= 0) {
        activeEventSources.push_back(sourceIndex);
        std::cout << "[SoundManager] Played 2D event: " << AudioEventTypeToString(event)
                  << " vol=" << volume << " pitch=" << pitch << std::endl;
    }

    return sourceIndex;
}

int SoundManager::PlayCustomEvent(const std::string& eventName, const Vec3& position,
                                  float volumeMultiplier) {
    if (!initialized || !audioEngine) return -1;

    const SoundPack* pack = GetActivePack();
    if (!pack) return -1;

    const EventSoundSet* soundSet = pack->GetCustomEventSounds(eventName);
    if (!soundSet || soundSet->sounds.empty()) {
        std::cerr << "[SoundManager] No sounds registered for custom event: " << eventName << std::endl;
        return -1;
    }

    const SoundEntry* entry = soundSet->PickRandom();
    if (!entry) return -1;

    int soundHandle = LoadSoundForEvent(*entry);
    if (soundHandle < 0) return -1;

    float volume = entry->volume * volumeMultiplier;
    int sourceIndex = audioEngine->Play3D(soundHandle, position, VolumeGroup::SFX,
                                          volume, 1.0f, false, entry->priority);

    if (sourceIndex >= 0) {
        activeEventSources.push_back(sourceIndex);
    }

    return sourceIndex;
}

// ============================================================
// Direct Sound Playback
// ============================================================

int SoundManager::PlaySound(const std::string& filePath, const Vec3& position,
                            VolumeGroup group, float volume, bool loop) {
    if (!initialized || !audioEngine) return -1;

    int soundHandle = audioEngine->LoadSound(filePath);
    if (soundHandle < 0) return -1;

    int sourceIndex = audioEngine->Play3D(soundHandle, position, group, volume, 1.0f,
                                          loop, 0);

    if (sourceIndex >= 0) {
        activeEventSources.push_back(sourceIndex);
    }

    return sourceIndex;
}

int SoundManager::PlaySound2D(const std::string& filePath, VolumeGroup group,
                              float volume, bool loop) {
    if (!initialized || !audioEngine) return -1;

    int soundHandle = audioEngine->LoadSound(filePath);
    if (soundHandle < 0) return -1;

    int sourceIndex = audioEngine->Play2D(soundHandle, group, volume, 1.0f, loop, 0);

    if (sourceIndex >= 0) {
        activeEventSources.push_back(sourceIndex);
    }

    return sourceIndex;
}

void SoundManager::StopEvent(int sourceIndex) {
    if (!audioEngine) return;

    audioEngine->Stop(sourceIndex);

    auto it = std::find(activeEventSources.begin(), activeEventSources.end(), sourceIndex);
    if (it != activeEventSources.end()) {
        activeEventSources.erase(it);
    }
}

void SoundManager::StopAllEvents() {
    if (!audioEngine) return;

    for (int sourceIndex : activeEventSources) {
        audioEngine->Stop(sourceIndex);
    }
    activeEventSources.clear();
}

void SoundManager::StopAllByGroup(VolumeGroup group) {
    if (!audioEngine) return;
    audioEngine->StopGroup(group);

    // Remove tracked sources in this group (simplified - we don't track group per source)
    // In a full implementation, we'd check each source's group
}

// ============================================================
// Music
// ============================================================

void SoundManager::PlayMusic(const std::string& trackPath) {
    musicPlaylist.Clear();
    musicPlaylist.AddTrack(trackPath);
    musicPlaylist.Play(0);
}

void SoundManager::StopMusic() {
    musicPlaylist.Stop();
}

void SoundManager::SetMusicVolume(float volume) {
    musicPlaylist.SetVolume(volume);
}

// ============================================================
// Ambient Zones
// ============================================================

AmbientZone* SoundManager::AddAmbientZone(const std::string& name, const Vec3& center,
                                           float radius, AudioEventType ambientType) {
    auto zone = std::make_unique<AmbientZone>();
    zone->name = name;
    zone->center = center;
    zone->radius = radius;
    zone->ambientType = ambientType;

    AmbientZone* ptr = zone.get();
    ambientZones.push_back(std::move(zone));
    return ptr;
}

void SoundManager::RemoveAmbientZone(const std::string& name) {
    auto it = std::remove_if(ambientZones.begin(), ambientZones.end(),
        [&name](const std::unique_ptr<AmbientZone>& zone) {
            return zone->name == name;
        });
    ambientZones.erase(it, ambientZones.end());
}

void SoundManager::UpdateAmbientZones(const Vec3& listenerPos) {
    if (!initialized || !audioEngine) return;

    const SoundPack* pack = GetActivePack();
    if (!pack) return;

    for (auto& zone : ambientZones) {
        // Calculate distance to zone center
        Vec3 diff = listenerPos - zone->center;
        float dist = diff.length();

        // Calculate target volume based on distance
        float targetVolume = 0.0f;
        if (dist < zone->radius) {
            targetVolume = 1.0f;
        } else if (dist < zone->radius + zone->fadeDistance) {
            targetVolume = 1.0f - (dist - zone->radius) / zone->fadeDistance;
        }
        targetVolume *= zone->volume;

        // Start/stop ambient sound
        if (targetVolume > 0.01f && !zone->isActive) {
            // Start ambient sound
            const EventSoundSet* soundSet = pack->GetEventSounds(zone->ambientType);
            if (soundSet && !soundSet->sounds.empty()) {
                const SoundEntry* entry = soundSet->PickRandom();
                if (entry) {
                    int soundHandle = LoadSoundForEvent(*entry);
                    if (soundHandle >= 0) {
                        zone->sourceIndex = audioEngine->Play3D(soundHandle, zone->center,
                                                                 VolumeGroup::Ambient,
                                                                 targetVolume, 1.0f, true, 0);
                        zone->isActive = true;
                        zone->currentVolume = targetVolume;
                    }
                }
            }
        } else if (targetVolume <= 0.01f && zone->isActive) {
            // Stop ambient sound
            if (zone->sourceIndex >= 0) {
                audioEngine->Stop(zone->sourceIndex);
                zone->sourceIndex = -1;
            }
            zone->isActive = false;
            zone->currentVolume = 0.0f;
        } else if (zone->isActive && zone->sourceIndex >= 0) {
            // Update volume
            if (std::abs(targetVolume - zone->currentVolume) > 0.01f) {
                audioEngine->SetSourceVolume(zone->sourceIndex, targetVolume);
                zone->currentVolume = targetVolume;
            }
        }
    }
}

void SoundManager::ClearAmbientZones() {
    for (auto& zone : ambientZones) {
        if (zone->sourceIndex >= 0 && audioEngine) {
            audioEngine->Stop(zone->sourceIndex);
        }
    }
    ambientZones.clear();
}

// ============================================================
// Volume & Listener
// ============================================================

void SoundManager::SetVolume(VolumeGroup group, float volume) {
    if (audioEngine) {
        audioEngine->SetVolume(group, volume);
    }
}

float SoundManager::GetVolume(VolumeGroup group) const {
    if (audioEngine) {
        return audioEngine->GetVolume(group);
    }
    return 1.0f;
}

void SoundManager::SetMasterVolume(float volume) {
    if (audioEngine) {
        audioEngine->SetMasterVolume(volume);
    }
}

void SoundManager::SetListenerPosition(const Vec3& pos) {
    if (audioEngine) {
        audioEngine->SetListenerPosition(pos);
    }
}

void SoundManager::SetListenerOrientation(const Vec3& forward, const Vec3& up) {
    if (audioEngine) {
        audioEngine->SetListenerOrientation(forward, up);
    }
}

// ============================================================
// Update
// ============================================================

void SoundManager::Update(float deltaTime) {
    if (!initialized) return;

    // Update audio engine
    if (audioEngine) {
        audioEngine->Update(deltaTime);
    }

    // Update music playlist
    musicPlaylist.Update(deltaTime);

    // Cleanup finished sources
    CleanupFinishedSources();
}

// ============================================================
// Stats
// ============================================================

size_t SoundManager::GetActiveSoundCount() const {
    if (!audioEngine) return 0;
    return audioEngine->GetActiveSourceCount();
}

// ============================================================
// Internal Helpers
// ============================================================

int SoundManager::LoadSoundForEvent(const SoundEntry& entry) {
    if (!audioEngine) return -1;

    std::string fullPath = "assets/audio/" + entry.filePath;
    return audioEngine->LoadSound(fullPath);
}

float SoundManager::RandomPitch(float minPitch, float maxPitch) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(minPitch, maxPitch);
    return dist(rng);
}

void SoundManager::CleanupFinishedSources() {
    auto it = std::remove_if(activeEventSources.begin(), activeEventSources.end(),
        [this](int sourceIndex) {
            return !audioEngine || !audioEngine->IsPlaying(sourceIndex);
        });
    activeEventSources.erase(it, activeEventSources.end());
}

} // namespace vge
