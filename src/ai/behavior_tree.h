#pragma once
#include "math/vec3.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

namespace vge {

// ============================================
// Behavior Tree - AI Decision Making
// ============================================

// Forward declarations
class BTNode;
class BTBlackboard;
class BehaviorTree;

// ============================================
// Node Status
// ============================================
enum class BTStatus {
    Success,    // Node completed successfully
    Failure,    // Node failed
    Running,    // Node is still executing
    Error       // Node encountered an error
};

// ============================================
// Base Behavior Tree Node
// ============================================
class BTNode {
protected:
    std::string name;
    std::vector<std::unique_ptr<BTNode>> children;
    BTNode* parent;
    
    // Runtime state
    bool initialized;
    int childIndex;  // Current child being processed
    
public:
    BTNode(const std::string& name = "Node");
    virtual ~BTNode() = default;
    
    // Core execution
    virtual BTStatus Update(float deltaTime, BTBlackboard& blackboard) = 0;
    virtual void Initialize(BTBlackboard& blackboard);
    virtual void Terminate(BTBlackboard& blackboard);
    
    // Lifecycle
    BTStatus Tick(float deltaTime, BTBlackboard& blackboard);
    void Reset();
    
    // Hierarchy
    void AddChild(std::unique_ptr<BTNode> child);
    void RemoveChild(BTNode* child);
    BTNode* GetParent() const { return parent; }
    const std::vector<std::unique_ptr<BTNode>>& GetChildren() const { return children; }
    int GetChildCount() const { return children.size(); }
    
    // Debug
    std::string GetName() const { return name; }
    virtual void PrintTree(int indent = 0) const;
    
protected:
    BTNode* GetCurrentChild();
    void ResetChildren();
};

// ============================================
// Composite Nodes (Control Flow)
// ============================================

// Sequence - Execute children in order until one fails
class BTSequence : public BTNode {
public:
    BTSequence(const std::string& name = "Sequence");
    BTStatus Update(float deltaTime, BTBlackboard& blackboard) override;
};

// Selector - Execute children in order until one succeeds
class BTSelector : public BTNode {
public:
    BTSelector(const std::string& name = "Selector");
    BTStatus Update(float deltaTime, BTBlackboard& blackboard) override;
};

// Parallel - Execute all children simultaneously
class BTParallel : public BTNode {
public:
    enum class Policy {
        RequireAll,     // All must succeed
        RequireOne,     // At least one must succeed
        RequireAllFail  // All must fail
    };
    
private:
    Policy successPolicy;
    Policy failurePolicy;
    
public:
    BTParallel(const std::string& name = "Parallel", 
               Policy successPolicy = Policy::RequireAll,
               Policy failurePolicy = Policy::RequireOne);
    
    BTStatus Update(float deltaTime, BTBlackboard& blackboard) override;
};

// Random Selector - Pick random child to execute
class BTRandomSelector : public BTNode {
private:
    int lastSelectedIndex;
    
public:
    BTRandomSelector(const std::string& name = "RandomSelector");
    BTStatus Update(float deltaTime, BTBlackboard& blackboard) override;
};

// ============================================
// Decorator Nodes (Modify Child Behavior)
// ============================================

// Inverter - Inverts child result
class BTInverter : public BTNode {
public:
    BTInverter(const std::string& name = "Inverter");
    BTStatus Update(float deltaTime, BTBlackboard& blackboard) override;
};

// Repeater - Repeats child N times or infinitely
class BTRepeater : public BTNode {
private:
    int maxRepeats;
    int currentCount;
    bool repeatForever;
    
public:
    BTRepeater(int maxRepeats = -1, const std::string& name = "Repeater");
    
    BTStatus Update(float deltaTime, BTBlackboard& blackboard) override;
    void Reset();
};

// Succeeder - Always returns success
class BTSucceeder : public BTNode {
public:
    BTSucceeder(const std::string& name = "Succeeder");
    BTStatus Update(float deltaTime, BTBlackboard& blackboard) override;
};

// Failer - Always returns failure
class BTFailer : public BTNode {
public:
    BTFailer(const std::string& name = "Failer");
    BTStatus Update(float deltaTime, BTBlackboard& blackboard) override;
};

// Limiter - Only allows child to execute N times
class BTLimiter : public BTNode {
private:
    int maxExecutions;
    int executionCount;
    
public:
    BTLimiter(int maxExecutions, const std::string& name = "Limiter");
    
