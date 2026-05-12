#pragma once
#include "network_serialization.h"
#include "math/vec3.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace vge {

// ============================================
// Game State
// ============================================
enum class GameState {
    Waiting,
    Countdown,
    Playing,
    Ended
};

// ============================================
// Player Info
// ============================================
struct PlayerInfo {
    ClientID clientId;
    std::string playerName;
    int teamId;
    Vec3 spawnPosition;
    bool isReady;
    bool isConnected;

    PlayerInfo() : clientId(INVALID_CLIENT_ID), playerName("Player"), teamId(0),
                   spawnPosition(0, 0, 0), isReady(false), isConnected(true) {}
};

// ============================================
// Spawn Point
// ============================================
struct SpawnPoint {
    Vec3 position;
    int teamId;
    bool occupied;

    SpawnPoint() : position(0, 0, 0), teamId(0), occupied(false) {}
    SpawnPoint(const Vec3& pos, int team = 0) : position(pos), teamId(team), occupied(false) {}
};

// ============================================
// Networked Game Mode Base
// ============================================
class NetworkedGameMode {
private:
    GameState gameState;
    float countdownTimer;
    float gameTimer;
    int maxPlayers;
    int winningScore;

    std::unordered_map<ClientID, PlayerInfo> players;
    std::vector<ClientID> playerOrder;
    std::vector<SpawnPoint> spawnPoints;

    std::function<void(ClientID)> onPlayerJoinedCallback;
    std::function<void(ClientID)> onPlayerLeftCallback;

protected:
    int nextTeamId;

public:
    NetworkedGameMode();
    virtual ~NetworkedGameMode() = default;

    // Game lifecycle
    virtual void Initialize();
    virtual void StartGame();
    virtual void EndGame();
    virtual void Update(float deltaTime);

    // Player management
    virtual void OnPlayerJoined(ClientID clientId);
    virtual void OnPlayerLeft(ClientID clientId);
    virtual void OnPlayerReady(ClientID clientId);

    // Spawn management
    void AddSpawnPoint(const Vec3& position, int teamId = 0);
    Vec3 GetNextSpawnPoint();
    Vec3 GetSpawnPointForTeam(int teamId);
    void ResetSpawnPoints();

    // Team management
    virtual int AssignTeam(ClientID clientId);
    void SetPlayerTeam(ClientID clientId, int teamId);
    int GetPlayerTeam(ClientID clientId) const;
    std::vector<ClientID> GetPlayersOnTeam(int teamId) const;

    // Queries
    bool IsPlayerConnected(ClientID clientId) const;
    size_t GetPlayerCount() const { return players.size(); }
    std::vector<ClientID> GetAllPlayers() const;
    const PlayerInfo* GetPlayerInfo(ClientID clientId) const;

    // Game state
    GameState GetGameState() const { return gameState; }
    void SetGameState(GameState state) { gameState = state; }
    float GetGameTime() const { return gameTimer; }
    void SetGameTime(float time) { gameTimer = time; }

    // Configuration
    void SetMaxPlayers(int max) { maxPlayers = max; }
    int GetMaxPlayers() const { return maxPlayers; }
    void SetWinningScore(int score) { winningScore = score; }
    int GetWinningScore() const { return winningScore; }

    // Callbacks
    void OnPlayerJoined(std::function<void(ClientID)> callback) { onPlayerJoinedCallback = callback; }
    void OnPlayerLeft(std::function<void(ClientID)> callback) { onPlayerLeftCallback = callback; }

    // Utility
    bool CanStartGame() const;
    bool IsGameInProgress() const { return gameState == GameState::Playing; }
    void ResetGame();
};

} // namespace vge
