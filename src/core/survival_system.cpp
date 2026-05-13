#include "survival_system.h"
#include <algorithm>
#include <cmath>

namespace vge {

SurvivalSystem::SurvivalSystem()
    : health(20.0f), maxHealth(20.0f)
    , healthRegenRate(0.5f), healthRegenDelay(10.0f), healthRegenTimer(0.0f)
    , hunger(20.0f), maxHunger(20.0f)
    , hungerDecayRate(0.05f), hungerDamageRate(1.0f), hungerDamageTimer(0.0f)
    , breath(20.0f), maxBreath(20.0f)
    , breathDecayRate(2.0f), breathRegenRate(5.0f), isUnderwater(false)
    , experience(0.0f), level(0), expToNextLevel(10.0f)
    , spawnPoint(0.0f, 50.0f, 0.0f), deathCount(0)
    , respawnInvulnerabilityTime(3.0f), invulnerabilityTimer(0.0f) {
}

SurvivalSystem::~SurvivalSystem() {
}

void SurvivalSystem::Update(float deltaTime) {
    if (IsDead()) return;

    UpdateHunger(deltaTime);
    UpdateBreath(deltaTime);
    UpdateHealthRegen(deltaTime);

    if (invulnerabilityTimer > 0.0f) {
        invulnerabilityTimer -= deltaTime;
    }
}

void SurvivalSystem::SetHealth(float value) {
    float oldHealth = health;
    health = std::clamp(value, 0.0f, maxHealth);
    if (health != oldHealth && onHealthChanged) {
        onHealthChanged(health);
    }
    if (health <= 0.0f && oldHealth > 0.0f) {
        Die();
    }
}

void SurvivalSystem::TakeDamage(float amount) {
    if (invulnerabilityTimer > 0.0f || IsDead()) return;

    healthRegenTimer = healthRegenDelay; // Reset regen timer
    SetHealth(health - amount);
}

void SurvivalSystem::Heal(float amount) {
    if (IsDead()) return;
    SetHealth(health + amount);
}

void SurvivalSystem::Die() {
    deathCount++;
    if (onDeath) {
        onDeath();
    }
}

void SurvivalSystem::Respawn(const Vec3& spawnPos) {
    health = maxHealth;
    hunger = maxHunger;
    breath = maxBreath;
    invulnerabilityTimer = respawnInvulnerabilityTime;
    if (onRespawn) {
        onRespawn();
    }
}

void SurvivalSystem::SetHunger(float value) {
    float oldHunger = hunger;
    hunger = std::clamp(value, 0.0f, maxHunger);
    if (hunger != oldHunger && onHungerChanged) {
        onHungerChanged(hunger);
    }
}

void SurvivalSystem::Eat(float amount) {
    if (IsDead()) return;
    SetHunger(hunger + amount);
    // Eating also heals slightly
    Heal(amount * 0.5f);
}

void SurvivalSystem::Starve(float amount) {
    if (IsDead()) return;
    SetHunger(hunger - amount);
}

void SurvivalSystem::SetBreath(float value) {
    breath = std::clamp(value, 0.0f, maxBreath);
}

void SurvivalSystem::SetUnderwater(bool underwater) {
    isUnderwater = underwater;
}

void SurvivalSystem::AddExperience(float amount) {
    experience += amount;
    CheckLevelUp();
}

void SurvivalSystem::UpdateHealthRegen(float deltaTime) {
    if (IsDead()) return;
    if (health >= maxHealth) return;
    if (hunger < maxHunger * 0.5f) return; // Need enough hunger to regen

    healthRegenTimer -= deltaTime;
    if (healthRegenTimer <= 0.0f) {
        Heal(healthRegenRate * deltaTime);
    }
}

void SurvivalSystem::UpdateHunger(float deltaTime) {
    if (IsDead()) return;

    // Natural hunger decay
    Starve(hungerDecayRate * deltaTime);

    // Damage from starvation
    if (IsStarving()) {
        hungerDamageTimer += deltaTime;
        if (hungerDamageTimer >= 4.0f) { // Damage every 4 seconds
            TakeDamage(hungerDamageRate);
            hungerDamageTimer = 0.0f;
        }
    } else {
        hungerDamageTimer = 0.0f;
    }
}

void SurvivalSystem::UpdateBreath(float deltaTime) {
    if (IsDead()) return;

    if (isUnderwater) {
        SetBreath(breath - breathDecayRate * deltaTime);
        if (IsDrowning()) {
            TakeDamage(2.0f * deltaTime); // Drowning damage
        }
    } else {
        SetBreath(breath + breathRegenRate * deltaTime);
    }
}

void SurvivalSystem::CheckLevelUp() {
    while (experience >= expToNextLevel) {
        experience -= expToNextLevel;
        level++;
        expToNextLevel *= 1.5f; // Increasing difficulty
        if (onLevelUp) {
            onLevelUp(level);
        }
    }
}

} // namespace vge
