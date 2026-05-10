#include "behavior_tree.h"
#include <iostream>
#include <cmath>
#include <random>

namespace vge {

// ============================================
// BTNode Base Implementation
// ============================================

BTNode::BTNode(const std::string& name)
    : name(name)
    , parent(nullptr)
    , initialized(false)
    , childIndex(0) {}

void BTNode::Initialize(BTBlackboard& blackboard) {
    if (!initialized) {
        initialized = true;
        childIndex = 0;
        ResetChildren();
    }
    (void)blackboard;
}

void BTNode::Terminate(BTBlackboard& blackboard) {
    initialized = false;
    (void)blackboard;
}

BTStatus BTNode::Tick(float deltaTime, BTBlackboard& blackboard) {
    if (!initialized) {
        Initialize(blackboard);
    }
    
    BTStatus status = Update(deltaTime, blackboard);
    
    if (status != BTStatus::Running) {
        Terminate(blackboard);
        initialized = false;
    }
    
    return status;
}

void BTNode::Reset() {
    initialized = false;
    childIndex = 0;
    ResetChildren();
}

void BTNode::AddChild(std::unique_ptr<BTNode> child) {
    child->parent = this;
    children.push_back(std::move(child));
}

void BTNode::RemoveChild(BTNode* child) {
    children.erase(
        std::remove_if(children.begin(), children.end(),
            [child](const std::unique_ptr<BTNode>& c) { return c.get() == child; }),
        children.end()
    );
}

void BTNode::PrintTree(int indent) const {
    std::string prefix(indent * 2, ' ');
    std::cout << prefix << "[" << name << "]" << std::endl;
    
    for (const auto& child : children) {
        child->PrintTree(indent + 1);
    }
}

BTNode* BTNode::GetCurrentChild() {
    if (childIndex >= 0 && static_cast<size_t>(childIndex) < children.size()) {
        return children[childIndex].get();
    }
    return nullptr;
}

void BTNode::ResetChildren() {
    for (auto& child : children) {
        child->Reset();
    }
    childIndex = 0;
}

// ============================================
// Sequence Node
// ============================================

BTSequence::BTSequence(const std::string& name)
    : BTNode(name) {}

BTStatus BTSequence::Update(float deltaTime, BTBlackboard& blackboard) {
    while (static_cast<size_t>(childIndex) < children.size()) {
        BTNode* child = children[childIndex].get();
        BTStatus status = child->Tick(deltaTime, blackboard);
        
        switch (status) {
            case BTStatus::Failure:
                childIndex = 0;
                return BTStatus::Failure;
            case BTStatus::Success:
                childIndex++;
                break;
            case BTStatus::Running:
                return BTStatus::Running;
            case BTStatus::Error:
                childIndex = 0;
                return BTStatus::Error;
        }
    }
    
    childIndex = 0;
    return BTStatus::Success;
}

// ============================================
// Selector Node
// ============================================

BTSelector::BTSelector(const std::string& name)
    : BTNode(name) {}

BTStatus BTSelector::Update(float deltaTime, BTBlackboard& blackboard) {
    while (static_cast<size_t>(childIndex) < children.size()) {
        BTNode* child = children[childIndex].get();
        BTStatus status = child->Tick(deltaTime, blackboard);
        
        switch (status) {
            case BTStatus::Failure:
                childIndex++;
                break;
            case BTStatus::Success:
                childIndex = 0;
                return BTStatus::Success;
            case BTStatus::Running:
                return BTStatus::Running;
            case BTStatus::Error:
                childIndex = 0;
                return BTStatus::Error;
        }
    }
    
    childIndex = 0;
    return BTStatus::Failure;
}

// ============================================
// Parallel Node
// ============================================

BTParallel::BTParallel(const std::string& name, Policy successPolicy, Policy failurePolicy)
    : BTNode(name)
    , successPolicy(successPolicy)
    , failurePolicy(failurePolicy) {}

BTStatus BTParallel::Update(float deltaTime, BTBlackboard& blackboard) {
    int successCount = 0;
    int failureCount = 0;
    
    for (auto& child : children) {
        BTStatus status = child->Tick(deltaTime, blackboard);
        
        switch (status) {
            case BTStatus::Success:
                successCount++;
                break;
            case BTStatus::Failure:
                failureCount++;
                break;
            case BTStatus::Running:
                break;
            case BTStatus::Error:
                return BTStatus::Error;
        }
    }
    
    bool successCondition = false;
    switch (successPolicy) {
        case Policy::RequireAll:
            successCondition = (successCount == static_cast<int>(children.size()));
            break;
        case Policy::RequireOne:
            successCondition = (successCount >= 1);
            break;
        case Policy::RequireAllFail:
            successCondition = (successCount == 0);
            break;
    }
    
    if (successCondition) return BTStatus::Success;
    
    bool failureCondition = false;
    switch (failurePolicy) {
        case Policy::RequireAll:
            failureCondition = (failureCount == static_cast<int>(children.size()));
            break;
        case Policy::RequireOne:
            failureCondition = (failureCount >= 1);
            break;
        case Policy::RequireAllFail:
            failureCondition = (failureCount == static_cast<int>(children.size()));
            break;
    }
    
    if (failureCondition) return BTStatus::Failure;
    
    return BTStatus::Running;
}

// ============================================
// Random Selector
// ============================================

BTRandomSelector::BTRandomSelector(const std::string& name)
    : BTNode(name)
    , lastSelectedIndex(-1) {}

BTStatus BTRandomSelector::Update(float deltaTime, BTBlackboard& blackboard) {
    if (children.empty()) return BTStatus::Failure;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, children.size() - 1);
    
