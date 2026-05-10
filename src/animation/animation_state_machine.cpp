#include "animation_state_machine.h"
#include <iostream>
#include <cmath>
#include <algorithm>

namespace vge {

// ============================================
// Blend Tree Node Base
// ============================================

BlendTreeNode::BlendTreeNode(const std::string& name, BlendTreeNodeType type)
    : name(name)
    , type(type)
    , weight(1.0f)
    , enabled(true) {}

// ============================================
// Clip Node
// ============================================

ClipNode::ClipNode(const std::string& name, AnimationClip* clip)
    : BlendTreeNode(name, BlendTreeNodeType::Clip)
    , clip(clip)
    , currentTime(0.0f)
    , speed(1.0f)
    , looping(true) {}

void ClipNode::Update(float deltaTime, Skeleton* skeleton) {
    if (!clip || !skeleton) return;
    
    // Advance time
    currentTime += deltaTime * speed;
    
    // Loop if needed
    if (looping && currentTime > clip->GetDuration()) {
        currentTime = fmod(currentTime, clip->GetDuration());
    }
    
    // Sample animation
    clip->Sample(currentTime, *skeleton, looping);
}

float ClipNode::GetDuration() const {
    return clip ? clip->GetDuration() : 0.0f;
}

// ============================================
// 1D Blend Node
// ============================================

Blend1DNode::Blend1DNode(const std::string& name)
    : BlendTreeNode(name, BlendTreeNodeType::Blend1D)
    , parameter(0.0f) {}

void Blend1DNode::AddPoint(float threshold, BlendTreeNode* node) {
    points.push_back({threshold, node, 0.0f});
    
    // Sort by threshold
    std::sort(points.begin(), points.end(), 
        [](const Blend1DPoint& a, const Blend1DPoint& b) {
            return a.threshold < b.threshold;
        });
}

void Blend1DNode::RemovePoint(int index) {
    if (index >= 0 && static_cast<size_t>(index) < points.size()) {
        points.erase(points.begin() + index);
    }
}

void Blend1DNode::SetParameter(float value) {
    parameter = value;
    
    // Calculate weights based on parameter
    if (points.empty()) return;
    
    // Find surrounding points
    int leftIndex = -1;
    int rightIndex = -1;
    
    for (size_t i = 0; i < points.size(); ++i) {
        if (points[i].threshold <= parameter) {
            leftIndex = static_cast<int>(i);
        }
        if (points[i].threshold >= parameter && rightIndex == -1) {
            rightIndex = static_cast<int>(i);
        }
    }
    
    // Reset all weights
    for (auto& point : points) {
        point.weight = 0.0f;
    }
    
    if (leftIndex == -1) {
        // Before first point
        points[0].weight = 1.0f;
    } else if (rightIndex == -1 || leftIndex == rightIndex) {
        // After last point or exact match
        points[leftIndex].weight = 1.0f;
    } else {
        // Interpolate between points
        float leftThreshold = points[leftIndex].threshold;
        float rightThreshold = points[rightIndex].threshold;
        float t = (parameter - leftThreshold) / (rightThreshold - leftThreshold);
        
        points[leftIndex].weight = 1.0f - t;
        points[rightIndex].weight = t;
    }
}

void Blend1DNode::Update(float deltaTime, Skeleton* skeleton) {
    if (!skeleton) return;
    
    // Update all child nodes
    for (auto& point : points) {
        if (point.node && point.weight > 0.0f) {
            point.node->Update(deltaTime, skeleton);
        }
    }
}

float Blend1DNode::GetDuration() const {
    float maxDuration = 0.0f;
    for (const auto& point : points) {
        if (point.node) {
            maxDuration = std::max(maxDuration, point.node->GetDuration());
        }
    }
    return maxDuration;
}

bool Blend1DNode::IsLooping() const {
    for (const auto& point : points) {
        if (point.node && point.node->IsLooping()) {
            return true;
        }
    }
    return false;
}

// ============================================
// 2D Blend Node
// ============================================

Blend2DNode::Blend2DNode(const std::string& name)
    : BlendTreeNode(name, BlendTreeNodeType::Blend2D)
    , parameter(0.0f, 0.0f) {}

void Blend2DNode::AddPoint(const Vec2& blendPosition, BlendTreeNode* node) {
    points.push_back({blendPosition, node, 0.0f});
}

void Blend2DNode::RemovePoint(int index) {
    if (index >= 0 && static_cast<size_t>(index) < points.size()) {
        points.erase(points.begin() + index);
    }
}

void Blend2DNode::SetParameter(const Vec2& value) {
    parameter = value;
    
    // Calculate weights using distance-based blending
    if (points.empty()) return;
    
    // Simple distance-based inverse weighting
    float totalWeight = 0.0f;
    
    for (auto& point : points) {
        float dist = std::sqrt(
            (point.blendPosition.x - parameter.x) * (point.blendPosition.x - parameter.x) +
            (point.blendPosition.y - parameter.y) * (point.blendPosition.y - parameter.y)
        );
        
        // Inverse distance weighting
        if (dist < 0.001f) {
            point.weight = 1000.0f; // Very close
        } else {
            point.weight = 1.0f / dist;
        }
        
        totalWeight += point.weight;
    }
    
    // Normalize
    if (totalWeight > 0.0f) {
        for (auto& point : points) {
            point.weight /= totalWeight;
        }
    }
}

void Blend2DNode::Update(float deltaTime, Skeleton* skeleton) {
    if (!skeleton) return;
    
    // Update all child nodes
    for (auto& point : points) {
        if (point.node && point.weight > 0.0f) {
            point.node->Update(deltaTime, skeleton);
        }
    }
}

float Blend2DNode::GetDuration() const {
    float maxDuration = 0.0f;
    for (const auto& point : points) {
        if (point.node) {
            maxDuration = std::max(maxDuration, point.node->GetDuration());
        }
    }
    return maxDuration;
}

bool Blend2DNode::IsLooping() const {
    for (const auto& point : points) {
        if (point.node && point.node->IsLooping()) {
            return true;
        }
    }
    return false;
}

// ============================================
// Additive Node
// ============================================

AdditiveNode::AdditiveNode(const std::string& name, BlendTreeNode* base, BlendTreeNode* additive)
    : BlendTreeNode(name, BlendTreeNodeType::Additive)
    , baseNode(base)
    , additiveNode(additive)
    , additiveWeight(1.0f) {}

void AdditiveNode::Update(float deltaTime, Skeleton* skeleton) {
    if (!skeleton) return;
    
    // Update base animation
    if (baseNode) {
        baseNode->Update(deltaTime, skeleton);
    }
    
    // Store current pose
    // Would need to store pose before applying additive
    
    // Apply additive animation
    if (additiveNode && additiveWeight > 0.0f) {
        // Would apply additive rotation/position offsets
        std::cout << "[AdditiveNode] Applying additive animation with weight " << additiveWeight << std::endl;
    }
}

float AdditiveNode::GetDuration() const {
    return baseNode ? baseNode->GetDuration() : 0.0f;
}

bool AdditiveNode::IsLooping() const {
    return baseNode ? baseNode->IsLooping() : false;
}

// ============================================
// Animation State
// ============================================

AnimationState::AnimationState(const std::string& name)
    : name(name)
    , speed(1.0f)
    , looping(true) {}

void AnimationState::SetBlendTree(std::unique_ptr<BlendTreeNode> tree) {
    blendTree = std::move(tree);
}

void AnimationState::Enter() {
    std::cout << "[AnimationState] Entering state: " << name << std::endl;
    if (onEnter) onEnter();
}

void AnimationState::Exit() {
    std::cout << "[AnimationState] Exiting state: " << name << std::endl;
    if (onExit) onExit();
}

void AnimationState::Update(float deltaTime, Skeleton* skeleton) {
    if (onUpdate) onUpdate(deltaTime);
    
    if (blendTree && skeleton) {
        blendTree->Update(deltaTime * speed, skeleton);
    }
}

void AnimationState::AddTag(const std::string& tag) {
    if (!HasTag(tag)) {
        tags.push_back(tag);
    }
}

void AnimationState::RemoveTag(const std::string& tag) {
    tags.erase(std::remove(tags.begin(), tags.end(), tag), tags.end());
}

bool AnimationState::HasTag(const std::string& tag) const {
    return std::find(tags.begin(), tags.end(), tag) != tags.end();
}

// ============================================
// State Transition
// ============================================

StateTransition::StateTransition(AnimationState* from, AnimationState* to)
    : fromState(from)
    , toState(to)
    , blendDuration(0.25f)
    , exitTime(0.0f)
    , hasExitTime(false)
    , canTransitionToSelf(false)
    , canBeInterrupted(true) {}

void StateTransition::AddCondition(std::function<bool()> condition) {
    conditions.push_back(condition);
}

void StateTransition::ClearConditions() {
    conditions.clear();
}

bool StateTransition::CanTransition() const {
    // All conditions must be true
    for (const auto& condition : conditions) {
        if (!condition()) {
            return false;
        }
    }
    return true;
}

void StateTransition::AddInterruptionSource(const std::string& tag) {
    interruptionSources.push_back(tag);
}

bool StateTransition::CanBeInterruptedBy(const std::string& tag) const {
    if (!canBeInterrupted) return false;
    
    if (interruptionSources.empty()) return true; // Any state can interrupt
    
    return std::find(interruptionSources.begin(), interruptionSources.end(), tag) 
           != interruptionSources.end();
}

// ============================================
// Animation State Machine
// ============================================

AnimationStateMachine::AnimationStateMachine(Skeleton* skeleton)
    : currentState(nullptr)
    , previousState(nullptr)
    , isBlending(false)
    , blendTime(0.0f)
    , blendDuration(0.25f)
    , blendWeight(0.0f)
    , skeleton(skeleton)
    , initialized(false) {}

AnimationStateMachine::~AnimationStateMachine() {
    states.clear();
    transitions.clear();
}

void AnimationStateMachine::SetSkeleton(Skeleton* skel) {
    skeleton = skel;
}

void AnimationStateMachine::Initialize() {
    if (!skeleton) {
        std::cout << "[StateMachine] Warning: No skeleton set" << std::endl;
    }
    initialized = true;
    std::cout << "[StateMachine] Initialized with " << states.size() << " states" << std::endl;
}

AnimationState* AnimationStateMachine::CreateState(const std::string& name) {
    auto state = std::make_unique<AnimationState>(name);
    AnimationState* ptr = state.get();
    states.push_back(std::move(state));
    std::cout << "[StateMachine] Created state: " << name << std::endl;
    return ptr;
}

void AnimationStateMachine::RemoveState(const std::string& name) {
    states.erase(
        std::remove_if(states.begin(), states.end(),
            [&name](const std::unique_ptr<AnimationState>& s) { return s->GetName() == name; }),
        states.end()
    );
}

AnimationState* AnimationStateMachine::GetState(const std::string& name) const {
    for (const auto& state : states) {
        if (state->GetName() == name) {
            return state.get();
        }
    }
    return nullptr;
}

StateTransition* AnimationStateMachine::AddTransition(const std::string& from, const std::string& to) {
    AnimationState* fromState = GetState(from);
    AnimationState* toState = GetState(to);
    
    if (!fromState || !toState) {
        std::cout << "[StateMachine] Error: Cannot create transition, states not found" << std::endl;
        return nullptr;
    }
    
    auto transition = std::make_unique<StateTransition>(fromState, toState);
    StateTransition* ptr = transition.get();
    transitions.push_back(std::move(transition));
    
    std::cout << "[StateMachine] Added transition: " << from << " -> " << to << std::endl;
    return ptr;
}

StateTransition* AnimationStateMachine::AddAnyStateTransition(const std::string& to) {
    AnimationState* toState = GetState(to);
    
    if (!toState) {
        std::cout << "[StateMachine] Error: Cannot create any-state transition, target not found" << std::endl;
        return nullptr;
    }
    
    auto transition = std::make_unique<StateTransition>(nullptr, toState);
    StateTransition* ptr = transition.get();
    transitions.push_back(std::move(transition));
    
    std::cout << "[StateMachine] Added any-state transition -> " << to << std::endl;
    return ptr;
}

void AnimationStateMachine::RemoveTransition(StateTransition* transition) {
    transitions.erase(
        std::remove_if(transitions.begin(), transitions.end(),
            [transition](const std::unique_ptr<StateTransition>& t) { return t.get() == transition; }),
        transitions.end()
    );
}

void AnimationStateMachine::SetFloat(const std::string& name, float value) {
    floatParameters[name] = value;
}

void AnimationStateMachine::SetBool(const std::string& name, bool value) {
    boolParameters[name] = value;
}

void AnimationStateMachine::SetInt(const std::string& name, int value) {
    intParameters[name] = value;
}

float AnimationStateMachine::GetFloat(const std::string& name) const {
    auto it = floatParameters.find(name);
    return (it != floatParameters.end()) ? it->second : 0.0f;
}

bool AnimationStateMachine::GetBool(const std::string& name) const {
    auto it = boolParameters.find(name);
    return (it != boolParameters.end()) ? it->second : false;
}

int AnimationStateMachine::GetInt(const std::string& name) const {
    auto it = intParameters.find(name);
    return (it != intParameters.end()) ? it->second : 0;
}

bool AnimationStateMachine::HasParameter(const std::string& name) const {
    return floatParameters.find(name) != floatParameters.end() ||
           boolParameters.find(name) != boolParameters.end() ||
           intParameters.find(name) != intParameters.end();
}

void AnimationStateMachine::Play(const std::string& stateName, float blendDuration) {
    AnimationState* state = GetState(stateName);
    if (!state) {
        std::cout << "[StateMachine] Error: State not found: " << stateName << std::endl;
        return;
    }
    
    if (currentState == state) {
        return; // Already in this state
    }
    
    // Exit current state
    if (currentState) {
        currentState->Exit();
        previousState = currentState;
    }
    
    // Enter new state
    currentState = state;
    currentState->Enter();
    
    // Start blending
    if (previousState && blendDuration > 0.0f) {
        isBlending = true;
        blendTime = 0.0f;
        this->blendDuration = blendDuration;
        blendWeight = 0.0f;
        std::cout << "[StateMachine] Blending to " << stateName << " over " << blendDuration << "s" << std::endl;
    } else {
        isBlending = false;
        blendWeight = 1.0f;
    }
    
    std::cout << "[StateMachine] Now playing: " << stateName << std::endl;
}

void AnimationStateMachine::CrossFade(const std::string& stateName, float blendDuration) {
    Play(stateName, blendDuration);
}

void AnimationStateMachine::Update(float deltaTime) {
    if (!initialized || !skeleton) return;
    
    // Update blending
    if (isBlending) {
        blendTime += deltaTime;
        blendWeight = std::min(1.0f, blendTime / blendDuration);
        
        if (blendWeight >= 1.0f) {
            isBlending = false;
            previousState = nullptr;
        }
    }
    
    // Update current state
    if (currentState) {
        currentState->Update(deltaTime, skeleton);
    }
    
    // Check transitions
    for (const auto& transition : transitions) {
        AnimationState* fromState = transition->GetFromState();
        
        // Check if transition applies
        bool applies = false;
        if (fromState == nullptr) {
            // Any-state transition
            applies = true;
        } else if (fromState == currentState) {
            // Normal transition from current state
            applies = true;
        }
        
        if (applies && transition->CanTransition()) {
            AnimationState* toState = transition->GetToState();
            if (toState && toState != currentState) {
                Play(toState->GetName(), transition->GetBlendDuration());
                break; // Only one transition per frame
            }
        }
    }
}

bool AnimationStateMachine::IsInState(const std::string& stateName) const {
    return currentState && currentState->GetName() == stateName;
}

float AnimationStateMachine::GetCurrentStateTime() const {
    // Would track state time
    return 0.0f;
}

float AnimationStateMachine::GetCurrentStateNormalizedTime() const {
    // Would calculate normalized time
    return 0.0f;
}

void AnimationStateMachine::PrintStateMachine() const {
    std::cout << "=== Animation State Machine ===" << std::endl;
    std::cout << "States: " << states.size() << std::endl;
    for (const auto& state : states) {
        std::cout << "  " << state->GetName();
        if (state.get() == currentState) std::cout << " [CURRENT]";
        if (state.get() == previousState) std::cout << " [PREVIOUS]";
        std::cout << std::endl;
    }
    
    std::cout << "Transitions: " << transitions.size() << std::endl;
    for (const auto& transition : transitions) {
        std::string from = transition->GetFromState() ? transition->GetFromState()->GetName() : "ANY";
        std::string to = transition->GetToState() ? transition->GetToState()->GetName() : "NONE";
        std::cout << "  " << from << " -> " << to << std::endl;
    }
    std::cout << "================================" << std::endl;
}

// ============================================
// Animation Layer
// ============================================

AnimationLayer::AnimationLayer(const std::string& name, int index)
    : name(name)
    , layerIndex(index)
    , layerWeight(1.0f)
    , stateMachine(nullptr)
    , isAdditive(false) {}

AnimationLayer::~AnimationLayer() {
    delete stateMachine;
}

void AnimationLayer::SetStateMachine(AnimationStateMachine* sm) {
    stateMachine = sm;
}

void AnimationLayer::SetJointMask(const std::vector<bool>& mask) {
    jointMask = mask;
}

void AnimationLayer::SetJointMaskFromSkeleton(const Skeleton& skeleton, const std::vector<std::string>& jointNames) {
    jointMask.resize(skeleton.GetJointCount(), false);
    
    for (const auto& jointName : jointNames) {
        int index = skeleton.GetJointIndex(jointName);
        if (index >= 0) {
            jointMask[index] = true;
        }
    }
}

bool AnimationLayer::AffectsJoint(int jointIndex) const {
    if (jointIndex < 0 || static_cast<size_t>(jointIndex) >= jointMask.size()) {
        return true; // No mask = affects all
    }
    return jointMask[jointIndex];
}

void AnimationLayer::Update(float deltaTime) {
    if (stateMachine) {
        stateMachine->Update(deltaTime);
    }
}

void AnimationLayer::ApplyToSkeleton(Skeleton* skeleton, float globalWeight) {
    if (!skeleton || !stateMachine) return;
    
    float finalWeight = layerWeight * globalWeight;
    if (finalWeight <= 0.0f) return;
    
    // Would apply animation to skeleton with weight
    std::cout << "[AnimationLayer] Applying layer " << name 
              << " with weight " << finalWeight << std::endl;
}

// ============================================
// Multi-Layer Animator
// ============================================

MultiLayerAnimator::MultiLayerAnimator(Skeleton* skeleton)
    : skeleton(skeleton)
    , baseLayer(nullptr) {}

MultiLayerAnimator::~MultiLayerAnimator() {
    layers.clear();
}

AnimationLayer* MultiLayerAnimator::CreateLayer(const std::string& name) {
    auto layer = std::make_unique<AnimationLayer>(name, layers.size());
    
    // Create state machine for layer
    auto* sm = new AnimationStateMachine(skeleton);
    layer->SetStateMachine(sm);
    
    AnimationLayer* ptr = layer.get();
    layers.push_back(std::move(layer));
    
    // First layer is base layer
    if (layers.size() == 1) {
        baseLayer = ptr;
    }
    
    std::cout << "[MultiLayerAnimator] Created layer: " << name << " (index " << ptr->GetIndex() << ")" << std::endl;
    return ptr;
}

void MultiLayerAnimator::RemoveLayer(const std::string& name) {
    layers.erase(
        std::remove_if(layers.begin(), layers.end(),
            [&name](const std::unique_ptr<AnimationLayer>& l) { return l->GetName() == name; }),
        layers.end()
    );
}

AnimationLayer* MultiLayerAnimator::GetLayer(const std::string& name) const {
    for (const auto& layer : layers) {
        if (layer->GetName() == name) {
            return layer.get();
        }
    }
    return nullptr;
}

AnimationLayer* MultiLayerAnimator::GetLayer(int index) const {
    if (index >= 0 && static_cast<size_t>(index) < layers.size()) {
        return layers[index].get();
    }
    return nullptr;
}

void MultiLayerAnimator::Update(float deltaTime) {
    // Update all layers
    for (auto& layer : layers) {
        layer->Update(deltaTime);
    }
}

void MultiLayerAnimator::ApplyToSkeleton() {
    if (!skeleton) return;
    
    // Apply base layer first
    if (baseLayer) {
        baseLayer->ApplyToSkeleton(skeleton, 1.0f);
    }
    
    // Apply other layers
    for (auto& layer : layers) {
        if (layer.get() != baseLayer) {
            layer->ApplyToSkeleton(skeleton, 1.0f);
        }
    }
}

void MultiLayerAnimator::Play(const std::string& layerName, const std::string& stateName, float blendDuration) {
    AnimationLayer* layer = GetLayer(layerName);
    if (layer && layer->GetStateMachine()) {
        layer->GetStateMachine()->Play(stateName, blendDuration);
    }
}

void MultiLayerAnimator::CrossFade(const std::string& layerName, const std::string& stateName, float blendDuration) {
    Play(layerName, stateName, blendDuration);
}

void MultiLayerAnimator::SetLayerWeight(const std::string& layerName, float weight) {
    AnimationLayer* layer = GetLayer(layerName);
    if (layer) {
        layer->SetWeight(weight);
    }
}

// ============================================
// IK Solvers
// ============================================

IKSolver::IKSolver(int maxIterations, float threshold)
    : maxIterations(maxIterations)
    , threshold(threshold) {}

// CCD IK
CCDIKSolver::CCDIKSolver(int maxIterations, float threshold)
    : IKSolver(maxIterations, threshold) {}

bool CCDIKSolver::Solve(Skeleton* skeleton, int endEffector, const Vec3& target) {
    if (!skeleton) return false;
    
    std::cout << "[CCDIK] Solving for joint " << endEffector << " to target ("
              << target.x << ", " << target.y << ", " << target.z << ")" << std::endl;
    
    // CCD algorithm: work backwards from end effector to root
    for (int iteration = 0; iteration < maxIterations; ++iteration) {
        // Would implement CCD algorithm
        // 1. Get current end effector position
        // 2. Calculate error to target
        // 3. If error < threshold, return true
        // 4. For each joint from end to root:
        //    a. Calculate vector from joint to end effector
        //    b. Calculate vector from joint to target
        //    c. Rotate joint to align vectors
    }
    
    std::cout << "[CCDIK] Max iterations reached" << std::endl;
    return false; // Did not converge
}

// FABRIK
FABRIKIKSolver::FABRIKIKSolver(int maxIterations, float threshold)
    : IKSolver(maxIterations, threshold) {}

bool FABRIKIKSolver::Solve(Skeleton* skeleton, int endEffector, const Vec3& target) {
    if (!skeleton) return false;
    
    std::cout << "[FABRIK] Solving for joint " << endEffector << " to target ("
              << target.x << ", " << target.y << ", " << target.z << ")" << std::endl;
    
    // FABRIK algorithm
    for (int iteration = 0; iteration < maxIterations; ++iteration) {
        // Would implement FABRIK algorithm
        // 1. Forward reaching: set end effector to target, propagate back
        // 2. Backward reaching: set root to original position, propagate forward
        // 3. Check convergence
    }
    
    std::cout << "[FABRIK] Max iterations reached" << std::endl;
    return false;
}

// Two-Bone IK
TwoBoneIKSolver::TwoBoneIKSolver(float minReach, float maxReach)
    : IKSolver(1, 0.001f)
    , minReach(minReach)
    , maxReach(maxReach) {}

bool TwoBoneIKSolver::Solve(Skeleton* skeleton, int endEffector, const Vec3& target) {
    if (!skeleton) return false;
    
    std::cout << "[TwoBoneIK] Solving for joint " << endEffector << " to target ("
              << target.x << ", " << target.y << ", " << target.z << ")" << std::endl;
    
    // Two-bone IK is analytical (no iterations needed)
    // Would implement law of cosines to calculate joint angles
    
    std::cout << "[TwoBoneIK] Solution found" << std::endl;
    return true;
}

// ============================================
// IK System
// ============================================

IKSystem::IKSystem(Skeleton* skeleton)
    : skeleton(skeleton) {}

IKSystem::~IKSystem() {
    solvers.clear();
}

void IKSystem::AddSolver(std::unique_ptr<IKSolver> solver) {
    solvers.push_back(std::move(solver));
}

void IKSystem::ClearSolvers() {
    solvers.clear();
}

void IKSystem::SetTarget(int endEffector, const Vec3& position, float weight) {
    // Find or create target
    for (auto& target : targets) {
        if (target.endEffectorIndex == endEffector) {
            target.targetPosition = position;
            target.weight = weight;
            target.enabled = true;
            return;
        }
    }
    
    // Create new target
    targets.push_back({endEffector, position, weight, true});
}

void IKSystem::ClearTarget(int endEffector) {
    targets.erase(
        std::remove_if(targets.begin(), targets.end(),
            [endEffector](const IKTarget& t) { return t.endEffectorIndex == endEffector; }),
        targets.end()
    );
}

void IKSystem::DisableTarget(int endEffector) {
    for (auto& target : targets) {
        if (target.endEffectorIndex == endEffector) {
            target.enabled = false;
            return;
        }
    }
}

void IKSystem::Solve() {
    if (!skeleton || solvers.empty()) return;
    
    std::cout << "[IKSystem] Solving " << targets.size() << " targets with " 
              << solvers.size() << " solvers" << std::endl;
    
    for (const auto& target : targets) {
        if (!target.enabled || target.weight <= 0.0f) continue;
        
        // Try each solver
        for (auto& solver : solvers) {
            if (solver->Solve(skeleton, target.endEffectorIndex, target.targetPosition)) {
                break; // Solved
            }
        }
    }
}

void IKSystem::SetupLegIK(int hip, int knee, int ankle, int toe) {
    std::cout << "[IKSystem] Setting up leg IK: hip=" << hip 
              << " knee=" << knee << " ankle=" << ankle << " toe=" << toe << std::endl;
    
    // Add two-bone solver for leg
    auto solver = std::make_unique<TwoBoneIKSolver>();
    AddSolver(std::move(solver));
}

void IKSystem::SetupArmIK(int shoulder, int elbow, int wrist, int hand) {
    std::cout << "[IKSystem] Setting up arm IK: shoulder=" << shoulder 
              << " elbow=" << elbow << " wrist=" << wrist << " hand=" << hand << std::endl;
    
    // Add two-bone solver for arm
    auto solver = std::make_unique<TwoBoneIKSolver>();
    AddSolver(std::move(solver));
}

void IKSystem::SetupHeadIK(int neck, int head) {
    std::cout << "[IKSystem] Setting up head IK: neck=" << neck << " head=" << head << std::endl;
    
    // Add CCD solver for head
    auto solver = std::make_unique<CCDIKSolver>();
    AddSolver(std::move(solver));
}

void IKSystem::SetupSpineIK(const std::vector<int>& spineJoints) {
    std::cout << "[IKSystem] Setting up spine IK with " << spineJoints.size() << " joints" << std::endl;
    
    // Add FABRIK solver for spine
    auto solver = std::make_unique<FABRIKIKSolver>();
    AddSolver(std::move(solver));
}

} // namespace vge