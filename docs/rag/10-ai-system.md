# AI System

The engine includes AI systems for NPCs, enemies, and autonomous agents.

## Behavior Trees

```cpp
#include "ai/behavior_tree.h"

// Create behavior tree
vge::BehaviorTree tree;

// Define nodes
auto* root = tree.CreateNode<SelectorNode>("Root");

// Combat branch
auto* combat = tree.CreateNode<SequenceNode>("Combat");
auto* seeEnemy = tree.CreateNode<ConditionNode>("SeeEnemy");
seeEnemy->SetCondition([](AIContext* ctx) {
    return ctx->CanSeeEnemy();
});
auto* attack = tree.CreateNode<ActionNode>("Attack");
attack->SetAction([](AIContext* ctx) {
    ctx->Attack();
    return NodeStatus::Success;
});
combat->AddChild(seeEnemy);
combat->AddChild(attack);

// Patrol branch
auto* patrol = tree.CreateNode<SequenceNode>("Patrol");
auto* hasPatrol = tree.CreateNode<ConditionNode>("HasPatrol");
hasPatrol->SetCondition([](AIContext* ctx) {
    return ctx->HasPatrolPath();
});
auto* moveToWaypoint = tree.CreateNode<ActionNode>("MoveToWaypoint");
moveToWaypoint->SetAction([](AIContext* ctx) {
    return ctx->MoveToNextWaypoint() ? NodeStatus::Success : NodeStatus::Running;
});
patrol->AddChild(hasPatrol);
patrol->AddChild(moveToWaypoint);

// Idle fallback
auto* idle = tree.CreateNode<ActionNode>("Idle");
idle->SetAction([](AIContext* ctx) {
    ctx->PlayIdleAnimation();
    return NodeStatus::Success;
});

// Build tree
root->AddChild(combat);
root->AddChild(patrol);
root->AddChild(idle);

tree.SetRoot(root);

// Update AI
void Update(float dt) {
    AIContext context;
    context.entity = enemyEntity;
    context.world = &world;
    context.player = &player;
    
    tree.Update(&context);
}
```

## Node Types

```cpp
// Selector: Returns success if ANY child succeeds (OR)
auto* selector = tree.CreateNode<SelectorNode>("Selector");

// Sequence: Returns success if ALL children succeed (AND)
auto* sequence = tree.CreateNode<SequenceNode>("Sequence");

// Parallel: Runs all children simultaneously
auto* parallel = tree.CreateNode<ParallelNode>("Parallel");
parallel->SetSuccessPolicy(ParallelPolicy::All);  // All must succeed
parallel->SetFailurePolicy(ParallelPolicy::One);   // Any failure fails

// Inverter: Inverts child result
auto* inverter = tree.CreateNode<InverterNode>("Inverter");

// Repeater: Repeats child N times or forever
auto* repeater = tree.CreateNode<RepeaterNode>("Repeater");
repeater->SetCount(5);  // Repeat 5 times, -1 = forever

// Succeeder: Always returns success
auto* succeeder = tree.CreateNode<SucceederNode>("Succeeder");

// Condition: Checks a condition
auto* condition = tree.CreateNode<ConditionNode>("Condition");
condition->SetCondition([](AIContext* ctx) {
    return ctx->health > 50;
});

// Action: Performs an action
auto* action = tree.CreateNode<ActionNode>("Action");
action->SetAction([](AIContext* ctx) {
    ctx->MoveTo(ctx->targetPos);
    return NodeStatus::Running;  // Still moving
});
```

## State Machine

