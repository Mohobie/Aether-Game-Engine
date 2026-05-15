#include "server_authority.h"
#include <iostream>

namespace vge {

void ServerAuthority::SetAuthority(EntityID entityId, Authority auth, ClientID clientId) {
    AuthorityRecord& record = authorityMap[entityId];
    record.type = auth;
    record.ownerClientId = (auth == Authority::Client) ? clientId : INVALID_CLIENT_ID;
}

Authority ServerAuthority::GetAuthority(EntityID entityId) const {
    auto it = authorityMap.find(entityId);
    if (it != authorityMap.end()) {
        return it->second.type;
    }
    return Authority::None;
}

ClientID ServerAuthority::GetOwnerClient(EntityID entityId) const {
    auto it = authorityMap.find(entityId);
    if (it != authorityMap.end()) {
        return it->second.ownerClientId;
    }
    return INVALID_CLIENT_ID;
}

bool ServerAuthority::HasAuthority(EntityID entityId, Authority auth, ClientID clientId) const {
    auto it = authorityMap.find(entityId);
    if (it == authorityMap.end()) {
        return false;
    }
    if (it->second.type != auth) {
        return false;
    }
    if (auth == Authority::Client && clientId != INVALID_CLIENT_ID) {
        return it->second.ownerClientId == clientId;
    }
    return true;
}

void ServerAuthority::RecordClientInput(ClientID clientId, const PredictedInput& input) {
    clientInputHistory[clientId].push(input);
}

bool ServerAuthority::HasClientInput(ClientID clientId) const {
    auto it = clientInputHistory.find(clientId);
    if (it != clientInputHistory.end()) {
        return !it->second.empty();
    }
    return false;
}

PredictedInput ServerAuthority::GetNextClientInput(ClientID clientId) {
    auto it = clientInputHistory.find(clientId);
    if (it != clientInputHistory.end() && !it->second.empty()) {
        PredictedInput input = it->second.front();
        it->second.pop();
        return input;
    }
    return PredictedInput();
}

void ServerAuthority::ClearClientInputs(ClientID clientId) {
    auto it = clientInputHistory.find(clientId);
    if (it != clientInputHistory.end()) {
        while (!it->second.empty()) {
            it->second.pop();
        }
    }
}

void ServerAuthority::RecordStateSnapshot(EntityID entityId, uint32_t tick, const EntityStateSnapshot& snapshot) {
    (void)tick;
    auto& history = stateHistory[entityId];
    history.push_back(snapshot);
    
    // Keep history bounded
    if (history.size() > MAX_HISTORY_SIZE) {
        history.erase(history.begin());
    }
}

bool ServerAuthority::GetStateSnapshot(EntityID entityId, uint32_t tick, EntityStateSnapshot& outSnapshot) const {
    auto it = stateHistory.find(entityId);
    if (it == stateHistory.end()) {
        return false;
    }
    
    for (const auto& snapshot : it->second) {
        if (snapshot.tick == tick) {
            outSnapshot = snapshot;
            return true;
        }
    }
    return false;
}

void ServerAuthority::ClearOldSnapshots(EntityID entityId, uint32_t olderThanTick) {
    auto it = stateHistory.find(entityId);
    if (it != stateHistory.end()) {
        auto& history = it->second;
        history.erase(
            std::remove_if(history.begin(), history.end(),
                [olderThanTick](const EntityStateSnapshot& s) { return s.tick < olderThanTick; }),
            history.end()
        );
    }
}

bool ServerAuthority::NeedsReconciliation(EntityID entityId, uint32_t clientTick, const EntityStateSnapshot& predictedState) const {
    EntityStateSnapshot serverState;
    if (!GetStateSnapshot(entityId, clientTick, serverState)) {
        return false;
    }
    
    // Check if positions diverge significantly
    float divergence = (serverState.position - predictedState.position).length();
    return divergence > 0.1f; // 10cm threshold
}

Vec3 ServerAuthority::GetCorrectedPosition(EntityID entityId, uint32_t clientTick, const Vec3& predictedPosition) const {
    EntityStateSnapshot serverState;
    if (GetStateSnapshot(entityId, clientTick, serverState)) {
        return serverState.position;
    }
    return predictedPosition;
}

void ServerAuthority::SetPredictedPosition(EntityID entityId, const Vec3& position) {
    predictedPositions[entityId] = position;
}

Vec3 ServerAuthority::GetPredictedPosition(EntityID entityId) const {
    auto it = predictedPositions.find(entityId);
    if (it != predictedPositions.end()) {
        return it->second;
    }
    return Vec3(0, 0, 0);
}

void ServerAuthority::RemoveEntity(EntityID entityId) {
    authorityMap.erase(entityId);
    stateHistory.erase(entityId);
    predictedPositions.erase(entityId);
}

void ServerAuthority::RemoveClient(ClientID clientId) {
    // Remove all entities owned by this client
    std::vector<EntityID> toRemove;
    for (const auto& pair : authorityMap) {
        if (pair.second.ownerClientId == clientId) {
            toRemove.push_back(pair.first);
        }
    }
    for (EntityID id : toRemove) {
        RemoveEntity(id);
    }
    
    clientInputHistory.erase(clientId);
}

void ServerAuthority::Clear() {
    authorityMap.clear();
    stateHistory.clear();
    clientInputHistory.clear();
    predictedPositions.clear();
}

} // namespace vge
