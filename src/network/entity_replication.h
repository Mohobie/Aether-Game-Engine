#pragma once
#include "network_serialization.h"
#include "entity/entity.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <functional>

namespace vge {

// ============================================
// Sync Strategy
// ============================================
enum class SyncStrategy {
    Continuous,  // Sync at fixed interval (e.g., 30Hz)
    OnChange     // Only sync when value changes
};

// ============================================
// Replicated Property Base
// ============================================
class ReplicatedPropertyBase {
public:
    std::string name;
    float syncInterval;
    float syncTimer;
    SyncStrategy strategy;
    bool dirty;

    ReplicatedPropertyBase(const std::string& n, float interval = 0.0f, SyncStrategy s = SyncStrategy::OnChange)
        : name(n), syncInterval(interval), syncTimer(0.0f), strategy(s), dirty(true) {}
    virtual ~ReplicatedPropertyBase() = default;

    virtual void Serialize(NetBuffer& buffer) const = 0;
    virtual void Deserialize(NetBuffer& buffer) = 0;
    virtual bool HasChanged() const = 0;
    virtual void MarkClean() = 0;
};

// ============================================
// Replicated Property Template
// ============================================
template<typename T>
class ReplicatedProperty : public ReplicatedPropertyBase {
private:
    T currentValue;
    T lastSyncedValue;

public:
    ReplicatedProperty(const std::string& n, float interval = 0.0f, SyncStrategy s = SyncStrategy::OnChange)
        : ReplicatedPropertyBase(n, interval, s) {}

    void SetValue(const T& value) {
        currentValue = value;
        dirty = true;
    }

    T GetValue() const { return currentValue; }

    void Serialize(NetBuffer& buffer) const override {
        SerializeValue(buffer, currentValue);
    }

    void Deserialize(NetBuffer& buffer) override {
        DeserializeValue(buffer, currentValue);
        lastSyncedValue = currentValue;
    }

    bool HasChanged() const override {
        return dirty || currentValue != lastSyncedValue;
    }

    void MarkClean() override {
        lastSyncedValue = currentValue;
        dirty = false;
    }

private:
    void SerializeValue(NetBuffer& buffer, const Vec3& value) const {
        buffer.WriteVec3(value);
    }
    void SerializeValue(NetBuffer& buffer, float value) const {
        buffer.WriteFloat(value);
    }
    void SerializeValue(NetBuffer& buffer, int32_t value) const {
        buffer.WriteInt(value);
    }
    void SerializeValue(NetBuffer& buffer, bool value) const {
        buffer.WriteBool(value);
    }
    void SerializeValue(NetBuffer& buffer, const std::string& value) const {
        buffer.WriteString(value);
    }

    void DeserializeValue(NetBuffer& buffer, Vec3& value) {
        value = buffer.ReadVec3();
    }
    void DeserializeValue(NetBuffer& buffer, float& value) {
        value = buffer.ReadFloat();
    }
    void DeserializeValue(NetBuffer& buffer, int32_t& value) {
        value = buffer.ReadInt();
    }
    void DeserializeValue(NetBuffer& buffer, bool& value) {
        value = buffer.ReadBool();
    }
    void DeserializeValue(NetBuffer& buffer, std::string& value) {
        value = buffer.ReadString();
    }
};

// ============================================
// Entity Type Registration
// ============================================
struct EntityTypeRegistration {
    uint32_t typeId;
    std::string typeName;
    std::vector<std::unique_ptr<ReplicatedPropertyBase>> properties;

    explicit EntityTypeRegistration(uint32_t id, const std::string& name)
        : typeId(id), typeName(name) {}
};

// ============================================
// Interest Manager (culling)
// ============================================
class InterestManager {
private:
    std::unordered_map<ClientID, Vec3> clientPositions;
    float relevanceRadius;

public:
    explicit InterestManager(float radius = 100.0f) : relevanceRadius(radius) {}

    void UpdateClientPosition(ClientID clientId, const Vec3& position) {
        clientPositions[clientId] = position;
    }

    bool IsRelevant(ClientID clientId, const Vec3& entityPosition) const {
        auto it = clientPositions.find(clientId);
        if (it == clientPositions.end()) {
            return true; // If no position known, assume relevant
        }
        float dist = (it->second - entityPosition).length();
        return dist <= relevanceRadius;
    }

    void RemoveClient(ClientID clientId) {
        clientPositions.erase(clientId);
    }

    void SetRelevanceRadius(float radius) { relevanceRadius = radius; }
    float GetRelevanceRadius() const { return relevanceRadius; }
};

// ============================================
// Entity Replicator
// ============================================
class EntityReplicator {
private:
    std::unordered_map<uint32_t, std::unique_ptr<EntityTypeRegistration>> entityTypes;
    std::unordered_map<EntityID, uint32_t> entityTypeMap;
    std::unordered_map<EntityID, Vec3> entityPositions;
    std::unordered_map<EntityID, std::vector<std::unique_ptr<ReplicatedPropertyBase>>> entityStates;
    InterestManager interestManager;

public:
    EntityReplicator() = default;
    ~EntityReplicator() = default;

    // Entity type registration
    void RegisterEntityType(uint32_t typeId, const std::string& typeName);
    void RegisterProperty(uint32_t typeId, uint32_t propId, const std::string& name, float syncInterval = 0.0f);

    // Entity lifecycle
    void SpawnEntity(EntityID entityId, uint32_t typeId, const Vec3& position);
    void DestroyEntity(EntityID entityId);
    bool IsEntityRegistered(EntityID entityId) const;

    // State updates
    template<typename T>
    void UpdateEntityState(EntityID entityId, uint32_t propId, const T& value) {
        auto it = entityStates.find(entityId);
        if (it != entityStates.end() && propId < it->second.size()) {
            auto* prop = dynamic_cast<ReplicatedProperty<T>*>(it->second[propId].get());
            if (prop) {
                prop->SetValue(value);
            }
        }
    }

    void UpdateEntityPosition(EntityID entityId, const Vec3& position);
    Vec3 GetEntityPosition(EntityID entityId) const;

    // Serialization
    std::vector<uint8_t> SerializeEntityState(EntityID entityId, ClientID clientId);
    void DeserializeEntityState(EntityID entityId, const std::vector<uint8_t>& data);

    // Interest management
    void UpdateClientPosition(ClientID clientId, const Vec3& position);
    bool IsEntityRelevantForClient(EntityID entityId, ClientID clientId) const;
    void SetRelevanceRadius(float radius) { interestManager.SetRelevanceRadius(radius); }

    // Delta compression
    std::vector<uint8_t> SerializeDeltaState(EntityID entityId, const std::vector<uint8_t>& baseline);

    // Update
    void Update(float deltaTime);

    // Cleanup
    void Clear();
};

} // namespace vge
