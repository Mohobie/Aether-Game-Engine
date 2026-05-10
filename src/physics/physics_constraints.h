#pragma once
#include "math/vec3.h"
#include "math/mat4.h"
#include <string>
#include <vector>
#include <memory>

namespace vge {

// Forward declarations
class Rigidbody;
class PhysicsWorld;

// ============================================
// Joint Types
// ============================================
enum class JointType {
    Fixed,      // No relative motion
    Hinge,      // Rotation around one axis
    Slider,     // Translation along one axis
    BallSocket, // Spherical rotation
    Universal,  // Two-axis rotation
    Distance,   // Fixed distance constraint
    Spring,     // Spring-damper
    Ragdoll     // Special joint for ragdolls
};

// ============================================
// Joint Base Class
// ============================================
class Joint {
protected:
    JointType type;
    Rigidbody* bodyA;
    Rigidbody* bodyB;
    
    // Local anchor points
    Vec3 localAnchorA;
    Vec3 localAnchorB;
    
    // World anchor points (updated each frame)
    Vec3 worldAnchorA;
    Vec3 worldAnchorB;
    
    // Constraint forces
    Vec3 linearImpulse;
    Vec3 angularImpulse;
    
    // Limits
    bool hasLimits;
    float minLimit;
    float maxLimit;
    
    // Motor
    bool hasMotor;
    float motorSpeed;
    float maxMotorForce;
    float motorImpulse;
    
    // Break threshold
    float breakForce;
    float breakTorque;
    bool broken;
    
    // Solver data
    float bias;
    float softness;
    float effectiveMass;
    
public:
    Joint(JointType type, Rigidbody* bodyA, Rigidbody* bodyB);
    virtual ~Joint();
    
    // Setup
    void SetLocalAnchorA(const Vec3& anchor) { localAnchorA = anchor; }
    void SetLocalAnchorB(const Vec3& anchor) { localAnchorB = anchor; }
    Vec3 GetLocalAnchorA() const { return localAnchorA; }
    Vec3 GetLocalAnchorB() const { return localAnchorB; }
    
    // Limits
    void EnableLimits(bool enable) { hasLimits = enable; }
    void SetLimits(float min, float max) { minLimit = min; maxLimit = max; hasLimits = true; }
    bool HasLimits() const { return hasLimits; }
    float GetMinLimit() const { return minLimit; }
    float GetMaxLimit() const { return maxLimit; }
    
    // Motor
    void EnableMotor(bool enable) { hasMotor = enable; }
    void SetMotorSpeed(float speed) { motorSpeed = speed; hasMotor = true; }
    void SetMaxMotorForce(float force) { maxMotorForce = force; }
    bool HasMotor() const { return hasMotor; }
    float GetMotorSpeed() const { return motorSpeed; }
    
    // Breakable
    void SetBreakThreshold(float force, float torque) { breakForce = force; breakTorque = torque; }
    bool IsBroken() const { return broken; }
    void Break();
    
    // Core
    virtual void Initialize();
    virtual void Solve(float deltaTime);
    virtual void ApplyImpulse();
    virtual float GetValue() const = 0; // Current joint value (angle, distance, etc.)
    
    // Update world anchors
    void UpdateWorldAnchors();
    
    // Type
    JointType GetType() const { return type; }
    Rigidbody* GetBodyA() const { return bodyA; }
    Rigidbody* GetBodyB() const { return bodyB; }
};

// ============================================
// Hinge Joint (Revolute)
// ============================================
class HingeJoint : public Joint {
private:
    Vec3 axisA; // Local rotation axis on bodyA
    Vec3 axisB; // Local rotation axis on bodyB
    Vec3 worldAxis;
    
    float currentAngle;
    float previousAngle;
    float angleAccumulated;
    
public:
    HingeJoint(Rigidbody* bodyA, Rigidbody* bodyB);
    
    void SetAxis(const Vec3& axis) { axisA = axis; axisB = axis; }
    void SetAxes(const Vec3& axisA, const Vec3& axisB) { this->axisA = axisA; this->axisB = axisB; }
    
    float GetAngle() const { return currentAngle; }
    float GetAngularVelocity() const;
    
    void Initialize() override;
    void Solve(float deltaTime) override;
    void ApplyImpulse() override;
    float GetValue() const override { return currentAngle; }
};

// ============================================
// Slider Joint (Prismatic)
// ============================================
class SliderJoint : public Joint {
private:
    Vec3 axisA;
    Vec3 axisB;
    Vec3 worldAxis;
    
    float currentTranslation;
    
public:
    SliderJoint(Rigidbody* bodyA, Rigidbody* bodyB);
    
    void SetAxis(const Vec3& axis) { axisA = axis; axisB = axis; }
    
    float GetTranslation() const { return currentTranslation; }
    float GetVelocity() const;
    
    void Initialize() override;
    void Solve(float deltaTime) override;
    void ApplyImpulse() override;
    float GetValue() const override { return currentTranslation; }
};

// ============================================
// Ball Socket Joint (Spherical)
// ============================================
class BallSocketJoint : public Joint {
private:
    Vec3 swingAxis1;
    Vec3 swingAxis2;
    Vec3 twistAxis;
    
    float swingAngle1;
    float swingAngle2;
    float twistAngle;
    
public:
    BallSocketJoint(Rigidbody* bodyA, Rigidbody* bodyB);
    
    void SetSwingLimits(float angle1, float angle2);
    void SetTwistLimits(float min, float max);
    
