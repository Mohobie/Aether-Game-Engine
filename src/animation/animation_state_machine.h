#pragma once
#include "math/vec3.h"
#include "math/vec2.h"
#include "animation.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

namespace vge {

// ============================================
// Animation State Machine
// ============================================

// Forward declarations
class AnimationState;
class StateTransition;
class AnimationStateMachine;

// ============================================
// Blend Tree Node Types
// ============================================
enum class BlendTreeNodeType {
    Clip,           // Single animation clip
    Blend1D,        // 1D blend (e.g., speed)
    Blend2D,        // 2D blend (e.g., direction)
    Additive,       // Additive animation (e.g., recoil)
    Layered         // Layered animation (upper body)
};

// ============================================
// Blend Tree Node
// ============================================
class BlendTreeNode {
protected:
    std::string name;
    BlendTreeNodeType type;
    float weight;
    bool enabled;
    
public:
    BlendTreeNode(const std::string& name, BlendTreeNodeType type);
    virtual ~BlendTreeNode() = default;
    
    virtual void Update(float deltaTime, Skeleton* skeleton) = 0;
    virtual float GetDuration() const = 0;
    virtual bool IsLooping() const = 0;
    
    void SetWeight(float w) { weight = w; }
    float GetWeight() const { return weight; }
    void SetEnabled(bool e) { enabled = e; }
    bool IsEnabled() const { return enabled; }
    std::string GetName() const { return name; }
    BlendTreeNodeType GetType() const { return type; }
};

// ============================================
// Clip Node (Single Animation)
// ============================================
class ClipNode : public BlendTreeNode {
private:
    AnimationClip* clip;
    float currentTime;
    float speed;
    bool looping;
    
public:
    ClipNode(const std::string& name, AnimationClip* clip);
    
    void Update(float deltaTime, Skeleton* skeleton) override;
    float GetDuration() const override;
    bool IsLooping() const override { return looping; }
    
    void SetSpeed(float s) { speed = s; }
    float GetSpeed() const { return speed; }
    void SetLooping(bool loop) { looping = loop; }
    void SetTime(float time) { currentTime = time; }
    float GetTime() const { return currentTime; }
    AnimationClip* GetClip() const { return clip; }
};

// ============================================
// 1D Blend Node (e.g., Idle -> Walk -> Run)
// ============================================
struct Blend1DPoint {
    float threshold;
    BlendTreeNode* node;
    float weight;
};

class Blend1DNode : public BlendTreeNode {
private:
    std::vector<Blend1DPoint> points;
    float parameter;
    
public:
    Blend1DNode(const std::string& name);
    
    void AddPoint(float threshold, BlendTreeNode* node);
    void RemovePoint(int index);
    void SetParameter(float value);
    float GetParameter() const { return parameter; }
    
    void Update(float deltaTime, Skeleton* skeleton) override;
    float GetDuration() const override;
    bool IsLooping() const override;
    
    int GetPointCount() const { return static_cast<int>(points.size()); }
};

// ============================================
// 2D Blend Node (e.g., Directional Movement)
// ============================================
struct Blend2DPoint {
    Vec2 blendPosition;
    BlendTreeNode* node;
    float weight;
};

class Blend2DNode : public BlendTreeNode {
private:
    std::vector<Blend2DPoint> points;
    Vec2 parameter;
    
public:
    Blend2DNode(const std::string& name);
    
    void AddPoint(const Vec2& blendPosition, BlendTreeNode* node);
    void RemovePoint(int index);
    void SetParameter(const Vec2& value);
    Vec2 GetParameter() const { return parameter; }
    
    void Update(float deltaTime, Skeleton* skeleton) override;
    float GetDuration() const override;
    bool IsLooping() const override;
    
    int GetPointCount() const { return static_cast<int>(points.size()); }
};

// ============================================
// Additive Node (e.g., Recoil, Hit Reactions)
// ============================================
class AdditiveNode : public BlendTreeNode {
private:
    BlendTreeNode* baseNode;
    BlendTreeNode* additiveNode;
    float additiveWeight;
    
public:
    AdditiveNode(const std::string& name, BlendTreeNode* base, BlendTreeNode* additive);
    
    void Update(float deltaTime, Skeleton* skeleton) override;
    float GetDuration() const override;
    bool IsLooping() const override;
    
    void SetAdditiveWeight(float w) { additiveWeight = w; }
    float GetAdditiveWeight() const { return additiveWeight; }
};

// ============================================
// Animation State
// ============================================
class AnimationState {
private:
    std::string name;
    std::unique_ptr<BlendTreeNode> blendTree;
    float speed;
    bool looping;
    
    // State behavior
    std::function<void()> onEnter;
    std::function<void()> onExit;
    std::function<void(float)> onUpdate;
    
    // Tags for state categorization
    std::vector<std::string> tags;
    
public:
    AnimationState(const std::string& name);
    ~AnimationState() = default;
    
    // Blend tree
    void SetBlendTree(std::unique_ptr<BlendTreeNode> tree);
    BlendTreeNode* GetBlendTree() const { return blendTree.get(); }
    