```cpp
#include "ai/ai_system.h"

// Define states
enum class EnemyState {
    Idle,
    Patrol,
    Chase,
    Attack,
    Flee,
    Dead
};

class EnemyAI {
    EnemyState currentState = EnemyState::Idle;
    float stateTimer = 0.0f;
    
public:
    void Update(float dt, AIContext* ctx) {
        stateTimer += dt;
        
        switch (currentState) {
            case EnemyState::Idle:
                UpdateIdle(dt, ctx);
                break;
            case EnemyState::Patrol:
                UpdatePatrol(dt, ctx);
                break;
            case EnemyState::Chase:
                UpdateChase(dt, ctx);
                break;
            case EnemyState::Attack:
                UpdateAttack(dt, ctx);
                break;
            case EnemyState::Flee:
                UpdateFlee(dt, ctx);
                break;
            case EnemyState::Dead:
                // Do nothing
                break;
        }
    }
    
private:
    void UpdateIdle(float dt, AIContext* ctx) {
        ctx->PlayAnimation("idle");
        
        // Transition to patrol after delay
        if (stateTimer > 3.0f) {
            ChangeState(EnemyState::Patrol);
        }
        
        // Transition to chase if player detected
        if (ctx->CanSeePlayer()) {
            ChangeState(EnemyState::Chase);
        }
    }
    
    void UpdatePatrol(float dt, AIContext* ctx) {
        ctx->PlayAnimation("walk");
        
        // Move to next waypoint
        if (ctx->MoveToNextWaypoint()) {
            ChangeState(EnemyState::Idle);
        }
        
        // Transition to chase if player detected
        if (ctx->CanSeePlayer()) {
            ChangeState(EnemyState::Chase);
        }
    }
    
    void UpdateChase(float dt, AIContext* ctx) {
        ctx->PlayAnimation("run");
        
        // Move toward player
        ctx->MoveTo(ctx->playerPos);
        
        float distToPlayer = vge::Vec3::Distance(ctx->GetPosition(), ctx->playerPos);
        
        // Transition to attack if close enough
        if (distToPlayer < ctx->attackRange) {
            ChangeState(EnemyState::Attack);
        }
        
        // Transition to idle if player lost
        if (!ctx->CanSeePlayer() && stateTimer > 5.0f) {
            ChangeState(EnemyState::Idle);
        }
    }
    
    void UpdateAttack(float dt, AIContext* ctx) {
        ctx->PlayAnimation("attack");
        
        // Face player
        ctx->LookAt(ctx->playerPos);
        
        // Attack
        if (stateTimer > ctx->attackCooldown) {
            ctx->Attack();
            stateTimer = 0;
        }
        
        float distToPlayer = vge::Vec3::Distance(ctx->GetPosition(), ctx->playerPos);
        
        // Transition to chase if player moves away
        if (distToPlayer > ctx->attackRange * 1.5f) {
            ChangeState(EnemyState::Chase);
        }
        
        // Transition to flee if low health
        if (ctx->health < ctx->maxHealth * 0.2f) {
            ChangeState(EnemyState::Flee);
        }
    }
    
    void UpdateFlee(float dt, AIContext* ctx) {
        ctx->PlayAnimation("run");
        
        // Run away from player
        vge::Vec3 fleeDir = (ctx->GetPosition() - ctx->playerPos).Normalized();
        ctx->MoveInDirection(fleeDir);
        
        // Transition to idle if safe
        float distToPlayer = vge::Vec3::Distance(ctx->GetPosition(), ctx->playerPos);
        if (distToPlayer > 30.0f) {
            ChangeState(EnemyState::Idle);
        }
    }
    
    void ChangeState(EnemyState newState) {
        // Exit current state
        OnExitState(currentState);
        
        // Enter new state
        currentState = newState;
        stateTimer = 0.0f;
        OnEnterState(newState);
    }
    
    void OnEnterState(EnemyState state) {
        switch (state) {
            case EnemyState::Chase:
                // Alert nearby enemies
                AlertNearbyEnemies();
                break;
            case EnemyState::Dead:
                // Drop loot, etc.
                break;
            default:
                break;
        }
    }
    
    void OnExitState(EnemyState state) {
        // Cleanup
    }
};
```

## Pathfinding

