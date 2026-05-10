#include "physics_constraints.h"
#include "physics_system.h"
#include <iostream>
#include <cmath>
#include <string>

namespace vge {

// ============================================
// Joint Base Implementation
// ============================================

Joint::Joint(JointType type, Rigidbody* bodyA, Rigidbody* bodyB)
    : type(type)
    , bodyA(bodyA)
    , bodyB(bodyB)
    , localAnchorA(0, 0, 0)
    , localAnchorB(0, 0, 0)
    , linearImpulse(0, 0, 0)
    , angularImpulse(0, 0, 0)
    , hasLimits(false)
    , minLimit(0)
    , maxLimit(0)
    , hasMotor(false)
    , motorSpeed(0)
    , maxMotorForce(0)
    , motorImpulse(0)
    , breakForce(std::numeric_limits<float>::max())
    , breakTorque(std::numeric_limits<float>::max())
    , broken(false)
    , bias(0)
    , softness(0)
    , effectiveMass(0) {}

Joint::~Joint() {}

void Joint::Initialize() {
    UpdateWorldAnchors();
}

void Joint::Solve(float deltaTime) {
    (void)deltaTime;
    // Base implementation - override in derived classes
}

void Joint::ApplyImpulse() {
    // Base implementation - override in derived classes
}

void Joint::UpdateWorldAnchors() {
    // Would transform local anchors to world space using body transforms
    worldAnchorA = localAnchorA;
    worldAnchorB = localAnchorB;
}

void Joint::Break() {
    broken = true;
    std::cout << "[Joint] Joint broken!" << std::endl;
}

// ============================================
// Hinge Joint Implementation
// ============================================

HingeJoint::HingeJoint(Rigidbody* bodyA, Rigidbody* bodyB)
    : Joint(JointType::Hinge, bodyA, bodyB)
    , axisA(0, 1, 0)
    , axisB(0, 1, 0)
    , worldAxis(0, 1, 0)
    , currentAngle(0)
    , previousAngle(0)
    , angleAccumulated(0) {}

void HingeJoint::Initialize() {
    Joint::Initialize();
    
    // Calculate initial angle
    previousAngle = 0;
    angleAccumulated = 0;
    
    std::cout << "[HingeJoint] Initialized" << std::endl;
}

float HingeJoint::GetAngularVelocity() const {
    // Would calculate from body angular velocities
    return 0.0f;
}

void HingeJoint::Solve(float deltaTime) {
    if (broken) return;
    (void)deltaTime;
    
    UpdateWorldAnchors();
    
    // Calculate current angle
    // Would use body rotations to calculate hinge angle
    currentAngle = 0; // Placeholder
    
    // Check limits
    if (hasLimits) {
        if (currentAngle < minLimit || currentAngle > maxLimit) {
            // Apply limit correction
            float targetAngle = std::max(minLimit, std::min(maxLimit, currentAngle));
            float error = targetAngle - currentAngle;
            
            // Would apply angular impulse to correct
            (void)error;
        }
    }
    
    // Apply motor
    if (hasMotor) {
        float currentSpeed = GetAngularVelocity();
        float speedError = motorSpeed - currentSpeed;
        
        float motorImpulseDelta = speedError * effectiveMass;
        motorImpulseDelta = std::max(-maxMotorForce, std::min(maxMotorForce, motorImpulseDelta));
        
        motorImpulse += motorImpulseDelta;
        motorImpulse = std::max(-maxMotorForce, std::min(maxMotorForce, motorImpulse));
        
        // Would apply motor impulse
        (void)speedError;
    }
    
    // Check break threshold
    float force = linearImpulse.length();
    float torque = angularImpulse.length();
    
    if (force > breakForce || torque > breakTorque) {
        Break();
    }
}

void HingeJoint::ApplyImpulse() {
    if (broken) return;
    
    // Would apply accumulated impulses to bodies
    // This is where the actual physics integration happens
}

// ============================================
// Slider Joint Implementation
// ============================================

SliderJoint::SliderJoint(Rigidbody* bodyA, Rigidbody* bodyB)
    : Joint(JointType::Slider, bodyA, bodyB)
    , axisA(1, 0, 0)
    , axisB(1, 0, 0)
    , worldAxis(1, 0, 0)
    , currentTranslation(0) {}

void SliderJoint::Initialize() {
    Joint::Initialize();
    std::cout << "[SliderJoint] Initialized" << std::endl;
}

float SliderJoint::GetVelocity() const {
    // Would calculate relative velocity along axis
    return 0.0f;
}

void SliderJoint::Solve(float deltaTime) {
    if (broken) return;
    (void)deltaTime;
    
    UpdateWorldAnchors();
    
    // Calculate current translation
    Vec3 delta = worldAnchorB - worldAnchorA;
    currentTranslation = delta.dot(worldAxis);
    
    // Check limits
    if (hasLimits) {
        if (currentTranslation < minLimit || currentTranslation > maxLimit) {
            float targetTranslation = std::max(minLimit, std::min(maxLimit, currentTranslation));
            float error = targetTranslation - currentTranslation;
            
            // Would apply linear impulse to correct
            (void)error;
        }
    }
    
    // Apply motor
    if (hasMotor) {
        float currentSpeed = GetVelocity();
        float speedError = motorSpeed - currentSpeed;
        
        // Would apply motor impulse
        (void)speedError;
    }
    
    // Check break threshold
    float force = linearImpulse.length();
    if (force > breakForce) {
        Break();
    }
}

void SliderJoint::ApplyImpulse() {
    if (broken) return;
    // Would apply impulses
}

// ============================================
// Ball Socket Joint Implementation
// ============================================

BallSocketJoint::BallSocketJoint(Rigidbody* bodyA, Rigidbody* bodyB)
    : Joint(JointType::BallSocket, bodyA, bodyB)
    , swingAxis1(1, 0, 0)
    , swingAxis2(0, 1, 0)
    , twistAxis(0, 0, 1)
    , swingAngle1(0)
    , swingAngle2(0)
    , twistAngle(0) {}

void BallSocketJoint::SetSwingLimits(float angle1, float angle2) {
    swingAngle1 = angle1;
    swingAngle2 = angle2;
    hasLimits = true;
}

void BallSocketJoint::SetTwistLimits(float min, float max) {
    minLimit = min;
    maxLimit = max;
    hasLimits = true;
}

void BallSocketJoint::Initialize() {
    Joint::Initialize();
    std::cout << "[BallSocketJoint] Initialized" << std::endl;
}

void BallSocketJoint::Solve(float deltaTime) {
    if (broken) return;
    (void)deltaTime;
    
    UpdateWorldAnchors();
    
    // Constrain to common point
    Vec3 delta = worldAnchorB - worldAnchorA;
    float distance = delta.length();
    
    if (distance > 0.001f) {
        // Would apply position correction
        Vec3 correction = delta * (1.0f / distance);
        (void)correction;
    }
    
    // Check swing limits
    if (hasLimits) {
        // Would check swing angles against limits
    }
    
    // Check break threshold
    float force = linearImpulse.length();
    if (force > breakForce) {
        Break();
    }
}

void BallSocketJoint::ApplyImpulse() {
    if (broken) return;
    // Would apply impulses
}

// ============================================
// Distance Joint Implementation
// ============================================

DistanceJoint::DistanceJoint(Rigidbody* bodyA, Rigidbody* bodyB)
    : Joint(JointType::Distance, bodyA, bodyB)
    , restLength(1.0f)
    , minDistance(0.0f)
    , maxDistance(std::numeric_limits<float>::max())
    , stiffness(0.0f)
    , damping(0.0f) {}

float DistanceJoint::GetCurrentDistance() const {
    return (worldAnchorB - worldAnchorA).length();
}

void DistanceJoint::Initialize() {
    Joint::Initialize();
    
    // Calculate rest length if not set
    if (restLength <= 0) {
        restLength = GetCurrentDistance();
    }
    
    std::cout << "[DistanceJoint] Initialized with rest length: " << restLength << std::endl;
}

void DistanceJoint::Solve(float deltaTime) {
    if (broken) return;
    (void)deltaTime;
    
    UpdateWorldAnchors();
    
    float currentDistance = GetCurrentDistance();
    
    // Distance constraint
    if (currentDistance > 0.001f) {
        Vec3 delta = worldAnchorB - worldAnchorA;
        Vec3 direction = delta * (1.0f / currentDistance);
        
        float error = 0.0f;
        
        if (currentDistance < minDistance) {
            error = currentDistance - minDistance;
        } else if (currentDistance > maxDistance) {
            error = currentDistance - maxDistance;
        } else if (stiffness > 0.0f) {
            // Spring behavior
            error = currentDistance - restLength;
        }
        
        if (std::abs(error) > 0.001f) {
            // Would apply spring/distance correction
            (void)direction;
            (void)error;
        }
    }
    
    // Check break threshold
    float force = linearImpulse.length();
    if (force > breakForce) {
        Break();
    }
}

void DistanceJoint::ApplyImpulse() {
    if (broken) return;
    // Would apply impulses
}

// ============================================
// Spring Joint Implementation
// ============================================

SpringJoint::SpringJoint(Rigidbody* bodyA, Rigidbody* bodyB)
    : Joint(JointType::Spring, bodyA, bodyB)
    , restLength(1.0f)
    , springStrength(100.0f)
    , dampingRatio(0.1f) {}

float SpringJoint::GetCurrentDistance() const {
    return (worldAnchorB - worldAnchorA).length();
}

void SpringJoint::Initialize() {
    Joint::Initialize();
    
    if (restLength <= 0) {
        restLength = GetCurrentDistance();
    }
    
    std::cout << "[SpringJoint] Initialized with rest length: " << restLength 
              << " strength: " << springStrength << std::endl;
}

void SpringJoint::Solve(float deltaTime) {
    if (broken) return;
    (void)deltaTime;
    
    UpdateWorldAnchors();
    
    Vec3 delta = worldAnchorB - worldAnchorA;
    float currentDistance = delta.length();
    
    if (currentDistance > 0.001f) {
        Vec3 direction = delta * (1.0f / currentDistance);
        
        // Spring force: F = -k * (x - restLength) - c * velocity
        float displacement = currentDistance - restLength;
        
        // Would calculate relative velocity for damping
        float relativeVelocity = 0.0f; // Placeholder
        
        float springForce = -springStrength * displacement;
        float dampingForce = -dampingRatio * relativeVelocity;
        float totalForce = springForce + dampingForce;
        
        // Would apply force to both bodies
        (void)direction;
        (void)totalForce;
    }
    
    // Check break threshold
    float force = linearImpulse.length();
    if (force > breakForce) {
        Break();
    }
}

void SpringJoint::ApplyImpulse() {
    if (broken) return;
    // Would apply impulses
}

// ============================================
// Fixed Joint Implementation
// ============================================

FixedJoint::FixedJoint(Rigidbody* bodyA, Rigidbody* bodyB)
    : Joint(JointType::Fixed, bodyA, bodyB) {}

void FixedJoint::Initialize() {
    Joint::Initialize();
    std::cout << "[FixedJoint] Initialized" << std::endl;
}

void FixedJoint::Solve(float deltaTime) {
    if (broken) return;
    (void)deltaTime;
    
    UpdateWorldAnchors();
    
    // Constrain positions to be equal
    Vec3 delta = worldAnchorB - worldAnchorA;
    float distance = delta.length();
    
    if (distance > 0.001f) {
        // Would apply position correction
        Vec3 correction = delta * (1.0f / distance);
        (void)correction;
    }
    
    // Would also constrain rotations to be equal
    
    // Check break threshold
    float force = linearImpulse.length();
    float torque = angularImpulse.length();
    
    if (force > breakForce || torque > breakTorque) {
        Break();
    }
}

void FixedJoint::ApplyImpulse() {
    if (broken) return;
    // Would apply impulses
}

// ============================================
// Ragdoll Joint Implementation
// ============================================

RagdollJoint::RagdollJoint(Rigidbody* bodyA, Rigidbody* bodyB)
    : Joint(JointType::Ragdoll, bodyA, bodyB)
    , swing1Limit(45.0f)
    , swing2Limit(45.0f)
    , twistLimit(45.0f) {}

void RagdollJoint::Initialize() {
    Joint::Initialize();
    std::cout << "[RagdollJoint] Initialized with swing limits: " 
              << swing1Limit << ", " << swing2Limit 
              << " twist limit: " << twistLimit << std::endl;
}

void RagdollJoint::Solve(float deltaTime) {
    if (broken) return;
    (void)deltaTime;
    
    UpdateWorldAnchors();
    
    // Constrain to common point (like ball socket)
    Vec3 delta = worldAnchorB - worldAnchorA;
    float distance = delta.length();
    
    if (distance > 0.001f) {
        // Would apply position correction
        Vec3 correction = delta * (1.0f / distance);
        (void)correction;
    }
    
    // Check swing and twist limits
    // Would calculate relative rotation and apply angular constraints
    
    // Check break threshold
    float force = linearImpulse.length();
    float torque = angularImpulse.length();
    
    if (force > breakForce || torque > breakTorque) {
        Break();
    }
}

void RagdollJoint::ApplyImpulse() {
    if (broken) return;
    // Would apply impulses
}

// ============================================
// Ragdoll System Implementation
// ============================================

RagdollSystem::RagdollSystem(PhysicsWorld* world)
    : physicsWorld(world)
    , active(false)
    , kinematicWhenInactive(true) {}

RagdollSystem::~RagdollSystem() {
    // Joints and bones cleaned up by unique_ptr
}

void RagdollSystem::AddBone(const std::string& name, const Vec3& localPos, 
                            const Vec3& localRot, const Vec3& size, float mass) {
    auto bone = std::make_unique<RagdollBone>();
    bone->name = name;
    bone->localPosition = localPos;
    bone->localRotation = localRot;
    bone->size = size;
    bone->mass = mass;
    bone->body = nullptr; // Would create rigidbody in physics world
    
    bones.push_back(std::move(bone));
    
    std::cout << "[RagdollSystem] Added bone: " << name << std::endl;
}

void RagdollSystem::AddJoint(const std::string& name, const std::string& parent,
                             const std::string& child, JointType type,
                             const Vec3& localAnchor, const Vec3& axis,
                             float minLimit, float maxLimit) {
    (void)name;
    (void)parent;
    (void)child;
    (void)type;
    (void)localAnchor;
    (void)axis;
    (void)minLimit;
    (void)maxLimit;
    
    // Would create joint between parent and child bones
    std::cout << "[RagdollSystem] Added joint: " << name << std::endl;
}

void RagdollSystem::BuildFromSkeleton(const std::vector<RagdollBone>& boneDefs,
                                       const std::vector<RagdollJointConfig>& jointDefs) {
    // Clear existing
    bones.clear();
    joints.clear();
    
    // Create bones
    for (const auto& boneDef : boneDefs) {
        AddBone(boneDef.name, boneDef.localPosition, boneDef.localRotation,
                boneDef.size, boneDef.mass);
    }
    
    // Create joints
    for (const auto& jointDef : jointDefs) {
        AddJoint(jointDef.name, jointDef.parentBone, jointDef.childBone,
                 jointDef.type, jointDef.localAnchor, jointDef.axis,
                 jointDef.minLimit, jointDef.maxLimit);
    }
    
    std::cout << "[RagdollSystem] Built ragdoll with " << bones.size() 
              << " bones and " << joints.size() << " joints" << std::endl;
}

void RagdollSystem::Activate() {
    active = true;
    
    // Enable physics simulation on all bones
    for (auto& bone : bones) {
        if (bone->body) {
            // Would set body to dynamic
        }
    }
    
    std::cout << "[RagdollSystem] Activated" << std::endl;
}

void RagdollSystem::Deactivate() {
    active = false;
    
    if (kinematicWhenInactive) {
        // Set bones to kinematic
        for (auto& bone : bones) {
            if (bone->body) {
                // Would set body to kinematic
            }
        }
    }
    
    std::cout << "[RagdollSystem] Deactivated" << std::endl;
}

void RagdollSystem::ResetToPose() {
    // Reset all bones to their initial local poses
    for (auto& bone : bones) {
        if (bone->body) {
            // Would reset body position and rotation
        }
    }
    
    std::cout << "[RagdollSystem] Reset to pose" << std::endl;
}

Rigidbody* RagdollSystem::GetBone(const std::string& name) const {
    for (const auto& bone : bones) {
        if (bone->name == name) {
            return bone->body;
        }
    }
    return nullptr;
}

Joint* RagdollSystem::GetJoint(const std::string& name) const {
    (void)name;
    // Would search joints by name
    return nullptr;
}

std::vector<Rigidbody*> RagdollSystem::GetAllBones() const {
    std::vector<Rigidbody*> result;
    for (const auto& bone : bones) {
        if (bone->body) {
            result.push_back(bone->body);
        }
    }
    return result;
}

void RagdollSystem::Update(float deltaTime) {
    if (!active) return;
    
    // Update joints
    for (auto& joint : joints) {
        joint->Solve(deltaTime);
        joint->ApplyImpulse();
    }
}

void RagdollSystem::ApplyPose(const std::vector<std::pair<std::string, Mat4>>& boneTransforms) {
    if (active) return; // Don't override physics when active
    
    for (const auto& [name, transform] : boneTransforms) {
        Rigidbody* body = GetBone(name);
        if (body) {
            // Would apply transform to body
            (void)transform;
        }
    }
}

// ============================================
// Constraint Solver Implementation
// ============================================

ConstraintSolver::ConstraintSolver(int iterations)
    : iterations(iterations)
    , biasFactor(0.2f) {}

void ConstraintSolver::AddJoint(Joint* joint) {
    joints.push_back(joint);
}

void ConstraintSolver::RemoveJoint(Joint* joint) {
    auto it = std::find(joints.begin(), joints.end(), joint);
    if (it != joints.end()) {
        joints.erase(it);
    }
}

void ConstraintSolver::ClearJoints() {
    joints.clear();
}

void ConstraintSolver::PreSolve(float deltaTime) {
    for (auto* joint : joints) {
        if (!joint->IsBroken()) {
            joint->Initialize();
            joint->Solve(deltaTime);
        }
    }
}

void ConstraintSolver::Solve(float deltaTime) {
    for (int i = 0; i < iterations; ++i) {
        for (auto* joint : joints) {
            if (!joint->IsBroken()) {
                joint->Solve(deltaTime);
                joint->ApplyImpulse();
            }
        }
    }
}

void ConstraintSolver::PostSolve() {
    // Clear accumulated impulses for next frame
    for (auto* joint : joints) {
        if (!joint->IsBroken()) {
            // Would reset impulses
        }
    }
}

// ============================================
// Breakable Constraint Implementation
// ============================================

BreakableConstraint::BreakableConstraint(Joint* joint)
    : joint(joint)
    , accumulatedForce(0)
    , accumulatedTorque(0) {}

void BreakableConstraint::Update(float deltaTime) {
    if (!joint || joint->IsBroken()) return;
    
    // Accumulate forces over time
    float currentForce = 0.0f; // Would get from joint
    float currentTorque = 0.0f; // Would get from joint
    
    accumulatedForce += currentForce * deltaTime;
    accumulatedTorque += currentTorque * deltaTime;
    
    // Decay accumulation
    accumulatedForce *= 0.99f;
    accumulatedTorque *= 0.99f;
}

bool BreakableConstraint::ShouldBreak() const {
    if (!joint) return false;
    
    return accumulatedForce > joint->GetBodyA()->GetMass() * 10.0f ||
           accumulatedTorque > joint->GetBodyA()->GetMass() * 5.0f;
}

void BreakableConstraint::Break() {
    if (joint) {
        joint->Break();
    }
}

} // namespace vge