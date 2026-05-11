# Entity Component System (ECS)

The engine uses an Entity-Component-System architecture for game objects.

## Core Concepts

- **Entity**: A unique ID representing a game object
- **Component**: Data attached to entities (position, mesh, health)
- **System**: Logic that processes entities with specific components

## Creating Entities

```cpp
#include "entity/entity.h"
#include "entity/components.h"

// Create entity manager
vge::EntityManager entities;

// Create an entity
vge::EntityID player = entities.CreateEntity();

// Add components
entities.AddComponent<TransformComponent>(player);
entities.AddComponent<MeshComponent>(player);
entities.AddComponent<PhysicsComponent>(player);

// Get component
TransformComponent* transform = entities.GetComponent<TransformComponent>(player);
transform->position = vge::Vec3(0, 10, 0);
transform->rotation = vge::Vec3(0, 45, 0);
transform->scale = vge::Vec3(1, 1, 1);

// Remove component
entities.RemoveComponent<MeshComponent>(player);

// Destroy entity
entities.DestroyEntity(player);
```

## Built-in Components

### Transform Component
```cpp
struct TransformComponent {
    vge::Vec3 position = vge::Vec3::Zero;
    vge::Vec3 rotation = vge::Vec3::Zero;  // Euler angles in degrees
    vge::Vec3 scale = vge::Vec3(1, 1, 1);
    
    vge::Mat4 GetMatrix() {
        return vge::Mat4::Translation(position) *
               vge::Mat4::RotationY(rotation.y) *
               vge::Mat4::RotationX(rotation.x) *
               vge::Mat4::RotationZ(rotation.z) *
               vge::Mat4::Scale(scale);
    }
};
```

### Mesh Component
```cpp
struct MeshComponent {
    vge::Mesh mesh;
    vge::Material material;
    bool visible = true;
    bool castShadows = true;
    bool receiveShadows = true;
};
```

### Physics Component
```cpp
struct PhysicsComponent {
    vge::RigidBody* body = nullptr;
    float mass = 1.0f;
    bool isKinematic = false;
    bool useGravity = true;
    
    void SetVelocity(const vge::Vec3& vel) {
        if (body) body->SetVelocity(vel);
    }
    
    void AddForce(const vge::Vec3& force) {
        if (body) body->AddForce(force);
    }
};
```

### Health Component
```cpp
struct HealthComponent {
    float maxHealth = 100.0f;
    float currentHealth = 100.0f;
    bool isDead = false;
    
    void TakeDamage(float damage) {
        currentHealth -= damage;
        if (currentHealth <= 0) {
            currentHealth = 0;
            isDead = true;
        }
    }
    
    void Heal(float amount) {
        currentHealth = std::min(currentHealth + amount, maxHealth);
    }
};
```

### AI Component
```cpp
struct AIComponent {
    vge::BehaviorTree* behaviorTree = nullptr;
    float detectionRange = 20.0f;
    float attackRange = 2.0f;
    vge::EntityID target = vge::INVALID_ENTITY;
};
```

## Custom Components

```cpp
// Define custom component
struct PlayerComponent {
    float moveSpeed = 5.0f;
    float jumpForce = 10.0f;
    int score = 0;
    bool isGrounded = false;
};

// Register with ECS
entities.RegisterComponentType<PlayerComponent>();

// Use it
entities.AddComponent<PlayerComponent>(player);
PlayerComponent* pc = entities.GetComponent<PlayerComponent>(player);
pc->score += 100;
```

## Systems

### Render System
```cpp
class RenderSystem : public vge::System {
public:
    void Update(float dt) override {
        // Get all entities with Transform + Mesh
        auto entities = GetEntities<TransformComponent, MeshComponent>();
        
        for (auto entity : entities) {
            auto* transform = GetComponent<TransformComponent>(entity);
            auto* mesh = GetComponent<MeshComponent>(entity);
            
            if (!mesh->visible) continue;
            
            // Calculate model matrix
            vge::Mat4 model = transform->GetMatrix();
            
            // Render
            shader.SetMat4("uModel", model);
            mesh->mesh.Draw();
        }
    }
};
```

### Physics System
```cpp
class PhysicsSystem : public vge::System {
public:
    void Update(float dt) override {
        auto entities = GetEntities<TransformComponent, PhysicsComponent>();
        
        for (auto entity : entities) {
            auto* transform = GetComponent<TransformComponent>(entity);
            auto* physics = GetComponent<PhysicsComponent>(entity);
            
            if (!physics->body) continue;
            
            // Sync transform to physics
            physics->body->SetPosition(transform->position);
            physics->body->SetRotation(transform->rotation);
            
            // Step physics
            physics->body->Update(dt);
            
            // Sync physics back to transform
            transform->position = physics->body->GetPosition();
            transform->rotation = physics->body->GetRotation();
        }
    }
};
```

