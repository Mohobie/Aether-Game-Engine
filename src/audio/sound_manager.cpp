#include "audio/sound_manager.h"
#include "audio/audio_engine.h"
#include <iostream>

namespace vge {

SoundManager::SoundManager() : audioEngine(nullptr) {}

SoundManager::~SoundManager() {
    if (audioEngine) Shutdown();
}

bool SoundManager::Initialize(AudioEngine* engine) {
    audioEngine = engine;
    
    if (!audioEngine || !audioEngine->IsInitialized()) {
        std::cerr << "[SoundManager] Audio engine not initialized" << std::endl;
        return false;
    }
    
    std::cout << "[SoundManager] Initialized" << std::endl;
    return true;
}

void SoundManager::Shutdown() {
    sounds.clear();
    audioEngine = nullptr;
    std::cout << "[SoundManager] Shutdown" << std::endl;
}

bool SoundManager::LoadSound(const std::string& name, const std::string& path) {
    if (!audioEngine) return false;
    
    sounds[name] = path;
    std::cout << "[SoundManager] Loaded: " << name << " -> " << path << std::endl;
    return true;
}

void SoundManager::PlaySound(const std::string& name, float volume, bool loop) {
    if (!audioEngine) return;
    
    auto it = sounds.find(name);
    if (it == sounds.end()) {
        std::cerr << "[SoundManager] Sound not found: " << name << std::endl;
        return;
    }
    
    std::cout << "[SoundManager] Playing: " << name << " (vol=" << volume << ", loop=" << loop << ")" << std::endl;
}

void SoundManager::StopSound(const std::string& name) {
    if (!audioEngine) return;
    std::cout << "[SoundManager] Stopped: " << name << std::endl;
}

void SoundManager::StopAllSounds() {
    if (!audioEngine) return;
    std::cout << "[SoundManager] All sounds stopped" << std::endl;
}

void SoundManager::SetMasterVolume(float volume) {
    if (!audioEngine) return;
    audioEngine->SetVolume(volume);
}

// Convenience methods for game events
void SoundManager::PlayBlockPlace(BlockType type) {
    switch (type) {
        case BlockType::Stone: PlaySound("stone_place", 0.8f); break;
        case BlockType::Wood: PlaySound("wood_place", 0.8f); break;
        case BlockType::Dirt: PlaySound("dirt_place", 0.6f); break;
        case BlockType::Grass: PlaySound("grass_place", 0.6f); break;
        default: PlaySound("place_generic", 0.7f); break;
    }
}

void SoundManager::PlayBlockBreak(BlockType type) {
    switch (type) {
        case BlockType::Stone: PlaySound("stone_break", 0.8f); break;
        case BlockType::Wood: PlaySound("wood_break", 0.8f); break;
        case BlockType::Dirt: PlaySound("dirt_break", 0.6f); break;
        case BlockType::Grass: PlaySound("grass_break", 0.6f); break;
        default: PlaySound("break_generic", 0.7f); break;
    }
}

void SoundManager::PlayFootstep(BlockType groundType) {
    switch (groundType) {
        case BlockType::Stone: PlaySound("stone_step", 0.4f); break;
        case BlockType::Wood: PlaySound("wood_step", 0.4f); break;
        case BlockType::Dirt: 
        case BlockType::Grass: PlaySound("grass_step", 0.3f); break;
        default: PlaySound("step_generic", 0.3f); break;
    }
}

void SoundManager::PlayJump() {
    PlaySound("jump", 0.5f);
}

void SoundManager::PlayLand() {
    PlaySound("land", 0.4f);
}

} // namespace vge