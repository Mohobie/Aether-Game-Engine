#include "audio_events.h"
#include <cstdlib>
#include <iostream>

namespace vge {

// ============================================================
// AudioEventType String Conversion
// ============================================================

std::string AudioEventTypeToString(AudioEventType type) {
    switch (type) {
        case AudioEventType::Footstep:       return "footstep";
        case AudioEventType::Jump:           return "jump";
        case AudioEventType::Land:           return "land";
        case AudioEventType::Swim:           return "swim";
        case AudioEventType::Climb:          return "climb";
        case AudioEventType::Attack:         return "attack";
        case AudioEventType::AttackSwing:    return "attack_swing";
        case AudioEventType::AttackHit:      return "attack_hit";
        case AudioEventType::Hurt:           return "hurt";
        case AudioEventType::Die:            return "die";
        case AudioEventType::Block:          return "block";
        case AudioEventType::Parry:          return "parry";
        case AudioEventType::Interact:       return "interact";
        case AudioEventType::Use:            return "use";
        case AudioEventType::Pickup:         return "pickup";
        case AudioEventType::Drop:           return "drop";
        case AudioEventType::Open:           return "open";
        case AudioEventType::Close:          return "close";
        case AudioEventType::Craft:          return "craft";
        case AudioEventType::Explosion:      return "explosion";
        case AudioEventType::Fire:           return "fire";
        case AudioEventType::Splash:         return "splash";
        case AudioEventType::WeatherRain:    return "weather_rain";
        case AudioEventType::WeatherThunder: return "weather_thunder";
        case AudioEventType::UIClick:        return "ui_click";
        case AudioEventType::UIHover:        return "ui_hover";
        case AudioEventType::UIConfirm:      return "ui_confirm";
        case AudioEventType::UICancel:       return "ui_cancel";
        case AudioEventType::UIError:        return "ui_error";
        case AudioEventType::UISlider:       return "ui_slider";
        case AudioEventType::AmbientForest:  return "ambient_forest";
        case AudioEventType::AmbientCave:    return "ambient_cave";
        case AudioEventType::AmbientOcean:   return "ambient_ocean";
        case AudioEventType::AmbientWind:    return "ambient_wind";
        case AudioEventType::AmbientCity:    return "ambient_city";
        case AudioEventType::AmbientDungeon: return "ambient_dungeon";
        case AudioEventType::MusicExplore:   return "music_explore";
        case AudioEventType::MusicCombat:    return "music_combat";
        case AudioEventType::MusicBoss:      return "music_boss";
        case AudioEventType::MusicVictory:   return "music_victory";
        case AudioEventType::MusicDefeat:    return "music_defeat";
        case AudioEventType::MusicMenu:      return "music_menu";
        case AudioEventType::Custom:         return "custom";
        default:                             return "unknown";
    }
}

AudioEventType StringToAudioEventType(const std::string& str) {
    if (str == "footstep")       return AudioEventType::Footstep;
    if (str == "jump")           return AudioEventType::Jump;
    if (str == "land")           return AudioEventType::Land;
    if (str == "swim")           return AudioEventType::Swim;
    if (str == "climb")          return AudioEventType::Climb;
    if (str == "attack")         return AudioEventType::Attack;
    if (str == "attack_swing")   return AudioEventType::AttackSwing;
    if (str == "attack_hit")     return AudioEventType::AttackHit;
    if (str == "hurt")           return AudioEventType::Hurt;
    if (str == "die")            return AudioEventType::Die;
    if (str == "block")          return AudioEventType::Block;
    if (str == "parry")          return AudioEventType::Parry;
    if (str == "interact")       return AudioEventType::Interact;
    if (str == "use")            return AudioEventType::Use;
    if (str == "pickup")         return AudioEventType::Pickup;
    if (str == "drop")           return AudioEventType::Drop;
    if (str == "open")           return AudioEventType::Open;
    if (str == "close")          return AudioEventType::Close;
    if (str == "craft")          return AudioEventType::Craft;
    if (str == "explosion")      return AudioEventType::Explosion;
    if (str == "fire")           return AudioEventType::Fire;
    if (str == "splash")         return AudioEventType::Splash;
    if (str == "weather_rain")   return AudioEventType::WeatherRain;
    if (str == "weather_thunder")return AudioEventType::WeatherThunder;
    if (str == "ui_click")       return AudioEventType::UIClick;
    if (str == "ui_hover")       return AudioEventType::UIHover;
    if (str == "ui_confirm")     return AudioEventType::UIConfirm;
    if (str == "ui_cancel")      return AudioEventType::UICancel;
    if (str == "ui_error")       return AudioEventType::UIError;
    if (str == "ui_slider")      return AudioEventType::UISlider;
    if (str == "ambient_forest") return AudioEventType::AmbientForest;
    if (str == "ambient_cave")   return AudioEventType::AmbientCave;
    if (str == "ambient_ocean")  return AudioEventType::AmbientOcean;
    if (str == "ambient_wind")   return AudioEventType::AmbientWind;
    if (str == "ambient_city")   return AudioEventType::AmbientCity;
    if (str == "ambient_dungeon")return AudioEventType::AmbientDungeon;
    if (str == "music_explore")  return AudioEventType::MusicExplore;
    if (str == "music_combat")   return AudioEventType::MusicCombat;
    if (str == "music_boss")     return AudioEventType::MusicBoss;
    if (str == "music_victory")  return AudioEventType::MusicVictory;
    if (str == "music_defeat")   return AudioEventType::MusicDefeat;
    if (str == "music_menu")     return AudioEventType::MusicMenu;
    return AudioEventType::Custom;
}

// ============================================================
// EventSoundSet
// ============================================================

const SoundEntry* EventSoundSet::PickRandom() const {
    if (sounds.empty()) return nullptr;
    if (sounds.size() == 1) return &sounds[0];

    // Simple weighted random
    float totalProb = 0.0f;
    for (const auto& s : sounds) {
        totalProb += s.probability;
    }

    float pick = static_cast<float>(rand()) / RAND_MAX * totalProb;
    float accum = 0.0f;
    for (const auto& s : sounds) {
        accum += s.probability;
        if (pick <= accum) {
            return &s;
        }
    }
    return &sounds.back();
}

// ============================================================
// SoundPack
// ============================================================

void SoundPack::RegisterEvent(AudioEventType type, const EventSoundSet& soundSet) {
    events[type] = soundSet;
}

void SoundPack::RegisterEvent(AudioEventType type, const std::string& filePath, float volume) {
    EventSoundSet set(type);
    set.AddSound(filePath, volume);
    events[type] = std::move(set);
}

void SoundPack::RegisterCustomEvent(const std::string& eventName, const EventSoundSet& soundSet) {
    customEvents[eventName] = soundSet;
}

void SoundPack::RegisterCustomEvent(const std::string& eventName, const std::string& filePath, float volume) {
    EventSoundSet set(AudioEventType::Custom);
    set.AddSound(filePath, volume);
    customEvents[eventName] = std::move(set);
}

const EventSoundSet* SoundPack::GetEventSounds(AudioEventType type) const {
    auto it = events.find(type);
    if (it != events.end()) {
        return &it->second;
    }
    return nullptr;
}

const EventSoundSet* SoundPack::GetCustomEventSounds(const std::string& eventName) const {
    auto it = customEvents.find(eventName);
    if (it != customEvents.end()) {
        return &it->second;
    }
    return nullptr;
}

bool SoundPack::HasEvent(AudioEventType type) const {
    return events.find(type) != events.end();
}

bool SoundPack::HasCustomEvent(const std::string& eventName) const {
    return customEvents.find(eventName) != customEvents.end();
}

bool SoundPack::LoadFromFile(const std::string& path) {
    // TODO: Implement JSON parsing when JSON library is available
    std::cout << "[SoundPack] LoadFromFile not yet implemented (needs JSON parser): " << path << std::endl;
    return false;
}

bool SoundPack::SaveToFile(const std::string& path) const {
    // TODO: Implement JSON serialization when JSON library is available
    std::cout << "[SoundPack] SaveToFile not yet implemented (needs JSON parser): " << path << std::endl;
    return false;
}

std::vector<AudioEventType> SoundPack::GetRegisteredEvents() const {
    std::vector<AudioEventType> result;
    result.reserve(events.size());
    for (const auto& pair : events) {
        result.push_back(pair.first);
    }
    return result;
}

// ============================================================
// Default Sound Pack
// ============================================================

SoundPack CreateDefaultSoundPack() {
    SoundPack pack("default");

    // Movement
    pack.RegisterEvent(AudioEventType::Footstep, "sfx/footstep.wav", 0.4f);
    pack.RegisterEvent(AudioEventType::Jump, "sfx/jump.wav", 0.5f);
    pack.RegisterEvent(AudioEventType::Land, "sfx/land.wav", 0.4f);

    // Combat
    pack.RegisterEvent(AudioEventType::AttackSwing, "sfx/attack_swing.wav", 0.6f);
    pack.RegisterEvent(AudioEventType::AttackHit, "sfx/attack_hit.wav", 0.7f);
    pack.RegisterEvent(AudioEventType::Hurt, "sfx/hurt.wav", 0.6f);
    pack.RegisterEvent(AudioEventType::Die, "sfx/die.wav", 0.8f);
    pack.RegisterEvent(AudioEventType::Explosion, "sfx/explosion.wav", 1.0f);

    // Interaction
    pack.RegisterEvent(AudioEventType::Interact, "sfx/interact.wav", 0.5f);
    pack.RegisterEvent(AudioEventType::Pickup, "sfx/pickup.wav", 0.4f);
    pack.RegisterEvent(AudioEventType::Open, "sfx/open.wav", 0.5f);
    pack.RegisterEvent(AudioEventType::Close, "sfx/close.wav", 0.5f);

    // UI
    pack.RegisterEvent(AudioEventType::UIClick, "ui/click.wav", 0.5f);
    pack.RegisterEvent(AudioEventType::UIHover, "ui/hover.wav", 0.2f);
    pack.RegisterEvent(AudioEventType::UIConfirm, "ui/confirm.wav", 0.5f);
    pack.RegisterEvent(AudioEventType::UICancel, "ui/cancel.wav", 0.5f);
    pack.RegisterEvent(AudioEventType::UIError, "ui/error.wav", 0.5f);

    // Ambient
    pack.RegisterEvent(AudioEventType::AmbientForest, "ambient/forest.wav", 0.6f);
    pack.RegisterEvent(AudioEventType::AmbientCave, "ambient/cave.wav", 0.5f);
    pack.RegisterEvent(AudioEventType::AmbientOcean, "ambient/ocean.wav", 0.5f);
    pack.RegisterEvent(AudioEventType::AmbientWind, "ambient/wind.wav", 0.4f);

    return pack;
}

} // namespace vge
