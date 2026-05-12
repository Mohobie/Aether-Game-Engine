#include "entity_replication.h"
#include <iostream>
#include <algorithm>

namespace vge {

void EntityReplicator::RegisterEntityType(uint32_t typeId, const std::string& typeName) {
    entityTypes[typeId] = std::make_unique<EntityTypeRegistration>(typeId, typeName);
    std::cout << "[EntityReplicator] Registered entity type: " << typeName << " (ID: " << typeId << ")" << std::endl;
}

void EntityReplicator::RegisterProperty(uint32_t typeId, uint32_t propId, const std::string& name, float syncInterval) {
    auto it = entityTypes.find(typeId);
    if (it == entityTypes.end()) {
        return;
    }

    auto& props = it->second->properties;
    if (propId >= props.size()) {
        props.resize(propId + 1);
    }

    // Create a float property by default (can be extended for other types)
    props[propId] = std::make_unique<ReplicatedProperty<float>>(name, syncInterval, 
        syncInterval > 0.0f ? SyncStrategy::Continuous : SyncStrategy::OnChange);
}

void EntityReplicator::SpawnEntity(EntityID entityId, uint32_t typeId, const Vec3& position) {
    entityTypeMap[entityId] = typeId;
    entityPositions[entityId] = position;

    // Copy properties from type registration
    auto typeIt = entityTypes.find(typeId);
    if (typeIt != entityTypes.end()) {
        auto& state = entityStates[entityId];
        state.clear();
        for (const auto& prop : typeIt->second->properties) {
            if (prop) {
                // Clone the property
                auto* floatProp = dynamic_cast<ReplicatedProperty<float>*>(prop.get());
                if (floatProp) {
                    state.push_back(std::make_unique<ReplicatedProperty<float>>(
                        floatProp->name, floatProp->syncInterval, floatProp->strategy));
                }
            }
        }
    }

    std::cout << "[EntityReplicator] Spawned entity " << entityId << " of type " << typeId << std::endl;
}

void EntityReplicator::DestroyEntity(EntityID entityId) {
    entityTypeMap.erase(entityId);
    entityPositions.erase(entityId);
    entityStates.erase(entityId);
    std::cout << "[EntityReplicator] Destroyed entity " << entityId << std::endl;
}

bool EntityReplicator::IsEntityRegistered(EntityID entityId) const {
    return entityTypeMap.find(entityId) != entityTypeMap.end();
}

void EntityReplicator::UpdateEntityPosition(EntityID entityId, const Vec3& position) {
    entityPositions[entityId] = position;
}

Vec3 EntityReplicator::GetEntityPosition(EntityID entityId) const {
    auto it = entityPositions.find(entityId);
    if (it != entityPositions.end()) {
        return it->second;
    }
    return Vec3(0, 0, 0);
}

std::vector<uint8_t> EntityReplicator::SerializeEntityState(EntityID entityId, ClientID clientId) {
    NetBuffer buffer;

    auto typeIt = entityTypeMap.find(entityId);
    if (typeIt == entityTypeMap.end()) {
        return buffer.GetData();
    }

    // Write entity ID and type
    buffer.WriteInt(static_cast<int32_t>(entityId));
    buffer.WriteInt(static_cast<int32_t>(typeIt->second));

    // Write position
    auto posIt = entityPositions.find(entityId);
    if (posIt != entityPositions.end()) {
        buffer.WriteVec3(posIt->second);
    } else {
        buffer.WriteVec3(Vec3(0, 0, 0));
    }

    // Write properties
    auto stateIt = entityStates.find(entityId);
    if (stateIt != entityStates.end()) {
        for (const auto& prop : stateIt->second) {
            if (prop && prop->HasChanged()) {
                prop->Serialize(buffer);
            }
        }
    }

    return buffer.GetData();
}

void EntityReplicator::DeserializeEntityState(EntityID entityId, const std::vector<uint8_t>& data) {
    NetBuffer buffer(data);

    // Read entity ID (should match)
    int32_t readEntityId = buffer.ReadInt();
    if (static_cast<EntityID>(readEntityId) != entityId) {
        return;
    }

    // Read type ID
    int32_t typeId = buffer.ReadInt();

    // Read position
    Vec3 position = buffer.ReadVec3();
    entityPositions[entityId] = position;

    // Read properties
    auto stateIt = entityStates.find(entityId);
    if (stateIt != entityStates.end()) {
        for (auto& prop : stateIt->second) {
            if (prop && buffer.HasMore()) {
                prop->Deserialize(buffer);
                prop->MarkClean();
            }
        }
    }
}

void EntityReplicator::UpdateClientPosition(ClientID clientId, const Vec3& position) {
    interestManager.UpdateClientPosition(clientId, position);
}

bool EntityReplicator::IsEntityRelevantForClient(EntityID entityId, ClientID clientId) const {
    auto posIt = entityPositions.find(entityId);
    if (posIt == entityPositions.end()) {
        return false;
    }
    return interestManager.IsRelevant(clientId, posIt->second);
}

std::vector<uint8_t> EntityReplicator::SerializeDeltaState(EntityID entityId, const std::vector<uint8_t>& baseline) {
    // Simple delta: only include changed properties
    return SerializeEntityState(entityId, INVALID_CLIENT_ID);
}

void EntityReplicator::Update(float deltaTime) {
    // Update sync timers
    for (auto& pair : entityStates) {
        for (auto& prop : pair.second) {
            if (prop && prop->strategy == SyncStrategy::Continuous) {
                prop->syncTimer += deltaTime;
                if (prop->syncTimer >= prop->syncInterval) {
                    prop->dirty = true;
                    prop->syncTimer = 0.0f;
                }
            }
        }
    }
}

void EntityReplicator::Clear() {
    entityTypes.clear();
    entityTypeMap.clear();
    entityPositions.clear();
    entityStates.clear();
}

} // namespace vge
