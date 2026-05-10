# Entity Module API

**Files:** `entity/entity.h`, `entity/component.h`, `entity/entity_manager.h`, `entity/system.h`

---

## `entity/entity.h`

```cpp
namespace vge {
```

### `class Entity`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Entity` | (ctor) | `EntityID id` | Constructor |
| `GetID` | `EntityID` | `void` | Get ID |
| `IsValid` | `bool` | `void` | Check valid |
| `Destroy` | `void` | `void` | Destroy |
| `HasComponent` | `bool` | `ComponentID type` | Has component |
| `AddComponent` | `Component*` | `ComponentID type` | Add component |
| `RemoveComponent` | `void` | `ComponentID type` | Remove component |
| `GetComponent` | `Component*` | `ComponentID type` | Get component |
| `SetActive` | `void` | `bool active` | Set active |
| `IsActive` | `bool` | `void` | Check active |
| `SetName` | `void` | `const std::string& name` | Set name |
| `GetName` | `const std::string&` | `void` | Get name |
| `SetTag` | `void` | `const std::string& tag` | Set tag |
| `GetTag` | `const std::string&` | `void` | Get tag |
| `SetParent` | `void` | `EntityID parent` | Set parent |
| `GetParent` | `EntityID` | `void` | Get parent |
| `GetChildren` | `const std::vector<EntityID>&` | `void` | Get children |
| `AddChild` | `void` | `EntityID child` | Add child |
| `RemoveChild` | `void` | `EntityID child` | Remove child |

---

## `entity/component.h`

### `class Component`
Base class for all components.

| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `GetTypeID` | `ComponentID` | `void` | Get type ID |
| `GetEntity` | `EntityID` | `void` | Get owning entity |
| `SetEntity` | `void` | `EntityID entity` | Set entity |
| `Initialize` | `void` | `void` | Initialize |
| `Update` | `void` | `float deltaTime` | Update |
| `OnDestroy` | `void` | `void` | On destroy |
| `Serialize` | `std::string` | `void` | Serialize |
| `Deserialize` | `void` | `const std::string& data` | Deserialize |

---

## `entity/entity_manager.h`

### `class EntityManager`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `GetInstance` | `EntityManager&` | `void` | Singleton |
| `CreateEntity` | `EntityID` | `const std::string& name = ""` | Create entity |
| `DestroyEntity` | `void` | `EntityID id` | Destroy entity |
| `GetEntity` | `Entity*` | `EntityID id` | Get entity |
| `IsValid` | `bool` | `EntityID id` | Check valid |
| `GetAllEntities` | `std::vector<EntityID>` | `void` | All entities |
| `GetEntitiesWithTag` | `std::vector<EntityID>` | `const std::string& tag` | By tag |
| `GetEntitiesWithComponent` | `std::vector<EntityID>` | `ComponentID type` | By component |
| `UpdateAll` | `void` | `float deltaTime` | Update all |
| `Clear` | `void` | `void` | Clear all |
| `GetCount` | `size_t` | `void` | Entity count |

---

## `entity/system.h`

### `class System`
Base class for systems.

| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `GetTypeID` | `SystemID` | `void` | Get type ID |
| `Initialize` | `void` | `void` | Initialize |
| `Shutdown` | `void` | `void` | Shutdown |
| `Update` | `void` | `float deltaTime` | Update |
| `OnEntityCreated` | `void` | `EntityID id` | Entity created |
| `OnEntityDestroyed` | `void` | `EntityID id` | Entity destroyed |
| `OnComponentAdded` | `void` | `EntityID id, ComponentID type` | Component added |
| `OnComponentRemoved` | `void` | `EntityID id, ComponentID type` | Component removed |
| `GetName` | `const std::string&` | `void` | Get name |
| `SetEnabled` | `void` | `bool enabled` | Set enabled |
| `IsEnabled` | `bool` | `void` | Check enabled |