    // Speed
    void SetSpeed(float s) { speed = s; }
    float GetSpeed() const { return speed; }
    
    // Looping
    void SetLooping(bool loop) { looping = loop; }
    bool IsLooping() const { return looping; }
    
    // Callbacks
    void SetOnEnter(std::function<void()> callback) { onEnter = callback; }
    void SetOnExit(std::function<void()> callback) { onExit = callback; }
    void SetOnUpdate(std::function<void(float)> callback) { onUpdate = callback; }
    
    void Enter();
    void Exit();
    void Update(float deltaTime, Skeleton* skeleton);
    
    // Tags
    void AddTag(const std::string& tag);
    void RemoveTag(const std::string& tag);
    bool HasTag(const std::string& tag) const;
    const std::vector<std::string>& GetTags() const { return tags; }
    
    std::string GetName() const { return name; }
};

// ============================================
// State Transition
// ============================================
class StateTransition {
private:
    AnimationState* fromState;
    AnimationState* toState;
    
    // Transition conditions
    std::vector<std::function<bool()>> conditions;
    
    // Transition properties
    float blendDuration;
    float exitTime;        // Normalized time (0-1) when transition can start
    bool hasExitTime;
    bool canTransitionToSelf;
    
    // Interruption
    bool canBeInterrupted;
    std::vector<std::string> interruptionSources;
    
public:
    StateTransition(AnimationState* from, AnimationState* to);
    
    // Conditions
    void AddCondition(std::function<bool()> condition);
    void ClearConditions();
    bool CanTransition() const;
    
    // Properties
    void SetBlendDuration(float duration) { blendDuration = duration; }
    float GetBlendDuration() const { return blendDuration; }
    void SetExitTime(float time) { exitTime = time; hasExitTime = true; }
    float GetExitTime() const { return exitTime; }
    bool HasExitTime() const { return hasExitTime; }
    void SetCanTransitionToSelf(bool can) { canTransitionToSelf = can; }
    bool CanTransitionToSelf() const { return canTransitionToSelf; }
    
    // Interruption
    void SetCanBeInterrupted(bool can) { canBeInterrupted = can; }
    bool CanBeInterrupted() const { return canBeInterrupted; }
    void AddInterruptionSource(const std::string& tag);
    bool CanBeInterruptedBy(const std::string& tag) const;
    
    AnimationState* GetFromState() const { return fromState; }
    AnimationState* GetToState() const { return toState; }
};

// ============================================
// Animation State Machine
// ============================================
class AnimationStateMachine {
private:
    std::vector<std::unique_ptr<AnimationState>> states;
    std::vector<std::unique_ptr<StateTransition>> transitions;
    
    AnimationState* currentState;
    AnimationState* previousState;
    
    // Cross-fade blending
    bool isBlending;
    float blendTime;
    float blendDuration;
    float blendWeight;  // 0 = previous, 1 = current
    
    // Global parameters (for blend trees and conditions)
    std::unordered_map<std::string, float> floatParameters;
    std::unordered_map<std::string, bool> boolParameters;
    std::unordered_map<std::string, int> intParameters;
    
    Skeleton* skeleton;
    bool initialized;
    
public:
    AnimationStateMachine(Skeleton* skeleton = nullptr);
    ~AnimationStateMachine();
    
    // Initialization
    void SetSkeleton(Skeleton* skel);
    void Initialize();
    
    // State management
    AnimationState* CreateState(const std::string& name);
    void RemoveState(const std::string& name);
    AnimationState* GetState(const std::string& name) const;
    AnimationState* GetCurrentState() const { return currentState; }
    AnimationState* GetPreviousState() const { return previousState; }
    int GetStateCount() const { return static_cast<int>(states.size()); }
    
    // Transitions
    StateTransition* AddTransition(const std::string& from, const std::string& to);
    StateTransition* AddAnyStateTransition(const std::string& to);
    void RemoveTransition(StateTransition* transition);
    
    // Parameters
    void SetFloat(const std::string& name, float value);
    void SetBool(const std::string& name, bool value);
    void SetInt(const std::string& name, int value);
    float GetFloat(const std::string& name) const;
    bool GetBool(const std::string& name) const;
    int GetInt(const std::string& name) const;
    bool HasParameter(const std::string& name) const;
    
    // Playback
    void Play(const std::string& stateName, float blendDurationSeconds = 0.25f);
    void CrossFade(const std::string& stateName, float blendDurationSeconds);
    void Update(float deltaTime);
    
    // State queries
    bool IsInState(const std::string& stateName) const;
    float GetCurrentStateTime() const;
    float GetCurrentStateNormalizedTime() const;
    
    // Any state transitions
    void SetAnyStateTransition(const std::string& toState, float blendDurationSeconds = 0.1f);
    
    // Debug
    void PrintStateMachine() const;
};

// ============================================
// Animation Layer (for upper/lower body separation)
// ============================================
class AnimationLayer {
private:
    std::string name;
    int layerIndex;
    float layerWeight;
    