    BTStatus Update(float deltaTime, BTBlackboard& blackboard) override;
    void Reset();
};

// Cooldown - Prevents child from executing for a period
class BTCooldown : public BTNode {
private:
    float cooldownTime;
    float currentTime;
    bool onCooldown;
    
public:
    BTCooldown(float cooldownTime, const std::string& name = "Cooldown");
    
    BTStatus Update(float deltaTime, BTBlackboard& blackboard) override;
    void Reset();
};

// ============================================
// Action Nodes (Leaf Nodes)
// ============================================

// Generic action using lambda
class BTAction : public BTNode {
private:
    std::function<BTStatus(float, BTBlackboard&)> action;
    
public:
    BTAction(const std::string& name, std::function<BTStatus(float, BTBlackboard&)> action);
    
    BTStatus Update(float deltaTime, BTBlackboard& blackboard) override;
};

// Wait action
class BTWait : public BTNode {
private:
    float waitTime;
    float currentTime;
    
public:
    BTWait(float time, const std::string& name = "Wait");
    
    BTStatus Update(float deltaTime, BTBlackboard& blackboard) override;
    void Reset();
};

// Move to position
class BTMoveTo : public BTNode {
private:
    float speed;
    float stoppingDistance;
    
public:
    BTMoveTo(float speed, float stoppingDistance = 0.1f, const std::string& name = "MoveTo");
    
    BTStatus Update(float deltaTime, BTBlackboard& blackboard) override;
};

// Patrol between waypoints
class BTPatrol : public BTNode {
private:
    float speed;
    float waitTime;
    float currentWaitTime;
    int currentWaypoint;
    bool waiting;
    
public:
    BTPatrol(float speed, float waitTime = 2.0f, const std::string& name = "Patrol");
    
    BTStatus Update(float deltaTime, BTBlackboard& blackboard) override;
    void Reset();
};

// Attack target
class BTAttack : public BTNode {
private:
    float attackRange;
    float attackCooldown;
    float currentCooldown;
    float damage;
    
public:
    BTAttack(float range, float cooldown, float damage, const std::string& name = "Attack");
    
    BTStatus Update(float deltaTime, BTBlackboard& blackboard) override;
    void Reset();
};

// Check if target is in range
class BTCheckDistance : public BTNode {
private:
    float checkRange;
    bool checkLessThan;
    
public:
    BTCheckDistance(float range, bool lessThan = true, const std::string& name = "CheckDistance");
    
    BTStatus Update(float deltaTime, BTBlackboard& blackboard) override;
};

// Check if can see target (line of sight)
class BTCheckLineOfSight : public BTNode {
private:
    float viewDistance;
    float viewAngle;
    
public:
    BTCheckLineOfSight(float distance, float angle = 120.0f, const std::string& name = "CheckLOS");
    
    BTStatus Update(float deltaTime, BTBlackboard& blackboard) override;
};

// Set blackboard value
class BTSetBlackboard : public BTNode {
private:
    std::string key;
    std::function<void(BTBlackboard&)> setter;
    
public:
    BTSetBlackboard(const std::string& key, std::function<void(BTBlackboard&)> setter, 
                    const std::string& name = "SetBlackboard");
    
    BTStatus Update(float deltaTime, BTBlackboard& blackboard) override;
};

// ============================================
// Blackboard - Shared Data Storage
// ============================================
class BTBlackboard {
private:
    // Typed storage
    std::unordered_map<std::string, float> floatValues;
    std::unordered_map<std::string, int> intValues;
    std::unordered_map<std::string, bool> boolValues;
    std::unordered_map<std::string, Vec3> vectorValues;
    std::unordered_map<std::string, std::string> stringValues;
    std::unordered_map<std::string, void*> pointerValues;
    
public:
    BTBlackboard();
    ~BTBlackboard();
    
    // Setters
    void SetFloat(const std::string& key, float value);
    void SetInt(const std::string& key, int value);
    void SetBool(const std::string& key, bool value);
    void SetVector(const std::string& key, const Vec3& value);
    void SetString(const std::string& key, const std::string& value);
    void SetPointer(const std::string& key, void* value);
    
    // Getters
    float GetFloat(const std::string& key, float defaultValue = 0.0f) const;
    int GetInt(const std::string& key, int defaultValue = 0) const;
    bool GetBool(const std::string& key, bool defaultValue = false) const;
    Vec3 GetVector(const std::string& key, const Vec3& defaultValue = Vec3()) const;
    std::string GetString(const std::string& key, const std::string& defaultValue = "") const;
    void* GetPointer(const std::string& key, void* defaultValue = nullptr) const;
    
