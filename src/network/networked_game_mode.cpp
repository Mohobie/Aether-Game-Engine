#include "networked_game_mode.h"
#include <iostream>
#include <algorithm>

namespace vge {

NetworkedGameMode::NetworkedGameMode()
    : gameState(GameState::Waiting)
    , countdownTimer(5.0f)
    , gameTimer(0.0f)
    , maxPlayers(16)
    , winningScore(10)
    , nextTeamId(1)
{
}

void NetworkedGameMode::Initialize() {
    // Default spawn points
    spawnPoints.clear();
    AddSpawnPoint(Vec3(0, 10, 0), 0);
    AddSpawnPoint(Vec3(10, 10, 0), 0);
    AddSpawnPoint(Vec3(0, 10, 10), 0);
    AddSpawnPoint(Vec3(10, 10, 10), 0);

    std::cout << "[NetworkedGameMode] Initialized with " << spawnPoints.size() << " spawn points" << std::endl;
}

void NetworkedGameMode::StartGame() {
    gameState = GameState::Countdown;
    countdownTimer = 5.0f;
    std::cout << "[NetworkedGameMode] Starting countdown..." << std::endl;
}

void NetworkedGameMode::EndGame() {
    gameState = GameState::Ended;
    std::cout << "[NetworkedGameMode] Game ended" << std::endl;
}

void NetworkedGameMode::Update(float deltaTime) {
    switch (gameState) {
        case GameState::Countdown:
            countdownTimer -= deltaTime;
            if (countdownTimer <= 0.0f) {
                gameState = GameState::Playing;
                gameTimer = 0.0f;
                std::cout << "[NetworkedGameMode] Game started!" << std::endl;
            }
            break;

        case GameState::Playing:
            gameTimer += deltaTime;
            break;

        default:
            break;
    }
}

void NetworkedGameMode::OnPlayerJoined(ClientID clientId) {
    if (players.find(clientId) != players.end()) {
        return; // Already joined
    }

    PlayerInfo info;
    info.clientId = clientId;
    info.playerName = "Player_" + std::to_string(clientId);
    info.teamId = AssignTeam(clientId);
    info.spawnPosition = GetNextSpawnPoint();
    info.isReady = false;
    info.isConnected = true;

    players[clientId] = info;
    playerOrder.push_back(clientId);

    std::cout << "[NetworkedGameMode] Player " << clientId << " joined (Team " << info.teamId << ")" << std::endl;

    if (onPlayerJoinedCallback) {
        onPlayerJoinedCallback(clientId);
    }
}

void NetworkedGameMode::OnPlayerLeft(ClientID clientId) {
    auto it = players.find(clientId);
    if (it != players.end()) {
        it->second.isConnected = false;
        players.erase(it);

        auto orderIt = std::find(playerOrder.begin(), playerOrder.end(), clientId);
        if (orderIt != playerOrder.end()) {
            playerOrder.erase(orderIt);
        }

        std::cout << "[NetworkedGameMode] Player " << clientId << " left" << std::endl;

        if (onPlayerLeftCallback) {
            onPlayerLeftCallback(clientId);
        }
    }
}

void NetworkedGameMode::OnPlayerReady(ClientID clientId) {
    auto it = players.find(clientId);
    if (it != players.end()) {
        it->second.isReady = true;
        std::cout << "[NetworkedGameMode] Player " << clientId << " is ready" << std::endl;
    }
}

void NetworkedGameMode::AddSpawnPoint(const Vec3& position, int teamId) {
    spawnPoints.emplace_back(position, teamId);
}

Vec3 NetworkedGameMode::GetNextSpawnPoint() {
    // Simple round-robin spawn point selection
    static size_t nextSpawnIndex = 0;
    if (spawnPoints.empty()) {
        return Vec3(0, 10, 0);
    }

    Vec3 pos = spawnPoints[nextSpawnIndex].position;
    nextSpawnIndex = (nextSpawnIndex + 1) % spawnPoints.size();
    return pos;
}

Vec3 NetworkedGameMode::GetSpawnPointForTeam(int teamId) {
    for (const auto& spawn : spawnPoints) {
        if (spawn.teamId == teamId) {
            return spawn.position;
        }
    }
    return Vec3(0, 10, 0);
}

void NetworkedGameMode::ResetSpawnPoints() {
    for (auto& spawn : spawnPoints) {
        spawn.occupied = false;
    }
}

int NetworkedGameMode::AssignTeam(ClientID clientId) {
    // Simple alternating team assignment
    int team = nextTeamId;
    nextTeamId = (nextTeamId % 2) + 1; // Alternate between 1 and 2
    return team;
}

void NetworkedGameMode::SetPlayerTeam(ClientID clientId, int teamId) {
    auto it = players.find(clientId);
    if (it != players.end()) {
        it->second.teamId = teamId;
    }
}

int NetworkedGameMode::GetPlayerTeam(ClientID clientId) const {
    auto it = players.find(clientId);
    if (it != players.end()) {
        return it->second.teamId;
    }
    return 0;
}

std::vector<ClientID> NetworkedGameMode::GetPlayersOnTeam(int teamId) const {
    std::vector<ClientID> result;
    for (const auto& pair : players) {
        if (pair.second.teamId == teamId) {
            result.push_back(pair.first);
        }
    }
    return result;
}

bool NetworkedGameMode::IsPlayerConnected(ClientID clientId) const {
    auto it = players.find(clientId);
    if (it != players.end()) {
        return it->second.isConnected;
    }
    return false;
}

std::vector<ClientID> NetworkedGameMode::GetAllPlayers() const {
    return playerOrder;
}

const PlayerInfo* NetworkedGameMode::GetPlayerInfo(ClientID clientId) const {
    auto it = players.find(clientId);
    if (it != players.end()) {
        return &it->second;
    }
    return nullptr;
}

bool NetworkedGameMode::CanStartGame() const {
    if (players.empty()) {
        return false;
    }

    // Check if enough players are ready
    int readyCount = 0;
    for (const auto& pair : players) {
        if (pair.second.isReady) {
            readyCount++;
        }
    }

    return readyCount >= 1; // At least 1 player ready
}

void NetworkedGameMode::ResetGame() {
    gameState = GameState::Waiting;
    countdownTimer = 5.0f;
    gameTimer = 0.0f;
    nextTeamId = 1;

    for (auto& pair : players) {
        pair.second.isReady = false;
        pair.second.spawnPosition = GetNextSpawnPoint();
    }

    ResetSpawnPoints();
    std::cout << "[NetworkedGameMode] Game reset" << std::endl;
}

} // namespace vge