    int selectedIndex = dis(gen);
    lastSelectedIndex = selectedIndex;
    
    return children[selectedIndex]->Tick(deltaTime, blackboard);
}

// ============================================
// Decorator Nodes
// ============================================

BTInverter::BTInverter(const std::string& name) : BTNode(name) {}

BTStatus BTInverter::Update(float deltaTime, BTBlackboard& blackboard) {
    if (children.empty()) return BTStatus::Error;
    
    BTStatus status = children[0]->Tick(deltaTime, blackboard);
    
    if (status == BTStatus::Success) return BTStatus::Failure;
    if (status == BTStatus::Failure) return BTStatus::Success;
    return status;
}

BTRepeater::BTRepeater(int maxRepeats, const std::string& name)
    : BTNode(name)
    , maxRepeats(maxRepeats)
    , currentCount(0)
    , repeatForever(maxRepeats < 0) {}

BTStatus BTRepeater::Update(float deltaTime, BTBlackboard& blackboard) {
    if (children.empty()) return BTStatus::Error;
    
    BTStatus status = children[0]->Tick(deltaTime, blackboard);
    
    if (status == BTStatus::Running) return BTStatus::Running;
    
    currentCount++;
    
    if (!repeatForever && currentCount >= maxRepeats) {
        currentCount = 0;
        return status;
    }
    
    children[0]->Reset();
    return BTStatus::Running;
}

void BTRepeater::Reset() {
    currentCount = 0;
    BTNode::Reset();
}

BTSucceeder::BTSucceeder(const std::string& name) : BTNode(name) {}

BTStatus BTSucceeder::Update(float deltaTime, BTBlackboard& blackboard) {
    if (!children.empty()) children[0]->Tick(deltaTime, blackboard);
    return BTStatus::Success;
}

BTFailer::BTFailer(const std::string& name) : BTNode(name) {}

BTStatus BTFailer::Update(float deltaTime, BTBlackboard& blackboard) {
    if (!children.empty()) children[0]->Tick(deltaTime, blackboard);
    return BTStatus::Failure;
}

BTLimiter::BTLimiter(int maxExecutions, const std::string& name)
    : BTNode(name)
    , maxExecutions(maxExecutions)
    , executionCount(0) {}

BTStatus BTLimiter::Update(float deltaTime, BTBlackboard& blackboard) {
    if (children.empty()) return BTStatus::Error;
    if (executionCount >= maxExecutions) return BTStatus::Failure;
    
    executionCount++;
    return children[0]->Tick(deltaTime, blackboard);
}

void BTLimiter::Reset() {
    executionCount = 0;
    BTNode::Reset();
}