    void Initialize() override;
    void Solve(float deltaTime) override;
    void ApplyImpulse() override;
    float GetValue() const override { return swingAngle1; }
};

// ============================================
// Distance Joint
// ============================================
class DistanceJoint : public Joint {
private:
    float restLength;
    float minDistance;
    float maxDistance;
    float stiffness;
    float damping;
    
public:
    DistanceJoint(Rigidbody* bodyA, Rigidbody* bodyB);
    
    void SetRestLength(float length) { restLength = length; }
    void SetDistanceRange(float min, float max) { minDistance = min; maxDistance = max; }
    void SetSpringProperties(float stiffness, float damping) { this->stiffness = stiffness; this->damping = damping; }
    
    float GetCurrentDistance() const;
    
    void Initialize() override;
    void Solve(float deltaTime) override;
    void ApplyImpulse() override;
    float GetValue() const override { return GetCurrentDistance(); }
};

// ============================================
// Spring Joint
// ============================================
class SpringJoint : public Joint {
private:
    float restLength;
    float springStrength;
    float dampingRatio;
    
public:
    SpringJoint(Rigidbody* bodyA, Rigidbody* bodyB);
    
    void SetRestLength(float length) { restLength = length; }
    void SetSpringProperties(float strength, float damping) { springStrength = strength; dampingRatio = damping; }
    
    void Initialize() override;
    void Solve(float deltaTime) override;
    void ApplyImpulse() override;
    float GetValue() const override { return GetCurrentDistance(); }
    
private:
    float GetCurrentDistance() const;
};

// ============================================
// Fixed Joint
// ============================================
class FixedJoint : public Joint {
public:
    FixedJoint(Rigidbody* bodyA, Rigidbody* bodyB);
    
    void Initialize() override;
    void Solve(float deltaTime) override;
    void ApplyImpulse() override;
    float GetValue() const override { return 0.0f; }
};

// ============================================
// Ragdoll Joint (Humanoid)
// ============================================
class RagdollJoint : public Joint {
private:
    float swing1Limit;
    float swing2Limit;
    float twistLimit;
    
public:
    RagdollJoint(Rigidbody* bodyA, Rigidbody* bodyB);
    
    void SetSwingLimits(float swing1, float swing2) { swing1Limit = swing1; swing2Limit = swing2; }
    void SetTwistLimit(float twist) { twistLimit = twist; }
    
    void Initialize() override;
    void Solve(float deltaTime) override;
    void ApplyImpulse() override;
    float GetValue() const override { return 0.0f; }
};

// ============================================
// Ragdoll System
// ============================================
struct RagdollBone {
    std::string name;
    Rigidbody* body;
    Vec3 localPosition;
    Vec3 localRotation;
    Vec3 size;
    float mass;
};

struct RagdollJointConfig {
    std::string name;
    std::string parentBone;
    std::string childBone;
    JointType type;
    Vec3 localAnchor;
    Vec3 axis;
    float minLimit;
    float maxLimit;
};

class RagdollSystem {
private:
    std::vector<std::unique_ptr<RagdollBone>> bones;
    std::vector<std::unique_ptr<Joint>> joints;
    PhysicsWorld* physicsWorld;
    
    bool active;
    bool kinematicWhenInactive;
    
public:
    RagdollSystem(PhysicsWorld* world);
    ~RagdollSystem();
    
    // Setup
    void AddBone(const std::string& name, const Vec3& localPos, const Vec3& localRot, 
                 const Vec3& size, float mass);
    void AddJoint(const std::string& name, const std::string& parent, const std::string& child,
                  JointType type, const Vec3& localAnchor, const Vec3& axis,
                  float minLimit, float maxLimit);
    
    // Build ragdoll from skeleton
    void BuildFromSkeleton(const std::vector<RagdollBone>& boneDefs,
                          const std::vector<RagdollJointConfig>& jointDefs);
    
    // Activation
    void Activate();
    void Deactivate();
    bool IsActive() const { return active; }
    
    // Control
    void SetKinematicWhenInactive(bool enable) { kinematicWhenInactive = enable; }
    void ResetToPose();
    
    // Access
    Rigidbody* GetBone(const std::string& name) const;
    Joint* GetJoint(const std::string& name) const;
    std::vector<Rigidbody*> GetAllBones() const;
    
    // Update
    void Update(float deltaTime);
    void ApplyPose(const std::vector<std::pair<std::string, Mat4>>& boneTransforms);
};

// ============================================
// Constraint Solver
// ============================================
class ConstraintSolver {
private:
    std::vector<Joint*> joints;
    int iterations;
    float biasFactor;
    
public:
    ConstraintSolver(int iterations = 10);
    
    void AddJoint(Joint* joint);
    void RemoveJoint(Joint* joint);
    void ClearJoints();
    
    void PreSolve(float deltaTime);
    void Solve(float deltaTime);
    void PostSolve();
    
    void SetIterations(int iter) { iterations = iter; }
    void SetBiasFactor(float factor) { biasFactor = factor; }
};

// ============================================
// Breakable Constraints
// ============================================
class BreakableConstraint {
private:
    Joint* joint;
    float accumulatedForce;
    float accumulatedTorque;
    
public:
    BreakableConstraint(Joint* joint);
    
    void Update(float deltaTime);
    bool ShouldBreak() const;
    void Break();
    
    float GetAccumulatedForce() const { return accumulatedForce; }
    float GetAccumulatedTorque() const { return accumulatedTorque; }
};

} // namespace vge