    // Check if key exists
    bool HasKey(const std::string& key) const;
    
    // Remove key
    void RemoveKey(const std::string& key);
    
    // Clear all
    void Clear();
    
    // Debug
    void PrintContents() const;
};

// ============================================
// Behavior Tree
// ============================================
class BehaviorTree {
private:
    std::unique_ptr<BTNode> root;
    BTBlackboard blackboard;
    
    bool isRunning;
    float updateInterval;
    float currentTime;
    
public:
    BehaviorTree();
    ~BehaviorTree();
    
    // Setup
    void SetRoot(std::unique_ptr<BTNode> node);
    BTNode* GetRoot() const { return root.get(); }
    BTBlackboard& GetBlackboard() { return blackboard; }
    const BTBlackboard& GetBlackboard() const { return blackboard; }
    
    // Execution
    void Start();
    void Stop();
    void Update(float deltaTime);
    bool IsRunning() const { return isRunning; }
    
    // Settings
    void SetUpdateInterval(float interval) { updateInterval = interval; }
    float GetUpdateInterval() const { return updateInterval; }
    
    // Reset
    void Reset();
    
    // Debug
    void PrintTree() const;
    BTStatus GetLastStatus() const;
};

// ============================================
// Behavior Tree Builder (Fluent API)
// ============================================
class BTBuilder {
private:
    std::vector<BTNode*> nodeStack;
    std::unique_ptr<BehaviorTree> tree;
    
public:
    BTBuilder();
    ~BTBuilder();
    
    // Start building
    BTBuilder& CreateTree(const std::string& name = "BehaviorTree");
    
    // Composite nodes
    BTBuilder& Sequence(const std::string& name = "Sequence");
    BTBuilder& Selector(const std::string& name = "Selector");
    BTBuilder& Parallel(BTParallel::Policy successPolicy = BTParallel::Policy::RequireAll,
                        BTParallel::Policy failurePolicy = BTParallel::Policy::RequireOne,
                        const std::string& name = "Parallel");
    BTBuilder& RandomSelector(const std::string& name = "RandomSelector");
    
    // Decorators
    BTBuilder& Inverter(const std::string& name = "Inverter");
    BTBuilder& Repeater(int maxRepeats = -1, const std::string& name = "Repeater");
    BTBuilder& Succeeder(const std::string& name = "Succeeder");
    BTBuilder& Failer(const std::string& name = "Failer");
    BTBuilder& Limiter(int maxExecutions, const std::string& name = "Limiter");
    BTBuilder& Cooldown(float cooldownTime, const std::string& name = "Cooldown");
    
    // Actions
    BTBuilder& Action(const std::string& name, std::function<BTStatus(float, BTBlackboard&)> action);
    BTBuilder& Wait(float time, const std::string& name = "Wait");
    BTBuilder& MoveTo(float speed, float stoppingDistance = 0.1f, const std::string& name = "MoveTo");
    BTBuilder& Patrol(float speed, float waitTime = 2.0f, const std::string& name = "Patrol");
    BTBuilder& Attack(float range, float cooldown, float damage, const std::string& name = "Attack");
    BTBuilder& CheckDistance(float range, bool lessThan = true, const std::string& name = "CheckDistance");
    BTBuilder& CheckLineOfSight(float distance, float angle = 120.0f, const std::string& name = "CheckLOS");
    
    // Tree navigation
    BTBuilder& End();  // Pop current node from stack
    
    // Build
    std::unique_ptr<BehaviorTree> Build();
    
private:
    void PushNode(BTNode* node);
    void PopNode();
    BTNode* GetCurrentParent();
};

// ============================================
// Behavior Tree Factory (JSON-based)
// ============================================
class BTFactory {
public:
    // Create tree from JSON string
    static std::unique_ptr<BehaviorTree> CreateFromJSON(const std::string& json);
    
    // Create tree from JSON file
    static std::unique_ptr<BehaviorTree> CreateFromFile(const std::string& filepath);
    
    // Register custom node types
    static void RegisterNodeType(const std::string& typeName, 
                                  std::function<std::unique_ptr<BTNode>(const std::string&)> factory);
    
private:
    static std::unordered_map<std::string, std::function<std::unique_ptr<BTNode>(const std::string&)>> customFactories;
};

} // namespace vge