BTCooldown::BTCooldown(float cooldownTime, const std::string& name)
    : BTNode(name)
    , cooldownTime(cooldownTime)
    , currentTime(0.0f)
    , onCooldown(false) {}

BTStatus BTCooldown::Update(float deltaTime, BTBlackboard& blackboard) {
    if (children.empty()) return BTStatus::Error;
    
    if (onCooldown) {
        currentTime += deltaTime;
        if (currentTime >= cooldownTime) {
            onCooldown = false;
            currentTime = 0.0f;
        } else {
            return BTStatus::Failure;
        }
    }
    
    BTStatus status = children[0]->Tick(deltaTime, blackboard);
    
    if (status == BTStatus::Success || status == BTStatus::Failure) {
        onCooldown = true;
        currentTime = 0.0f;
    }
    
    return status;
}

void BTCooldown::Reset() {
    onCooldown = false;
    currentTime = 0.0f;
    BTNode::Reset();
}

// ============================================
// Action Nodes
// ============================================

BTAction::BTAction(const std::string& name, std::function<BTStatus(float, BTBlackboard&)> action)
    : BTNode(name)
    , action(action) {}

BTStatus BTAction::Update(float deltaTime, BTBlackboard& blackboard) {
    if (action) return action(deltaTime, blackboard);
    return BTStatus::Error;
}

BTWait::BTWait(float time, const std::string& name)
    : BTNode(name)
    , waitTime(time)
    , currentTime(0.0f) {}

BTStatus BTWait::Update(float deltaTime, BTBlackboard& blackboard) {
    currentTime += deltaTime;
    if (currentTime >= waitTime) {
        currentTime = 0.0f;
        std::cout << "[BTWait] Wait complete after " << waitTime << "s" << std::endl;
        return BTStatus::Success;
    }
    return BTStatus::Running;
    (void)blackboard;
}

void BTWait::Reset() {
    currentTime = 0.0f;
    BTNode::Reset();
}

BTMoveTo::BTMoveTo(float speed, float stoppingDistance, const std::string& name)
    : BTNode(name)
    , speed(speed)
    , stoppingDistance(stoppingDistance) {}

BTStatus BTMoveTo::Update(float deltaTime, BTBlackboard& blackboard) {
    Vec3 currentPos = blackboard.GetVector("position");
    Vec3 targetPos = blackboard.GetVector("targetPosition");
    
    Vec3 direction = targetPos - currentPos;
    float distance = direction.length();
    
    if (distance <= stoppingDistance) {
        std::cout << "[BTMoveTo] Reached target" << std::endl;
        return BTStatus::Success;
    }
    
    direction = direction.normalize();
    Vec3 newPos = currentPos + direction * speed * deltaTime;
    blackboard.SetVector("position", newPos);
    
    std::cout << "[BTMoveTo] Moving to target. Distance: " << distance << std::endl;
    return BTStatus::Running;
}

BTPatrol::BTPatrol(float speed, float waitTime, const std::string& name)
    : BTNode(name)
    , speed(speed)
    , waitTime(waitTime)
    , currentWaitTime(0.0f)
    , currentWaypoint(0)
    , waiting(false) {}

BTStatus BTPatrol::Update(float deltaTime, BTBlackboard& blackboard) {
    int waypointCount = blackboard.GetInt("waypointCount", 0);
    if (waypointCount == 0) return BTStatus::Failure;
    
    if (waiting) {
        currentWaitTime += deltaTime;
        if (currentWaitTime >= waitTime) {
            waiting = false;
            currentWaitTime = 0.0f;
            currentWaypoint = (currentWaypoint + 1) % waypointCount;
            std::cout << "[BTPatrol] Moving to waypoint " << currentWaypoint << std::endl;
        } else {
            return BTStatus::Running;
        }
    }
    
    std::string waypointKey = "waypoint" + std::to_string(currentWaypoint);
    Vec3 targetPos = blackboard.GetVector(waypointKey);
    Vec3 currentPos = blackboard.GetVector("position");
    
    Vec3 direction = targetPos - currentPos;
    float distance = direction.length();
    
    if (distance <= 0.1f) {
        waiting = true;
        currentWaitTime = 0.0f;
        std::cout << "[BTPatrol] Reached waypoint " << currentWaypoint << ", waiting " << waitTime << "s" << std::endl;
        return BTStatus::Running;
    }
    
    direction = direction.normalize();
    Vec3 newPos = currentPos + direction * speed * deltaTime;
    blackboard.SetVector("position", newPos);
    
    return BTStatus::Running;
}

