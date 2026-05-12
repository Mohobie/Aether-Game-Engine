#include "ai_system.h"
#include "voxel/world.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <queue>

namespace vge {

// ============================================
// Navigation Mesh Implementation
// ============================================

NavigationMesh::NavigationMesh() : nodeSpacing(1.0f) {}

void NavigationMesh::GenerateGrid(const Vec3& min, const Vec3& max, float spacing) {
    boundsMin = min;
    boundsMax = max;
    nodeSpacing = spacing;
    
    nodes.clear();
    
    int xCount = static_cast<int>((max.x - min.x) / spacing) + 1;
    int zCount = static_cast<int>((max.z - min.z) / spacing) + 1;
    
    for (int x = 0; x < xCount; ++x) {
        for (int z = 0; z < zCount; ++z) {
            Vec3 pos(min.x + x * spacing, min.y, min.z + z * spacing);
            AddNode(pos, true);
        }
    }
    
    RebuildConnections();
    
    std::cout << "[NavMesh] Generated grid: " << nodes.size() << " nodes (" 
              << xCount << "x" << zCount << ")" << std::endl;
}

void NavigationMesh::GenerateFromWorld(const World& world, float spacing) {
    // Would scan world for walkable areas
    // For now, just create a simple grid
    GenerateGrid(Vec3(-50, 0, -50), Vec3(50, 0, 50), spacing);
}

int NavigationMesh::AddNode(const Vec3& position, bool walkable) {
    NavNode node;
    node.position = position;
    node.walkable = walkable;
    
    int index = nodes.size();
    nodes.push_back(node);
    return index;
}

NavNode* NavigationMesh::GetNode(int index) {
    if (index >= 0 && index < nodes.size()) {
        return &nodes[index];
    }
    return nullptr;
}

void NavigationMesh::AddConnection(int from, int to) {
    if (from >= 0 && from < nodes.size() && to >= 0 && to < nodes.size()) {
        // Check if connection already exists
        auto& neighbors = nodes[from].neighbors;
        if (std::find(neighbors.begin(), neighbors.end(), to) == neighbors.end()) {
            neighbors.push_back(to);
        }
    }
}

void NavigationMesh::RemoveConnection(int from, int to) {
    if (from >= 0 && from < nodes.size()) {
        auto& neighbors = nodes[from].neighbors;
        neighbors.erase(std::remove(neighbors.begin(), neighbors.end(), to), neighbors.end());
    }
}

void NavigationMesh::RebuildConnections() {
    // Connect nodes that are close to each other
    for (int i = 0; i < nodes.size(); ++i) {
        nodes[i].neighbors.clear();
        
        for (int j = 0; j < nodes.size(); ++j) {
            if (i == j) continue;
            
            float dist = (nodes[i].position - nodes[j].position).length();
            if (dist <= nodeSpacing * 1.5f) {
                AddConnection(i, j);
            }
        }
    }
}

int NavigationMesh::FindNearestNode(const Vec3& position) const {
    int nearest = -1;
    float minDist = 999999.0f;
    
    for (int i = 0; i < nodes.size(); ++i) {
        float dist = (nodes[i].position - position).length();
        if (dist < minDist) {
            minDist = dist;
            nearest = i;
        }
    }
    
    return nearest;
}

std::vector<int> NavigationMesh::FindNodesInRadius(const Vec3& center, float radius) const {
    std::vector<int> result;
    
    for (int i = 0; i < nodes.size(); ++i) {
        if ((nodes[i].position - center).length() <= radius) {
            result.push_back(i);
        }
    }
    
    return result;
}

float NavigationMesh::Heuristic(const Vec3& a, const Vec3& b) const {
    // Euclidean distance
    return (a - b).length();
}

std::vector<int> NavigationMesh::AStar(int startIndex, int goalIndex) {
    if (startIndex < 0 || goalIndex < 0 || startIndex >= nodes.size() || goalIndex >= nodes.size()) {
        return {};
    }
    
    // Reset nodes
    for (auto& node : nodes) {
        node.gCost = 999999.0f;
        node.hCost = 0;
        node.parentIndex = -1;
        node.visited = false;
    }
    
    // Priority queue: pair<fCost, nodeIndex>
    auto cmp = [&](int a, int b) {
        return nodes[a].fCost() > nodes[b].fCost();
    };
    std::priority_queue<int, std::vector<int>, decltype(cmp)> openSet(cmp);
    
    nodes[startIndex].gCost = 0;
    nodes[startIndex].hCost = Heuristic(nodes[startIndex].position, nodes[goalIndex].position);
    openSet.push(startIndex);
    
    while (!openSet.empty()) {
        int current = openSet.top();
        openSet.pop();
        
        if (current == goalIndex) {
            // Reconstruct path
            std::vector<int> path;
            int node = goalIndex;
            while (node != -1) {
                path.push_back(node);
                node = nodes[node].parentIndex;
            }
            std::reverse(path.begin(), path.end());
            return path;
        }
        
        if (nodes[current].visited) continue;
        nodes[current].visited = true;
        
        for (int neighbor : nodes[current].neighbors) {
            if (nodes[neighbor].visited || !nodes[neighbor].walkable) continue;
            
            float tentativeG = nodes[current].gCost + 
                (nodes[current].position - nodes[neighbor].position).length();
            
            if (tentativeG < nodes[neighbor].gCost) {
                nodes[neighbor].parentIndex = current;
                nodes[neighbor].gCost = tentativeG;
                nodes[neighbor].hCost = Heuristic(nodes[neighbor].position, nodes[goalIndex].position);
                openSet.push(neighbor);
            }
        }
    }
    
    return {}; // No path found
}

std::vector<Vec3> NavigationMesh::FindPath(const Vec3& start, const Vec3& goal) const {
    int startNode = FindNearestNode(start);
    int goalNode = FindNearestNode(goal);
    
    if (startNode == -1 || goalNode == -1) {
        return {};
    }
    
    std::vector<int> nodePath = const_cast<NavigationMesh*>(this)->AStar(startNode, goalNode);
    
    std::vector<Vec3> path;
    for (int nodeIndex : nodePath) {
        path.push_back(nodes[nodeIndex].position);
    }
    
    return path;
}

bool NavigationMesh::IsWalkable(const Vec3& position) const {
    int node = FindNearestNode(position);
    if (node >= 0 && node < nodes.size()) {
        return nodes[node].walkable;
    }
    return false;
}

void NavigationMesh::Visualize() const {
    std::cout << "[NavMesh] Nodes: " << nodes.size() << std::endl;
    for (int i = 0; i < nodes.size(); ++i) {
        const NavNode& node = nodes[i];
        std::cout << "  [" << i << "] (" << node.position.x << ", " << node.position.z << ") "
                  << (node.walkable ? "walkable" : "blocked") << " - "
                  << node.neighbors.size() << " connections" << std::endl;
    }
}

// ============================================
// Path Implementation
// ============================================

Vec3 AIPath::GetCurrentWaypoint() const {
    if (currentWaypoint < waypoints.size()) {
        return waypoints[currentWaypoint];
    }
    return Vec3(0, 0, 0);
}

Vec3 AIPath::GetNextWaypoint() {
    if (currentWaypoint < waypoints.size() - 1) {
        currentWaypoint++;
        return waypoints[currentWaypoint];
    }
    completed = true;
    return waypoints.empty() ? Vec3(0, 0, 0) : waypoints.back();
}

bool AIPath::HasReachedEnd() const {
    return completed || currentWaypoint >= waypoints.size() - 1;
}

void AIPath::Reset() {
    currentWaypoint = 0;
    completed = false;
}

// ============================================
// Steering Behaviors Implementation
// ============================================

Vec3 SteeringBehavior::Seek(const Vec3& position, const Vec3& velocity, const Vec3& target, float maxSpeed) {
    Vec3 desired = target - position;
    desired = desired.normalize() * maxSpeed;
    return desired - velocity;
}

Vec3 SteeringBehavior::Flee(const Vec3& position, const Vec3& velocity, const Vec3& target, float maxSpeed) {
    Vec3 desired = position - target;
    desired = desired.normalize() * maxSpeed;
    return desired - velocity;
}

Vec3 SteeringBehavior::Arrive(const Vec3& position, const Vec3& velocity, const Vec3& target, 
                               float maxSpeed, float slowingRadius) {
    Vec3 desired = target - position;
    float dist = desired.length();
    
    if (dist < slowingRadius) {
        desired = desired.normalize() * (maxSpeed * (dist / slowingRadius));
    } else {
        desired = desired.normalize() * maxSpeed;
    }
    
    return desired - velocity;
}

Vec3 SteeringBehavior::Wander(const Vec3& velocity, float wanderRadius, float wanderDistance, float wanderJitter) {
    // Simplified wander
    Vec3 circleCenter = velocity.normalize() * wanderDistance;
    
    // Random displacement
    static float theta = 0;
    theta += (rand() % 100 - 50) * 0.01f * wanderJitter;
    
    Vec3 displacement(std::cos(theta) * wanderRadius, 0, std::sin(theta) * wanderRadius);
    
    return circleCenter + displacement;
}

Vec3 SteeringBehavior::AvoidObstacles(const Vec3& position, const Vec3& velocity, 
                                       const std::vector<Vec3>& obstacles, float maxSeeAhead, float maxAvoidForce) {
    Vec3 ahead = position + velocity.normalize() * maxSeeAhead;
    Vec3 avoidance(0, 0, 0);
    
    for (const Vec3& obstacle : obstacles) {
        float dist = (ahead - obstacle).length();
        if (dist < maxSeeAhead) {
            Vec3 avoidDir = ahead - obstacle;
            avoidDir = avoidDir.normalize();
            avoidance = avoidance + avoidDir * maxAvoidForce * (1.0f - dist / maxSeeAhead);
        }
    }
    
    return avoidance;
}

Vec3 SteeringBehavior::FollowPath(AIPath& path, const Vec3& position, const Vec3& velocity, 
                                   float maxSpeed, float waypointRadius) {
    if (path.HasReachedEnd()) {
        return Vec3(0, 0, 0);
    }
    
    Vec3 currentWaypoint = path.GetCurrentWaypoint();
    float dist = (currentWaypoint - position).length();
    
    if (dist < waypointRadius) {
        path.GetNextWaypoint();
        if (!path.HasReachedEnd()) {
            currentWaypoint = path.GetCurrentWaypoint();
        }
    }
    
    return Seek(position, velocity, currentWaypoint, maxSpeed);
}

Vec3 SteeringBehavior::Separation(const Vec3& position, const std::vector<Vec3>& neighborPositions, float separationRadius) {
    Vec3 separation(0, 0, 0);
    int count = 0;
    
    for (const Vec3& neighbor : neighborPositions) {
        Vec3 diff = position - neighbor;
        float dist = diff.length();
        
        if (dist > 0 && dist < separationRadius) {
            separation = separation + diff.normalize() / dist;
            count++;
        }
    }
    
    if (count > 0) {
        separation = separation / count;
    }
    
    return separation;
}

Vec3 SteeringBehavior::Cohesion(const Vec3& position, const std::vector<Vec3>& neighborPositions, float cohesionRadius) {
    Vec3 center(0, 0, 0);
    int count = 0;
    
    for (const Vec3& neighbor : neighborPositions) {
        if ((position - neighbor).length() < cohesionRadius) {
            center = center + neighbor;
            count++;
        }
    }
    
    if (count > 0) {
        center = center / count;
        return center - position;
    }
    
    return Vec3(0, 0, 0);
}

Vec3 SteeringBehavior::Alignment(const Vec3& velocity, const std::vector<Vec3>& neighborVelocities, float alignmentRadius) {
    Vec3 avgVelocity(0, 0, 0);
    int count = 0;
    
    for (const Vec3& vel : neighborVelocities) {
        if (vel.length() > 0) {
            avgVelocity = avgVelocity + vel;
            count++;
        }
    }
    
    if (count > 0) {
        avgVelocity = avgVelocity / count;
        return avgVelocity - velocity;
    }
    
    return Vec3(0, 0, 0);
}

// ============================================
// AI Agent Implementation
// ============================================

AIAgent::AIAgent(const Vec3& startPos, float maxSpeed)
    : position(startPos)
    , velocity(0, 0, 0)
    , target(startPos)
    , maxSpeed(maxSpeed)
    , maxForce(10.0f)
    , mass(1.0f)
    , hasTarget(false)
    , seekWeight(1.0f)
    , fleeWeight(0.0f)
    , wanderWeight(0.0f)
    , avoidWeight(1.0f)
    , separationWeight(0.5f) {}

void AIAgent::SetTarget(const Vec3& newTarget) {
    target = newTarget;
    hasTarget = true;
    
    // Request new path if navmesh available
    currentPath.Reset();
}

void AIAgent::ClearTarget() {
    hasTarget = false;
    currentPath.Reset();
}

void AIAgent::SetPath(const AIPath& path) {
    currentPath = path;
    currentPath.Reset();
}

void AIAgent::ApplyForce(const Vec3& force) {
    Vec3 clampedForce = force;
    float forceLen = clampedForce.length();
    if (forceLen > maxForce) {
        clampedForce = clampedForce.normalize() * maxForce;
    }
    
    Vec3 acceleration = clampedForce / mass;
    velocity = velocity + acceleration;
    
    float speed = velocity.length();
    if (speed > maxSpeed) {
        velocity = velocity.normalize() * maxSpeed;
    }
}

Vec3 AIAgent::CalculateSteering(const NavigationMesh* navMesh) {
    Vec3 steering(0, 0, 0);
    
    // Path following
    if (!currentPath.HasReachedEnd() && !currentPath.waypoints.empty()) {
        steering = steering + SteeringBehavior::FollowPath(currentPath, position, velocity, maxSpeed, 1.0f) * seekWeight;
    }
    
    // Seek target
    if (hasTarget && currentPath.waypoints.empty()) {
        // If no path, just seek directly
        if (navMesh) {
            // Find path through navmesh
            std::vector<Vec3> path = navMesh->FindPath(position, target);
            if (!path.empty()) {
                AIPath newPath;
                newPath.waypoints = path;
                SetPath(newPath);
            }
        }
        
        if (currentPath.waypoints.empty()) {
            steering = steering + SteeringBehavior::Seek(position, velocity, target, maxSpeed) * seekWeight;
        }
    }
    
    return steering;
}

void AIAgent::Update(float deltaTime, const NavigationMesh* navMesh) {
    Vec3 steering = CalculateSteering(navMesh);
    ApplyForce(steering);
    
    position = position + velocity * deltaTime;
}

void AIAgent::DrawDebug() const {
    std::cout << "[AIAgent] Pos: (" << position.x << ", " << position.y << ", " << position.z << ") "
              << "Vel: (" << velocity.x << ", " << velocity.y << ", " << velocity.z << ")" << std::endl;
}

// ============================================
// AI System Implementation
// ============================================

AISystem::AISystem() {}

AISystem::~AISystem() {
    Clear();
}

void AISystem::GenerateNavMesh(const World& world, float spacing) {
    navMesh = std::make_unique<NavigationMesh>();
    navMesh->GenerateFromWorld(world, spacing);
}

AIAgent* AISystem::CreateAgent(const Vec3& position, float maxSpeed) {
    auto agent = std::make_unique<AIAgent>(position, maxSpeed);
    AIAgent* ptr = agent.get();
    agents.push_back(std::move(agent));
    return ptr;
}

void AISystem::DestroyAgent(AIAgent* agent) {
    auto it = std::find_if(agents.begin(), agents.end(),
        [agent](const std::unique_ptr<AIAgent>& a) { return a.get() == agent; });
    if (it != agents.end()) {
        agents.erase(it);
    }
}

void AISystem::Update(float deltaTime) {
    for (auto& agent : agents) {
        agent->Update(deltaTime, navMesh.get());
    }
}

std::vector<AIAgent*> AISystem::GetAgentsInRadius(const Vec3& center, float radius) const {
    std::vector<AIAgent*> result;
    
    for (const auto& agent : agents) {
        if ((agent->GetPosition() - center).length() <= radius) {
            result.push_back(agent.get());
        }
    }
    
    return result;
}

void AISystem::Clear() {
    agents.clear();
    navMesh.reset();
}

} // namespace vge
