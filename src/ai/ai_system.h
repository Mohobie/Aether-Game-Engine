#pragma once
#include "math/vec3.h"
#include <vector>
#include <memory>
#include <functional>

namespace vge {

// ============================================
// Navigation Mesh Node
// ============================================
struct NavNode {
    Vec3 position;
    float gCost;  // Cost from start
    float hCost;  // Heuristic cost to goal
    float fCost() const { return gCost + hCost; }
    
    bool walkable;
    std::vector<int> neighbors;  // Indices of connected nodes
    
    int parentIndex;  // For path reconstruction
    bool visited;
    
    NavNode() : gCost(0), hCost(0), walkable(true), parentIndex(-1), visited(false) {}
};

// ============================================
// Navigation Mesh
// ============================================
class NavigationMesh {
private:
    std::vector<NavNode> nodes;
    float nodeSpacing;
    Vec3 boundsMin;
    Vec3 boundsMax;
    
public:
    NavigationMesh();
    
    // Generation
    void GenerateFromWorld(const class World& world, float spacing = 1.0f);
    void GenerateGrid(const Vec3& min, const Vec3& max, float spacing);
    void AddConnection(int from, int to);
    void RemoveConnection(int from, int to);
    
    // Node management
    int AddNode(const Vec3& position, bool walkable = true);
    NavNode* GetNode(int index);
    int GetNodeCount() const { return nodes.size(); }
    
    // Queries
    int FindNearestNode(const Vec3& position) const;
    std::vector<int> FindNodesInRadius(const Vec3& center, float radius) const;
    
    // Pathfinding
    std::vector<Vec3> FindPath(const Vec3& start, const Vec3& goal) const;
    bool IsWalkable(const Vec3& position) const;
    
    // Debug
    void Visualize() const;
    
private:
    // A* pathfinding
    std::vector<int> AStar(int startIndex, int goalIndex);
    float Heuristic(const Vec3& a, const Vec3& b) const;
    void RebuildConnections();
};

// ============================================
// Path
// ============================================
struct Path {
    std::vector<Vec3> waypoints;
    int currentWaypoint;
    bool completed;
    
    Path() : currentWaypoint(0), completed(false) {}
    
    bool IsValid() const { return !waypoints.empty(); }
    Vec3 GetCurrentWaypoint() const;
    Vec3 GetNextWaypoint();
    bool HasReachedEnd() const;
    void Reset();
};

// ============================================
// Steering Behaviors
// ============================================
class SteeringBehavior {
public:
    // Seek towards target
    static Vec3 Seek(const Vec3& position, const Vec3& velocity, const Vec3& target, float maxSpeed);
    
    // Flee from target
    static Vec3 Flee(const Vec3& position, const Vec3& velocity, const Vec3& target, float maxSpeed);
    
    // Arrive at target with deceleration
    static Vec3 Arrive(const Vec3& position, const Vec3& velocity, const Vec3& target, 
                        float maxSpeed, float slowingRadius);
    
    // Wander randomly
    static Vec3 Wander(const Vec3& velocity, float wanderRadius, float wanderDistance, float wanderJitter);
    
    // Avoid obstacles
    static Vec3 AvoidObstacles(const Vec3& position, const Vec3& velocity, 
                               const std::vector<Vec3>& obstacles, float maxSeeAhead, float maxAvoidForce);
    
    // Follow path
    static Vec3 FollowPath(Path& path, const Vec3& position, const Vec3& velocity, 
                           float maxSpeed, float waypointRadius);
    
    // Separation from other agents
    static Vec3 Separation(const Vec3& position, const std::vector<Vec3>& neighborPositions, float separationRadius);
    
    // Cohesion with group
    static Vec3 Cohesion(const Vec3& position, const std::vector<Vec3>& neighborPositions, float cohesionRadius);
    
    // Alignment with group direction
    static Vec3 Alignment(const Vec3& velocity, const std::vector<Vec3>& neighborVelocities, float alignmentRadius);
};

// ============================================
// AI Agent
// ============================================
class AIAgent {
private:
    Vec3 position;
    Vec3 velocity;
    Vec3 target;
    float maxSpeed;
    float maxForce;
    float mass;
    
    Path currentPath;
    bool hasTarget;
    
    // Behavior weights
    float seekWeight;
    float fleeWeight;
    float wanderWeight;
    float avoidWeight;
    float separationWeight;
    
public:
    AIAgent(const Vec3& startPos, float maxSpeed = 5.0f);
    
    // Movement
    void SetTarget(const Vec3& target);
    void ClearTarget();
    bool HasTarget() const { return hasTarget; }
    
    // Path following
    void SetPath(const Path& path);
    bool IsPathComplete() const { return currentPath.completed; }
    
    // Update
    void Update(float deltaTime, const NavigationMesh* navMesh = nullptr);
    
    // Steering
    void ApplyForce(const Vec3& force);
    Vec3 CalculateSteering(const NavigationMesh* navMesh);
    
    // Getters
    Vec3 GetPosition() const { return position; }
    Vec3 GetVelocity() const { return velocity; }
    void SetPosition(const Vec3& pos) { position = pos; }
    void SetMaxSpeed(float speed) { maxSpeed = speed; }
    
    // Debug
    void DrawDebug() const;
};

// ============================================
// AI System
// ============================================
class AISystem {
private:
    std::unique_ptr<NavigationMesh> navMesh;
    std::vector<std::unique_ptr<AIAgent>> agents;
    
public:
    AISystem();
    ~AISystem();
    
    // Navigation mesh
    void GenerateNavMesh(const class World& world, float spacing = 1.0f);
    NavigationMesh* GetNavMesh() const { return navMesh.get(); }
    
    // Agent management
    AIAgent* CreateAgent(const Vec3& position, float maxSpeed = 5.0f);
    void DestroyAgent(AIAgent* agent);
    
    // Updates
    void Update(float deltaTime);
    
    // Queries
    std::vector<AIAgent*> GetAgentsInRadius(const Vec3& center, float radius) const;
    
    // Stats
    size_t GetAgentCount() const { return agents.size(); }
    
    void Clear();
};

} // namespace vge