void BTPatrol::Reset() {
    currentWaitTime = 0.0f;
    currentWaypoint = 0;
    waiting = false;
    BTNode::Reset();
}

BTAttack::BTAttack(float range, float cooldown, float damage, const std::string& name)
    : BTNode(name)
    , attackRange(range)
    , attackCooldown(cooldown)
    , currentCooldown(0.0f)
    , damage(damage) {}

BTStatus BTAttack::Update(float deltaTime, BTBlackboard& blackboard) {
    if (currentCooldown > 0.0f) currentCooldown -= deltaTime;
    
    Vec3 myPos = blackboard.GetVector("position");
    Vec3 targetPos = blackboard.GetVector("targetPosition");
    
    float distance = (targetPos - myPos).length();
    if (distance > attackRange) return BTStatus::Failure;
    
    if (currentCooldown <= 0.0f) {
        currentCooldown = attackCooldown;
        
        float targetHealth = blackboard.GetFloat("targetHealth", 100.0f);
        targetHealth -= damage;
        blackboard.SetFloat("targetHealth", targetHealth);
        
        std::cout << "[BTAttack] Attacked for " << damage << " damage. Target health: " << targetHealth << std::endl;
        
        if (targetHealth <= 0.0f) {
            std::cout << "[BTAttack] Target destroyed!" << std::endl;
            return BTStatus::Success;
        }
    }
    
    return BTStatus::Running;
}

void BTAttack::Reset() {
    currentCooldown = 0.0f;
    BTNode::Reset();
}

BTCheckDistance::BTCheckDistance(float range, bool lessThan, const std::string& name)
    : BTNode(name)
    , checkRange(range)
    , checkLessThan(lessThan) {}

BTStatus BTCheckDistance::Update(float deltaTime, BTBlackboard& blackboard) {
    Vec3 myPos = blackboard.GetVector("position");
    Vec3 targetPos = blackboard.GetVector("targetPosition");
    
    float distance = (targetPos - myPos).length();
    bool inRange = checkLessThan ? (distance <= checkRange) : (distance >= checkRange);
    
    std::cout << "[BTCheckDistance] Distance: " << distance << " (range: " << checkRange << ")" << std::endl;
    
    return inRange ? BTStatus::Success : BTStatus::Failure;
    (void)deltaTime;
}

BTCheckLineOfSight::BTCheckLineOfSight(float distance, float angle, const std::string& name)
    : BTNode(name)
    , viewDistance(distance)
    , viewAngle(angle) {}

BTStatus BTCheckLineOfSight::Update(float deltaTime, BTBlackboard& blackboard) {
    Vec3 myPos = blackboard.GetVector("position");
    Vec3 myForward = blackboard.GetVector("forward");
    Vec3 targetPos = blackboard.GetVector("targetPosition");
    
    Vec3 toTarget = targetPos - myPos;
    float distance = toTarget.length();
    
    if (distance > viewDistance) return BTStatus::Failure;
    
    toTarget.normalize();
    float dot = myForward.dot(toTarget);
    float angle = std::acos(dot) * 180.0f / 3.14159f;
    
    if (angle > viewAngle / 2.0f) return BTStatus::Failure;
    
    std::cout << "[BTCheckLOS] Target visible at distance " << distance << " angle " << angle << std::endl;
    
    return BTStatus::Success;
    (void)deltaTime;
}

BTSetBlackboard::BTSetBlackboard(const std::string& key, std::function<void(BTBlackboard&)> setter, const std::string& name)
    : BTNode(name)
    , key(key)
    , setter(setter) {}

