# Entity Module API
**Files:** src/entity/component.h, src/entity/entity.h, src/entity/entity_manager.h, src/entity/system.h

## `entity/component.h`
```cpp
namespace aether {
```

### `class Component`

## `entity/entity.h`
```cpp
namespace aether {
```

### `class Entity`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Entity` | `explicit` | `EntityID id` |

## `entity/entity_manager.h`
```cpp
namespace aether {
```

### `class EntityManager`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `createEntity` | `Entity*` | `` |
| `destroyEntity` | `void` | `EntityID id` |
| `getEntity` | `Entity*` | `EntityID id` |
| `getEntityCount` | `size_t` | `` |
| `clear` | `void` | `` |

## `entity/system.h`
```cpp
namespace aether {
```

### `class System`

### `class SystemManager`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `addSystem` | `void` | `std::unique_ptr<System> system` |
| `initializeAll` | `void` | `` |
| `updateAll` | `void` | `float deltaTime` |
| `shutdownAll` | `void` | `` |
