#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace vge {

// ============================================================
// Audio Events - Decoupled, Game-Agnostic Sound Event System
// ============================================================
// Games register their own sound files for each event type.
// No hardcoded "zombie groan" or "minecraft block break".

enum class AudioEventType {
    // Movement
    Footstep,
    Jump,
    Land,
    Swim,
    Climb,

    // Combat
    Attack,
    AttackSwing,      // Weapon swing (miss)
    AttackHit,        // Weapon hit target
    Hurt,
    Die,
    Block,
    Parry,

    // World Interaction
    Interact,
    Use,
    Pickup,
    Drop,
    Open,
    Close,
    Craft,

    // Explosions / Effects
    Explosion,
    Fire,
    Splash,
    WeatherRain,
    WeatherThunder,

    // UI
    UIClick,
    UIHover,
    UIConfirm,
    UICancel,
    UIError,
    UISlider,

    // Ambient (loops / zones)
    AmbientForest,
    AmbientCave,
    AmbientOcean,
    AmbientWind,
    AmbientCity,
    AmbientDungeon,

    // Music triggers
    MusicExplore,
    MusicCombat,
    MusicBoss,
    MusicVictory,
    MusicDefeat,
    MusicMenu,

    // Custom (games can define their own)
    Custom,

    Count
};

// String conversion helpers
std::string AudioEventTypeToString(AudioEventType type);
AudioEventType StringToAudioEventType(const std::string& str);

// A single sound entry for an event (supports randomization)
struct SoundEntry {
    std::string filePath;       // Relative to assets/audio/
    float volume = 1.0f;        // Per-sound volume multiplier
    float pitchMin = 1.0f;      // Random pitch range
    float pitchMax = 1.0f;
    float probability = 1.0f;   // For weighted random selection
    int priority = 0;           // Higher = more important

    SoundEntry() = default;
    SoundEntry(const std::string& path, float vol = 1.0f, float pMin = 1.0f, float pMax = 1.0f)
        : filePath(path), volume(vol), pitchMin(pMin), pitchMax(pMax), probability(1.0f), priority(0) {}
};

// All sounds registered for a single event type
struct EventSoundSet {
    AudioEventType eventType;
    std::vector<SoundEntry> sounds;
    float defaultVolume = 1.0f;
    bool randomizePitch = false;
    float pitchVariation = 0.1f;  // +/- variation when randomizePitch is true

    EventSoundSet() : eventType(AudioEventType::Custom) {}
    explicit EventSoundSet(AudioEventType type) : eventType(type) {}

    void AddSound(const SoundEntry& entry) { sounds.push_back(entry); }
    void AddSound(const std::string& path, float vol = 1.0f, float pMin = 1.0f, float pMax = 1.0f) {
        sounds.emplace_back(path, vol, pMin, pMax);
    }

    // Pick a random sound from this set (weighted by probability)
    const SoundEntry* PickRandom() const;
};

// Complete sound pack for a game
class SoundPack {
private:
    std::string name;
    std::unordered_map<AudioEventType, EventSoundSet> events;
    std::unordered_map<std::string, EventSoundSet> customEvents;

public:
    explicit SoundPack(const std::string& packName = "default") : name(packName) {}

    // Register sounds for a standard event
    void RegisterEvent(AudioEventType type, const EventSoundSet& soundSet);
    void RegisterEvent(AudioEventType type, const std::string& filePath, float volume = 1.0f);

    // Register custom events (for game-specific sounds)
    void RegisterCustomEvent(const std::string& eventName, const EventSoundSet& soundSet);
    void RegisterCustomEvent(const std::string& eventName, const std::string& filePath, float volume = 1.0f);

    // Get sounds for an event
    const EventSoundSet* GetEventSounds(AudioEventType type) const;
    const EventSoundSet* GetCustomEventSounds(const std::string& eventName) const;

    // Check if event is registered
    bool HasEvent(AudioEventType type) const;
    bool HasCustomEvent(const std::string& eventName) const;

    // Load from JSON config file (assets/audio/packs/<pack_name>.json)
    bool LoadFromFile(const std::string& path);

    // Save to JSON config file
    bool SaveToFile(const std::string& path) const;

    const std::string& GetName() const { return name; }
    void SetName(const std::string& newName) { name = newName; }

    // Get all registered event types
    std::vector<AudioEventType> GetRegisteredEvents() const;
};

// Default sound pack with sensible fallback entries
SoundPack CreateDefaultSoundPack();

} // namespace vge