BTStatus BTSetBlackboard::Update(float deltaTime, BTBlackboard& blackboard) {
    if (setter) {
        setter(blackboard);
        std::cout << "[BTSetBlackboard] Set value for key: " << key << std::endl;
        return BTStatus::Success;
    }
    return BTStatus::Error;
    (void)deltaTime;
}

// ============================================
// Blackboard Implementation
// ============================================

BTBlackboard::BTBlackboard() {}
BTBlackboard::~BTBlackboard() {}

void BTBlackboard::SetFloat(const std::string& key, float value) { floatValues[key] = value; }
void BTBlackboard::SetInt(const std::string& key, int value) { intValues[key] = value; }
void BTBlackboard::SetBool(const std::string& key, bool value) { boolValues[key] = value; }
void BTBlackboard::SetVector(const std::string& key, const Vec3& value) { vectorValues[key] = value; }
void BTBlackboard::SetString(const std::string& key, const std::string& value) { stringValues[key] = value; }
void BTBlackboard::SetPointer(const std::string& key, void* value) { pointerValues[key] = value; }

float BTBlackboard::GetFloat(const std::string& key, float defaultValue) const {
    auto it = floatValues.find(key);
    return (it != floatValues.end()) ? it->second : defaultValue;
}

int BTBlackboard::GetInt(const std::string& key, int defaultValue) const {
    auto it = intValues.find(key);
    return (it != intValues.end()) ? it->second : defaultValue;
}

bool BTBlackboard::GetBool(const std::string& key, bool defaultValue) const {
    auto it = boolValues.find(key);
    return (it != boolValues.end()) ? it->second : defaultValue;
}

Vec3 BTBlackboard::GetVector(const std::string& key, const Vec3& defaultValue) const {
    auto it = vectorValues.find(key);
    return (it != vectorValues.end()) ? it->second : defaultValue;
}

std::string BTBlackboard::GetString(const std::string& key, const std::string& defaultValue) const {
    auto it = stringValues.find(key);
    return (it != stringValues.end()) ? it->second : defaultValue;
}

void* BTBlackboard::GetPointer(const std::string& key, void* defaultValue) const {
    auto it = pointerValues.find(key);
    return (it != pointerValues.end()) ? it->second : defaultValue;
}

bool BTBlackboard::HasKey(const std::string& key) const {
    return floatValues.find(key) != floatValues.end() ||
           intValues.find(key) != intValues.end() ||
           boolValues.find(key) != boolValues.end() ||
           vectorValues.find(key) != vectorValues.end() ||
           stringValues.find(key) != stringValues.end() ||
           pointerValues.find(key) != pointerValues.end();
}

void BTBlackboard::RemoveKey(const std::string& key) {
    floatValues.erase(key);
    intValues.erase(key);
    boolValues.erase(key);
    vectorValues.erase(key);
    stringValues.erase(key);
    pointerValues.erase(key);
}

void BTBlackboard::Clear() {
    floatValues.clear();
    intValues.clear();
    boolValues.clear();
    vectorValues.clear();
    stringValues.clear();
    pointerValues.clear();
}

void BTBlackboard::PrintContents() const {
    std::cout << "=== Blackboard Contents ===" << std::endl;
    std::cout << "Floats: " << floatValues.size() << std::endl;
    for (const auto& [key, value] : floatValues) {
        std::cout << "  " << key << " = " << value << std::endl;
    }
    std::cout << "Ints: " << intValues.size() << std::endl;
    for (const auto& [key, value] : intValues) {
        std::cout << "  " << key << " = " << value << std::endl;
    }
    std::cout << "Bools: " << boolValues.size() << std::endl;
    for (const auto& [key, value] : boolValues) {
        std::cout << "  " << key << " = " << (value ? "true" : "false") << std::endl;
    }
    std::cout << "Vectors: " << vectorValues.size() << std::endl;
    for (const auto& [key, value] : vectorValues) {
        std::cout << "  " << key << " = (" << value.x << ", " << value.y << ", " << value.z << ")" << std::endl;
    }
    std::cout << "=========================" << std::endl;
}

// ============================================
// Behavior Tree Implementation
// ============================================

BehaviorTree::BehaviorTree()
    : isRunning(false)
    , updateInterval(0.0f)
    , currentTime(0.0f) {}

