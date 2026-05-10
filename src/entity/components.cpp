#include "components.h"
#include <cmath>

namespace vge {

// ============================================
// Transform Component
// ============================================

Vec3 TransformComponent::GetForward() const {
    float yaw = rotation.y * 3.14159f / 180.0f;
    float pitch = rotation.x * 3.14159f / 180.0f;
    return Vec3(
        std::cos(yaw) * std::cos(pitch),
        std::sin(pitch),
        std::sin(yaw) * std::cos(pitch)
    ).normalize();
}

Vec3 TransformComponent::GetRight() const {
    Vec3 forward = GetForward();
    Vec3 up(0, 1, 0);
    return Vec3(
        forward.y * up.z - forward.z * up.y,
        forward.z * up.x - forward.x * up.z,
        forward.x * up.y - forward.y * up.x
    ).normalize();
}

Vec3 TransformComponent::GetUp() const {
    Vec3 forward = GetForward();
    Vec3 right = GetRight();
    return Vec3(
        forward.y * right.z - forward.z * right.y,
        forward.z * right.x - forward.x * right.z,
        forward.x * right.y - forward.y * right.x
    ).normalize();
}

// ============================================
// Health Component
// ============================================

void HealthComponent::TakeDamage(float amount) {
    currentHealth -= amount;
    if (currentHealth < 0) currentHealth = 0;
    
    if (onDamage) onDamage(amount);
    
    if (currentHealth <= 0 && onDeath) {
        onDeath();
    }
}

void HealthComponent::Heal(float amount) {
    currentHealth += amount;
    if (currentHealth > maxHealth) currentHealth = maxHealth;
}

// ============================================
// Movement Component
// ============================================

void MovementComponent::Update(float deltaTime) {
    // Apply acceleration
    velocity = velocity + acceleration * deltaTime;
    
    // Apply friction
    velocity = velocity * friction;
    
    // Clamp to max speed
    float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y + velocity.z * velocity.z);
    if (speed > maxSpeed) {
        float scale = maxSpeed / speed;
        velocity = velocity * scale;
    }
    
    // Update position if owner has transform
    if (owner && owner->HasComponent<TransformComponent>()) {
        auto* transform = owner->GetComponent<TransformComponent>();
        transform->Translate(velocity * deltaTime);
    }
    
    // Reset acceleration
    acceleration = Vec3(0, 0, 0);
}

} // namespace vge
