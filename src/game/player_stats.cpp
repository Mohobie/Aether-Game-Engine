#include "game/player_stats.h"
#include "core/logger.h"
#include <algorithm>

namespace vge {

PlayerStats::PlayerStats()
    : health(100.0f), hunger(100.0f), stamina(100.0f),
      maxHealth(100.0f), maxHunger(100.0f), maxStamina(100.0f),
      isDead(false), isSprinting(false),
      hungerDecayTimer(0.0f), healthRegenTimer(0.0f),
      hungerDecayRate(0.5f), staminaRegenRate(10.0f),
      staminaSprintCost(15.0f), healthRegenRate(1.0f),
      starvationDamage(1.0f),
      spawnPoint(0, 20, 0) {}

PlayerStats::PlayerStats(const Vec3& spawn)
    : health(100.0f), hunger(100.0f), stamina(100.0f),
      maxHealth(100.0f), maxHunger(100.0f), maxStamina(100.0f),
      isDead(false), isSprinting(false),
      hungerDecayTimer(0.0f), healthRegenTimer(0.0f),
      hungerDecayRate(0.5f), staminaRegenRate(10.0f),
      staminaSprintCost(15.0f), healthRegenRate(1.0f),
      starvationDamage(1.0f),
      spawnPoint(spawn) {}

void PlayerStats::ClampStats() {
    health = std::max(0.0f, std::min(health, maxHealth));
    hunger = std::max(0.0f, std::min(hunger, maxHunger));
    stamina = std::max(0.0f, std::min(stamina, maxStamina));
}

void PlayerStats::TakeDamage(float amount) {
    if (isDead || amount <= 0.0f) return;
    health -= amount;
    ClampStats();
    if (health <= 0.0f) {
        health = 0.0f;
        Die();
    }
}

void PlayerStats::Heal(float amount) {
    if (isDead || amount <= 0.0f) return;
    health += amount;
    ClampStats();
}

void PlayerStats::Eat(float foodValue) {
    if (isDead || foodValue <= 0.0f) return;
    hunger += foodValue;
    ClampStats();
}

bool PlayerStats::UseStamina(float amount) {
    if (isDead) return false;
    if (stamina < amount) return false;
    stamina -= amount;
    ClampStats();
    return true;
}

void PlayerStats::RegenerateStamina(float amount) {
    if (isDead) return;
    stamina += amount;
    ClampStats();
}

void PlayerStats::Update(float deltaTime) {
    if (isDead) return;

    // Hunger decay over time
    hungerDecayTimer += deltaTime;
    if (hungerDecayTimer >= 1.0f) {
        hunger -= hungerDecayRate * hungerDecayTimer;
        hungerDecayTimer = 0.0f;
        ClampStats();
    }

    // Stamina regeneration / sprint cost
    if (isSprinting) {
        stamina -= staminaSprintCost * deltaTime;
    } else {
        stamina += staminaRegenRate * deltaTime;
    }
    ClampStats();

    // Health regeneration when well-fed
    if (hunger > maxHunger * 0.8f && health < maxHealth) {
        healthRegenTimer += deltaTime;
        if (healthRegenTimer >= 1.0f) {
            health += healthRegenRate * healthRegenTimer;
            healthRegenTimer = 0.0f;
            ClampStats();
        }
    } else {
        healthRegenTimer = 0.0f;
    }

    // Starvation damage when hunger is depleted
    if (hunger <= 0.0f) {
        health -= starvationDamage * deltaTime;
        ClampStats();
        if (health <= 0.0f) {
            health = 0.0f;
            Die();
        }
    }
}

void PlayerStats::Die() {
    if (isDead) return;
    isDead = true;
    health = 0.0f;
    Logger::Info("[PlayerStats] Player died");
}

void PlayerStats::Respawn() {
    isDead = false;
    health = maxHealth;
    hunger = maxHunger;
    stamina = maxStamina;
    isSprinting = false;
    hungerDecayTimer = 0.0f;
    healthRegenTimer = 0.0f;
    Logger::Info("[PlayerStats] Player respawned at (" +
                 std::to_string((int)spawnPoint.x) + ", " +
                 std::to_string((int)spawnPoint.y) + ", " +
                 std::to_string((int)spawnPoint.z) + ")");
}

void PlayerStats::SetHealth(float value) {
    health = value;
    ClampStats();
    if (health <= 0.0f && !isDead) {
        Die();
    }
}

void PlayerStats::SetHunger(float value) {
    hunger = value;
    ClampStats();
}

void PlayerStats::SetStamina(float value) {
    stamina = value;
    ClampStats();
}

} // namespace vge