BehaviorTree::~BehaviorTree() {}

void BehaviorTree::SetRoot(std::unique_ptr<BTNode> node) {
    root = std::move(node);
}

void BehaviorTree::Start() {
    isRunning = true;
    if (root) root->Reset();
    std::cout << "[BehaviorTree] Started" << std::endl;
}

void BehaviorTree::Stop() {
    isRunning = false;
    std::cout << "[BehaviorTree] Stopped" << std::endl;
}

void BehaviorTree::Update(float deltaTime) {
    if (!isRunning || !root) return;
    
    currentTime += deltaTime;
    if (updateInterval > 0.0f && currentTime < updateInterval) return;
    
    currentTime = 0.0f;
    
    BTStatus status = root->Tick(deltaTime, blackboard);
    
    if (status == BTStatus::Success || status == BTStatus::Failure) {
        std::cout << "[BehaviorTree] Tree finished with status: " 
                  << (status == BTStatus::Success ? "Success" : "Failure") << std::endl;
        root->Reset();
    }
}

void BehaviorTree::Reset() {
    if (root) root->Reset();
    currentTime = 0.0f;
}

void BehaviorTree::PrintTree() const {
    if (root) root->PrintTree();
}

BTStatus BehaviorTree::GetLastStatus() const {
    return BTStatus::Running;
}

// ============================================
// Behavior Tree Builder
// ============================================

BTBuilder::BTBuilder() {}
BTBuilder::~BTBuilder() {}

BTBuilder& BTBuilder::CreateTree(const std::string& name) {
    tree = std::make_unique<BehaviorTree>();
    return *this;
    (void)name;
}

BTBuilder& BTBuilder::Sequence(const std::string& name) {
    auto node = std::make_unique<BTSequence>(name);
    BTNode* ptr = node.get();
    PushNode(ptr);
    
    if (!tree->GetRoot()) {
        tree->SetRoot(std::move(node));
    } else if (GetCurrentParent()) {
        GetCurrentParent()->AddChild(std::move(node));
    }
    
    return *this;
}

BTBuilder& BTBuilder::Selector(const std::string& name) {
    auto node = std::make_unique<BTSelector>(name);
    BTNode* ptr = node.get();
    PushNode(ptr);
    
    if (!tree->GetRoot()) {
        tree->SetRoot(std::move(node));
    } else if (GetCurrentParent()) {
        GetCurrentParent()->AddChild(std::move(node));
    }
    
    return *this;
}

BTBuilder& BTBuilder::Parallel(BTParallel::Policy successPolicy, BTParallel::Policy failurePolicy, const std::string& name) {
    auto node = std::make_unique<BTParallel>(name, successPolicy, failurePolicy);
    BTNode* ptr = node.get();
    PushNode(ptr);
    
    if (!tree->GetRoot()) {
        tree->SetRoot(std::move(node));
    } else if (GetCurrentParent()) {
        GetCurrentParent()->AddChild(std::move(node));
    }
    
    return *this;
}

BTBuilder& BTBuilder::RandomSelector(const std::string& name) {
    auto node = std::make_unique<BTRandomSelector>(name);
    BTNode* ptr = node.get();
    PushNode(ptr);
    
    if (!tree->GetRoot()) {
        tree->SetRoot(std::move(node));
    } else if (GetCurrentParent()) {
        GetCurrentParent()->AddChild(std::move(node));
    }
    
    return *this;
}

BTBuilder& BTBuilder::Inverter(const std::string& name) {
    auto node = std::make_unique<BTInverter>(name);
    BTNode* ptr = node.get();
    PushNode(ptr);
    
    if (GetCurrentParent()) {
        GetCurrentParent()->AddChild(std::move(node));
    }
    
    return *this;
}

BTBuilder& BTBuilder::Repeater(int maxRepeats, const std::string& name) {
    auto node = std::make_unique<BTRepeater>(maxRepeats, name);
    BTNode* ptr = node.get();
    PushNode(ptr);
    
    if (GetCurrentParent()) {
        GetCurrentParent()->AddChild(std::move(node));
    }
    
    return *this;
}