    AnimationStateMachine* stateMachine;
    
    // Avatar mask (which joints this layer affects)
    std::vector<bool> jointMask;  // true = affected by this layer
    
    // Additive blending
    bool isAdditive;
    
public:
    AnimationLayer(const std::string& name, int index);
    ~AnimationLayer();
    
    void SetStateMachine(AnimationStateMachine* sm);
    AnimationStateMachine* GetStateMachine() const { return stateMachine; }
    
    // Joint mask
    void SetJointMask(const std::vector<bool>& mask);
    void SetJointMaskFromSkeleton(const Skeleton& skeleton, const std::vector<std::string>& jointNames);
    bool AffectsJoint(int jointIndex) const;
    
    // Weight
    void SetWeight(float w) { layerWeight = w; }
    float GetWeight() const { return layerWeight; }
    
    // Additive
    void SetAdditive(bool additive) { isAdditive = additive; }
    bool IsAdditive() const { return isAdditive; }
    
    void Update(float deltaTime);
    void ApplyToSkeleton(Skeleton* skeleton, float globalWeight = 1.0f);
    
    std::string GetName() const { return name; }
    int GetIndex() const { return layerIndex; }
};

// ============================================
// Multi-Layer Animator
// ============================================
class MultiLayerAnimator {
private:
    Skeleton* skeleton;
    std::vector<std::unique_ptr<AnimationLayer>> layers;
    
    // Base layer (layer 0)
    AnimationLayer* baseLayer;
    
public:
    MultiLayerAnimator(Skeleton* skeleton);
    ~MultiLayerAnimator();
    
    // Layer management
    AnimationLayer* CreateLayer(const std::string& name);
    void RemoveLayer(const std::string& name);
    AnimationLayer* GetLayer(const std::string& name) const;
    AnimationLayer* GetLayer(int index) const;
    int GetLayerCount() const { return static_cast<int>(layers.size()); }
    
    // Base layer
    AnimationLayer* GetBaseLayer() const { return baseLayer; }
    
    // Update all layers
    void Update(float deltaTime);
    
    // Apply all layers to skeleton
    void ApplyToSkeleton();
    
    // Convenience methods
    void Play(const std::string& layerName, const std::string& stateName, float blendDuration = 0.25f);
    void CrossFade(const std::string& layerName, const std::string& stateName, float blendDuration);
    void SetLayerWeight(const std::string& layerName, float weight);
};

// ============================================
// Inverse Kinematics (IK)
// ============================================
class IKSolver {
protected:
    int maxIterations;
    float threshold;
    
public:
    IKSolver(int maxIterations = 10, float threshold = 0.01f);
    virtual ~IKSolver() = default;
    
    virtual bool Solve(Skeleton* skeleton, int endEffector, const Vec3& target) = 0;
    
    void SetMaxIterations(int iterations) { maxIterations = iterations; }
    void SetThreshold(float t) { threshold = t; }
};

// CCD (Cyclic Coordinate Descent) IK
class CCDIKSolver : public IKSolver {
public:
    CCDIKSolver(int maxIterations = 10, float threshold = 0.01f);
    
    bool Solve(Skeleton* skeleton, int endEffector, const Vec3& target) override;
};

// FABRIK (Forward And Backward Reaching IK)
class FABRIKIKSolver : public IKSolver {
public:
    FABRIKIKSolver(int maxIterations = 10, float threshold = 0.01f);
    
    bool Solve(Skeleton* skeleton, int endEffector, const Vec3& target) override;
};

// Two-Bone IK (optimized for arms/legs)
class TwoBoneIKSolver : public IKSolver {
private:
    float minReach;
    float maxReach;
    
public:
    TwoBoneIKSolver(float minReach = 0.0f, float maxReach = 1.0f);
    
    bool Solve(Skeleton* skeleton, int endEffector, const Vec3& target) override;
    
    void SetReach(float min, float max) { minReach = min; maxReach = max; }
};

// ============================================
// IK System Integration
// ============================================
class IKSystem {
private:
    Skeleton* skeleton;
    std::vector<std::unique_ptr<IKSolver>> solvers;
    
    struct IKTarget {
        int endEffectorIndex;
        Vec3 targetPosition;
        float weight;
        bool enabled;
    };
    
    std::vector<IKTarget> targets;
    
public:
    IKSystem(Skeleton* skeleton);
    ~IKSystem();
    
    // Solver management
    void AddSolver(std::unique_ptr<IKSolver> solver);
    void ClearSolvers();
    
    // Targets
    void SetTarget(int endEffector, const Vec3& position, float weight = 1.0f);
    void ClearTarget(int endEffector);
    void DisableTarget(int endEffector);
    
    // Solve all IK
    void Solve();
    
    // Common presets
    void SetupLegIK(int hip, int knee, int ankle, int toe);
    void SetupArmIK(int shoulder, int elbow, int wrist, int hand);
    void SetupHeadIK(int neck, int head);
    void SetupSpineIK(const std::vector<int>& spineJoints);
};

} // namespace vge