```cpp
#include "ai/ai_system.h"

// A* Pathfinding
class Pathfinder {
    vge::World* world;
    
public:
    std::vector<vge::Vec3> FindPath(vge::Vec3 start, vge::Vec3 goal) {
        // Convert to grid coordinates
        Node startNode = WorldToNode(start);
        Node goalNode = WorldToNode(goal);
        
        // A* algorithm
        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openSet;
        std::unordered_set<Node> closedSet;
        std::unordered_map<Node, Node> cameFrom;
        std::unordered_map<Node, float> gScore;
        
        openSet.push(startNode);
        gScore[startNode] = 0;
        
        while (!openSet.empty()) {
            Node current = openSet.top();
            openSet.pop();
            
            if (current == goalNode) {
                return ReconstructPath(cameFrom, current);
            }
            
            closedSet.insert(current);
            
            for (Node neighbor : GetNeighbors(current)) {
                if (closedSet.find(neighbor) != closedSet.end()) continue;
                
                float tentativeG = gScore[current] + Distance(current, neighbor);
                
                if (gScore.find(neighbor) == gScore.end() || tentativeG < gScore[neighbor]) {
                    cameFrom[neighbor] = current;
                    gScore[neighbor] = tentativeG;
                    float fScore = tentativeG + Heuristic(neighbor, goalNode);
                    neighbor.fScore = fScore;
                    openSet.push(neighbor);
                }
            }
        }
        
        return {}; // No path found
    }
    
private:
    float Heuristic(Node a, Node b) {
        // Manhattan distance for grid
        return abs(a.x - b.x) + abs(a.y - b.y) + abs(a.z - b.z);
    }
    
    std::vector<Node> GetNeighbors(Node node) {
        std::vector<Node> neighbors;
        
        // Check 6 directions (up, down, north, south, east, west)
        const int dx[] = {0, 0, 0, 0, 1, -1};
        const int dy[] = {1, -1, 0, 0, 0, 0};
        const int dz[] = {0, 0, 1, -1, 0, 0};
        
        for (int i = 0; i < 6; ++i) {
            Node neighbor(node.x + dx[i], node.y + dy[i], node.z + dz[i]);
            
            // Check if walkable
            if (IsWalkable(neighbor)) {
                neighbors.push_back(neighbor);
            }
        }
        
        return neighbors;
    }
    
    bool IsWalkable(Node node) {
        // Check if block is solid
        vge::BlockType block = world->GetBlock(node.x, node.y, node.z);
        if (world->IsBlockSolid(block)) return false;
        
        // Check if there's ground below
        vge::BlockType below = world->GetBlock(node.x, node.y - 1, node.z);
        if (!world->IsBlockSolid(below)) return false;
        
        return true;
    }
    
    std::vector<vge::Vec3> ReconstructPath(
        std::unordered_map<Node, Node>& cameFrom,
        Node current
    ) {
        std::vector<vge::Vec3> path;
        path.push_back(NodeToWorld(current));
        
        while (cameFrom.find(current) != cameFrom.end()) {
            current = cameFrom[current];
            path.push_back(NodeToWorld(current));
        }
        
        std::reverse(path.begin(), path.end());
        return path;
    }
};

// Navigation mesh (for open areas)
class NavMesh {
    struct NavNode {
        vge::Vec3 position;
        std::vector<int> connections;
        float cost;
    };
    
    std::vector<NavNode> nodes;
    
public:
    void Build(vge::World* world, vge::Vec3 center, float radius) {
        // Sample walkable points
        for (float x = -radius; x < radius; x += 2.0f) {
            for (float z = -radius; z < radius; z += 2.0f) {
                vge::Vec3 pos = center + vge::Vec3(x, 0, z);
                
                // Find ground height
                float groundY = FindGroundHeight(world, pos);
                pos.y = groundY;
                
                // Check if walkable
                if (IsWalkable(world, pos)) {
                    NavNode node;
                    node.position = pos;
                    nodes.push_back(node);
                }
            }
        }
        
        // Connect nearby nodes
        for (int i = 0; i < nodes.size(); ++i) {
            for (int j = i + 1; j < nodes.size(); ++j) {
                float dist = vge::Vec3::Distance(nodes[i].position, nodes[j].position);
                if (dist < 3.0f) {
                    nodes[i].connections.push_back(j);
                    nodes[j].connections.push_back(i);
                }
            }
        }
    }
    
    std::vector<vge::Vec3> FindPath(vge::Vec3 start, vge::Vec3 goal) {
        // Find nearest nodes
        int startNode = FindNearestNode(start);
        int goalNode = FindNearestNode(goal);
        
        // A* on nav mesh
        // ... (similar to grid pathfinding)
    }
};
```

## AI Perception

```cpp
class AIPerception {
    vge::Vec3 position;
    vge::Vec3 forward;
    float viewDistance = 20.0f;
    float viewAngle = 120.0f;  // Degrees
    
public:
    bool CanSee(const vge::Vec3& targetPos) {
        // Distance check
        float dist = vge::Vec3::Distance(position, targetPos);
        if (dist > viewDistance) return false;
        
        // Angle check
        vge::Vec3 toTarget = (targetPos - position).Normalized();
        float angle = acos(forward.Dot(toTarget)) * 180.0f / M_PI;
        if (angle > viewAngle / 2.0f) return false;
        
        // Raycast check (line of sight)
        vge::Ray ray(position, toTarget);
        vge::RaycastHit hit;
        if (world.Raycast(ray, dist, hit)) {
            return hit.position == targetPos; // Can see if nothing blocks
        }
        
        return true;
    }
    
    bool CanHear(const vge::Vec3& soundPos, float loudness) {
        float dist = vge::Vec3::Distance(position, soundPos);
        float hearDistance = loudness * 10.0f;
        return dist <= hearDistance;
    }
    
    void UpdateSenses(float dt) {
        // Check vision
        if (CanSee(player.GetPosition())) {
            lastSeenPlayerPos = player.GetPosition();
            timeSinceSeenPlayer = 0;
            knowsPlayerLocation = true;
        } else {
            timeSinceSeenPlayer += dt;
            if (timeSinceSeenPlayer > 5.0f) {
                knowsPlayerLocation = false;
            }
        }
        
        // Check hearing
        // ... (process sound events)
    }
};
```