### Health System
```cpp
class HealthSystem : public vge::System {
public:
    void Update(float dt) override {
        auto entities = GetEntities<HealthComponent>();
        
        for (auto entity : entities) {
            auto* health = GetComponent<HealthComponent>(entity);
            
            if (health->isDead) {
                // Handle death
                OnEntityDeath(entity);
            }
        }
    }
    
    void OnEntityDeath(vge::EntityID entity) {
        // Could spawn particles, drop loot, etc.
        auto* transform = GetComponent<TransformComponent>(entity);
        if (transform) {
            // Spawn death effect at position
            particleSystem.SpawnEffect(transform->position, EffectType::Death);
        }
        
        // Destroy entity after delay
        DestroyEntityDelayed(entity, 2.0f);
    }
};
```

## System Manager

```cpp
vge::SystemManager systems;

// Register systems
systems.RegisterSystem<RenderSystem>();
systems.RegisterSystem<PhysicsSystem>();
systems.RegisterSystem<HealthSystem>();
systems.RegisterSystem<AISystem>();

// Initialize all systems
systems.Initialize();

// Update all systems
void Update(float dt) {
    systems.Update(dt);
}

// Shutdown
systems.Shutdown();
```

## Entity Prefabs

```cpp
// Define reusable entity templates
class PrefabManager {
public:
    vge::EntityID CreatePlayer(vge::EntityManager& entities, vge::Vec3 pos) {
        vge::EntityID player = entities.CreateEntity();
        
        // Transform
        auto* transform = entities.AddComponent<TransformComponent>(player);
        transform->position = pos;
        
        // Mesh
        auto* mesh = entities.AddComponent<MeshComponent>(player);
        mesh->mesh.LoadFromFile("models/player.obj");
        
        // Physics
        auto* physics = entities.AddComponent<PhysicsComponent>(player);
        physics->mass = 70.0f;
        physics->body = physicsWorld.CreateCapsule(1.0f, 2.0f, physics->mass);
        
        // Health
        auto* health = entities.AddComponent<HealthComponent>(player);
        health->maxHealth = 100.0f;
        
        // Player-specific
        auto* playerComp = entities.AddComponent<PlayerComponent>(player);
        playerComp->moveSpeed = 5.0f;
        
        return player;
    }
    
    vge::EntityID CreateEnemy(vge::EntityManager& entities, vge::Vec3 pos) {
        vge::EntityID enemy = entities.CreateEntity();
        
        auto* transform = entities.AddComponent<TransformComponent>(enemy);
        transform->position = pos;
        
        auto* mesh = entities.AddComponent<MeshComponent>(enemy);
        mesh->mesh.LoadFromFile("models/enemy.obj");
        
        auto* physics = entities.AddComponent<PhysicsComponent>(enemy);
        physics->body = physicsWorld.CreateCapsule(0.5f, 1.8f, 60.0f);
        
        auto* health = entities.AddComponent<HealthComponent>(enemy);
        health->maxHealth = 50.0f;
        
        auto* ai = entities.AddComponent<AIComponent>(enemy);
        ai->detectionRange = 15.0f;
        ai->attackRange = 1.5f;
        
        return enemy;
    }
};
```

## Querying Entities

```cpp
// Get all entities with specific components
auto renderables = entities.Query<TransformComponent, MeshComponent>();
auto physicsObjects = entities.Query<TransformComponent, PhysicsComponent>();
auto livingEntities = entities.Query<HealthComponent>();

// Iterate
for (vge::EntityID entity : renderables) {
    auto* transform = entities.GetComponent<TransformComponent>(entity);
    auto* mesh = entities.GetComponent<MeshComponent>(entity);
    // ...
}

// Check if entity has component
if (entities.HasComponent<HealthComponent>(entity)) {
    // ...
}

// Get component count
size_t playerCount = entities.Count<PlayerComponent>();
```

## Events

```cpp
// Define event
struct DamageEvent {
    vge::EntityID attacker;
    vge::EntityID victim;
    float damage;
    vge::Vec3 hitPoint;
};

// Subscribe to event
EventSystem::Subscribe<DamageEvent>([](const DamageEvent& event) {
    auto* health = entities.GetComponent<HealthComponent>(event.victim);
    if (health) {
        health->TakeDamage(event.damage);
        
        // Spawn hit effect
        particleSystem.SpawnEffect(event.hitPoint, EffectType::Blood);
    }
});

// Emit event
DamageEvent damage;
damage.attacker = player;
damage.victim = enemy;
damage.damage = 25.0f;
damage.hitPoint = hitPos;
EventSystem::Emit(damage);
```
