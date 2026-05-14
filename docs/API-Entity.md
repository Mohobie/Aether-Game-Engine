# Entity Module API

**Canonical files:** `src/entity/entity.h`, `src/entity/components.h`  
**Legacy / inactive for this audit:** `src/core/entity.h`, `src/entity/component.h`, `src/entity/system.h`

This page reflects the 2026-05-14 architecture audit. The supported entity layer in the current build is the `vge` entity/component system declared in `entity/entity.h` and extended by `entity/components.h`.

## `entity/entity.h`

```cpp
namespace vge {
```

### `using EntityID = uint32_t`

### `constexpr EntityID INVALID_ENTITY = 0`

### `class Component`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Component` | ctor | `` |
| `~Component` | virtual dtor | `` |
| `OnAttach` | `void` | `` |
| `OnDetach` | `void` | `` |
| `Update` | `void` | `float deltaTime` |
| `Init` | `void` | `` |

### `class Entity`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Entity` | ctor | `EntityID id, const std::string& name = "Entity"` |
| `~Entity` | dtor | `` |
| `GetID` | `EntityID` | `` |
| `GetName` | `const std::string&` | `` |
| `SetName` | `void` | `const std::string& n` |
| `IsActive` | `bool` | `` |
| `SetActive` | `void` | `bool a` |
| `AddComponent<T>` | `T*` | `Args&&... args` |
| `GetComponent<T>` | `T*` | `` |
| `HasComponent<T>` | `bool` | `` |
| `RemoveComponent<T>` | `void` | `` |
| `Update` | `void` | `float deltaTime` |
| `GetAllComponents` | `std::vector<Component*>` | `` |

### `class EntityManager`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `EntityManager` | ctor | `` |
| `~EntityManager` | dtor | `` |
| `CreateEntity` | `Entity*` | `const std::string& name = "Entity"` |
| `DestroyEntity` | `void` | `EntityID id` |
| `DestroyEntity` | `void` | `Entity* entity` |
| `GetEntity` | `Entity*` | `EntityID id` |
| `GetAllEntities` | `std::vector<Entity*>` | `` |
| `GetEntitiesWithComponent<T>` | `std::vector<Entity*>` | `` |
| `Update` | `void` | `float deltaTime` |
| `Clear` | `void` | `` |
| `GetEntityCount` | `size_t` | `` |

## `entity/components.h`

```cpp
namespace vge {
```

### Canonical concrete components

- `TransformComponent`
- `HealthComponent`
- `MovementComponent`
- `ScriptComponent`

## Legacy note

`core/entity.h` defines a different `vge::Entity` family and is not part of the canonical build path for this session.