## AI Spawning

```cpp
class AISpawner {
    struct SpawnPoint {
        vge::Vec3 position;
        std::string enemyType;
        float spawnRadius;
        int maxEnemies;
        float spawnInterval;
        float lastSpawnTime;
    };
    
    std::vector<SpawnPoint> spawnPoints;
    std::vector<vge::EntityID> activeEnemies;
    
public:
    void AddSpawnPoint(const SpawnPoint& point) {
        spawnPoints.push_back(point);
    }
    
    void Update(float dt, float currentTime) {
        for (auto& point : spawnPoints) {
            // Check if it's time to spawn
            if (currentTime - point.lastSpawnTime < point.spawnInterval) continue;
            
            // Count enemies near this spawn point
            int nearbyEnemies = CountNearbyEnemies(point.position, point.spawnRadius);
            if (nearbyEnemies >= point.maxEnemies) continue;
            
            // Check if player is nearby (don't spawn if too far)
            float distToPlayer = vge::Vec3::Distance(point.position, player.GetPosition());
            if (distToPlayer > 50.0f) continue;
            
            // Spawn enemy
            SpawnEnemy(point);
            point.lastSpawnTime = currentTime;
        }
    }
    
private:
    void SpawnEnemy(const SpawnPoint& point) {
        vge::EntityID enemy = entityManager.CreateEntity();
        
        // Random position within radius
        float angle = (rand() / (float)RAND_MAX) * 2 * M_PI;
        float radius = (rand() / (float)RAND_MAX) * point.spawnRadius;
        vge::Vec3 offset(cos(angle) * radius, 0, sin(angle) * radius);
        vge::Vec3 spawnPos = point.position + offset;
        
        // Setup enemy components
        auto* transform = entityManager.AddComponent<TransformComponent>(enemy);
        transform->position = spawnPos;
        
        auto* health = entityManager.AddComponent<HealthComponent>(enemy);
        health->maxHealth = 50;
        health->currentHealth = 50;
        
        auto* ai = entityManager.AddComponent<AIComponent>(enemy);
        ai->behaviorTree = LoadBehaviorTree(point.enemyType);
        
        activeEnemies.push_back(enemy);
    }
};
```

## AI Debugging

```cpp
class AIDebugger {
    bool showDebugInfo = false;
    
public:
    void ToggleDebug() {
        showDebugInfo = !showDebugInfo;
    }
    
    void Render(vge::Renderer* renderer, const AIContext* ctx) {
        if (!showDebugInfo) return;
        
        // Draw vision cone
        vge::Vec3 forward = ctx->GetForward();
        float halfAngle = ctx->viewAngle * 0.5f * M_PI / 180.0f;
        
        for (float angle = -halfAngle; angle <= halfAngle; angle += 0.1f) {
            vge::Vec3 dir;
            dir.x = forward.x * cos(angle) - forward.z * sin(angle);
            dir.z = forward.x * sin(angle) + forward.z * cos(angle);
            dir.y = forward.y;
            
            vge::Vec3 end = ctx->GetPosition() + dir * ctx->viewDistance;
            renderer->DrawLine(ctx->GetPosition(), end, vge::Vec4(0, 1, 0, 0.3f));
        }
        
        // Draw path
        if (!ctx->currentPath.empty()) {
            for (size_t i = 0; i < ctx->currentPath.size() - 1; ++i) {
                renderer->DrawLine(
                    ctx->currentPath[i],
                    ctx->currentPath[i + 1],
                    vge::Vec4(1, 0, 0, 1)
                );
            }
        }
        
        // Draw state text
        renderer->DrawText(
            ctx->GetPosition() + vge::Vec3(0, 2, 0),
            StateToString(ctx->currentState),
            vge::Vec4(1, 1, 0, 1)
        );
    }
};
```
