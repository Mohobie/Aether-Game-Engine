#pragma once
#include "network_serialization.h"
#include "entity/entity.h"
#include <unordered_map>
#include <queue>
#include <vector>

namespace vge {

// ============================================
// Authority Type
// ============================================
enum class Authority {
    None,
    Server,
    Client
};

// ============================================
// Predicted Input
// ============================================
struct PredictedInput {
    uint32_t tick;
    Vec3 moveInput;
    Vec3 rotationInput;
    bool jumpPressed;
    bool attackPressed;

    PredictedInput() : tick(0), moveInput(0, 0, 0), rotationInput(0, 0, 0), jumpPressed(false), attackPressed(false) {}
};

// ============================================
// Entity State Snapshot
// ============================================
struct EntityStateSnapshot {
    Vec3 position;
    Vec3 rotation;
    Vec3 velocity;
    float health;
    uint32_t tick;

    EntityStateSnapshot() : position(0, 0, 0), rotation(0, 0, 0), velocity(0, 0, 0), health(100.0f), tick(0) {}
};

// ============================================
// Authority Record
// ============================================
struct AuthorityRecord {
    Authority type;
    ClientID ownerClientId;
    uint32_t lastConfirmedTick;

    AuthorityRecord() : type(Authority::None), ownerClientId(INVALID_CLIENT_ID), lastConfirmedTick(0) {}
};

// ============================================
// Server Authority Manager
// ============================================
class ServerAuthority {
private:
    std::unordered_map<EntityID, AuthorityRecord> authorityMap;
    std::unordered_map<EntityID, std::vector<EntityStateSnapshot>> stateHistory;
    std::unordered_map<ClientID, std::queue<PredictedInput>> clientInputHistory;
    std::unordered_map<EntityID, Vec3> predictedPositions;

    static constexpr size_t MAX_HISTORY_SIZE = 120; // 2 seconds at 60Hz

public:
    ServerAuthority() = default;
    ~ServerAuthority() = default;

    // Authority management
    void SetAuthority(EntityID entityId, Authority auth, ClientID clientId = INVALID_CLIENT_ID);
    Authority GetAuthority(EntityID entityId) const;
    ClientID GetOwnerClient(EntityID entityId) const;
    bool HasAuthority(EntityID entityId, Authority auth, ClientID clientId = INVALID_CLIENT_ID) const;
    bool IsServerOwned(EntityID entityId) const { return HasAuthority(entityId, Authority::Server); }
    bool IsClientOwned(EntityID entityId, ClientID clientId) const { return HasAuthority(entityId, Authority::Client, clientId); }

    // Client prediction
    void RecordClientInput(ClientID clientId, const PredictedInput& input);
    bool HasClientInput(ClientID clientId) const;
    PredictedInput GetNextClientInput(ClientID clientId);
    void ClearClientInputs(ClientID clientId);

    // State snapshots
    void RecordStateSnapshot(EntityID entityId, uint32_t tick, const EntityStateSnapshot& snapshot);
    bool GetStateSnapshot(EntityID entityId, uint32_t tick, EntityStateSnapshot& outSnapshot) const;
    void ClearOldSnapshots(EntityID entityId, uint32_t olderThanTick);

    // Reconciliation
    bool NeedsReconciliation(EntityID entityId, uint32_t clientTick, const EntityStateSnapshot& predictedState) const;
    Vec3 GetCorrectedPosition(EntityID entityId, uint32_t clientTick, const Vec3& predictedPosition) const;

    // Prediction helpers
    void SetPredictedPosition(EntityID entityId, const Vec3& position);
    Vec3 GetPredictedPosition(EntityID entityId) const;

    // Cleanup
    void RemoveEntity(EntityID entityId);
    void RemoveClient(ClientID clientId);
    void Clear();
};

} // namespace vge