BTBuilder& BTBuilder::Succeeder(const std::string& name) {
    auto node = std::make_unique<BTSucceeder>(name);
    BTNode* ptr = node.get();
    PushNode(ptr);
    
    if (GetCurrentParent()) {
        GetCurrentParent()->AddChild(std::move(node));
    }
    
    return *this;
}

BTBuilder& BTBuilder::Failer(const std::string& name) {
    auto node = std::make_unique<BTFailer>(name);
    BTNode* ptr = node.get();
    PushNode(ptr);
    
    if (GetCurrentParent()) {
        GetCurrentParent()->AddChild(std::move(node));
    }
    
    return *this;
}

BTBuilder& BTBuilder::Limiter(int maxExecutions, const std::string& name) {
    auto node = std::make_unique<BTLimiter>(maxExecutions, name);
    BTNode* ptr = node.get();
    PushNode(ptr);
    
    if (GetCurrentParent()) {
        GetCurrentParent()->AddChild(std::move(node));
    }
    
    return *this;
}

BTBuilder& BTBuilder::Cooldown(float cooldownTime, const std::string& name) {
    auto node = std::make_unique<BTCooldown>(cooldownTime, name);
    BTNode* ptr = node.get();
    PushNode(ptr);
    
    if (GetCurrentParent()) {
        GetCurrentParent()->AddChild(std::move(node));
    }
    
    return *this;
}

BTBuilder& BTBuilder::Action(const std::string& name, std::function<BTStatus(float, BTBlackboard&)> action) {
    auto node = std::make_unique<BTAction>(name, action);
    
    if (GetCurrentParent()) {
        GetCurrentParent()->AddChild(std::move(node));
    }
    
    return *this;
}

BTBuilder& BTBuilder::Wait(float time, const std::string& name) {
    auto node = std::make_unique<BTWait>(time, name);
    
    if (GetCurrentParent()) {
        GetCurrentParent()->AddChild(std::move(node));
    }
    
    return *this;
}

BTBuilder& BTBuilder::MoveTo(float speed, float stoppingDistance, const std::string& name) {
    auto node = std::make_unique<BTMoveTo>(speed, stoppingDistance, name);
    
    if (GetCurrentParent()) {
        GetCurrentParent()->AddChild(std::move(node));
    }
    
    return *this;
}

BTBuilder& BTBuilder::Patrol(float speed, float waitTime, const std::string& name) {
    auto node = std::make_unique<BTPatrol>(speed, waitTime, name);
    
    if (GetCurrentParent()) {
        GetCurrentParent()->AddChild(std::move(node));
    }
    
    return *this;
}

BTBuilder& BTBuilder::Attack(float range, float cooldown, float damage, const std::string& name) {
    auto node = std::make_unique<BTAttack>(range, cooldown, damage, name);
    
    if (GetCurrentParent()) {
        GetCurrentParent()->AddChild(std::move(node));
    }
    
    return *this;
}

BTBuilder& BTBuilder::CheckDistance(float range, bool lessThan, const std::string& name) {
    auto node = std::make_unique<BTCheckDistance>(range, lessThan, name);
    
    if (GetCurrentParent()) {
        GetCurrentParent()->AddChild(std::move(node));
    }
    
    return *this;
}

BTBuilder& BTBuilder::CheckLineOfSight(float distance, float angle, const std::string& name) {
    auto node = std::make_unique<BTCheckLineOfSight>(distance, angle, name);
    
    if (GetCurrentParent()) {
        GetCurrentParent()->AddChild(std::move(node));
    }
    
    return *this;
}

BTBuilder& BTBuilder::End() {
    PopNode();
    return *this;
}

std::unique_ptr<BehaviorTree> BTBuilder::Build() {
    return std::move(tree);
}

void BTBuilder::PushNode(BTNode* node) {
    nodeStack.push_back(node);
}

void BTBuilder::PopNode() {
    if (!nodeStack.empty()) {
        nodeStack.pop_back();
    }
}

BTNode* BTBuilder::GetCurrentParent() {
    if (!nodeStack.empty()) {
        return nodeStack.back();
    }
    return nullptr;
}

} // namespace vge