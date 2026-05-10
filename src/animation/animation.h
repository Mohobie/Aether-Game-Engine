#pragma once
#include "math/vec3.h"
#include "math/mat4.h"
#include <vector>
#include <string>
#include <memory>

namespace vge {

// ============================================
// Joint/Bone
// ============================================
struct Joint {
    std::string name;
    int parentIndex;  // -1 for root
    Vec3 localPosition;
    Vec3 localRotation;  // Euler angles
    Vec3 localScale;
    
    // Bind pose (model space)
    Mat4 inverseBindPose;
    
    // Runtime transforms
    Mat4 localTransform;
    Mat4 globalTransform;
    Mat4 skinningMatrix;
    
    Joint() : parentIndex(-1), localScale(1, 1, 1) {}
};

// ============================================
// Skeleton
// ============================================
class Skeleton {
private:
    std::vector<Joint> joints;
    std::vector<int> rootJoints;  // Indices of root joints
    
public:
    Skeleton();
    
    // Joint management
    int AddJoint(const std::string& name, int parentIndex = -1);
    Joint* GetJoint(int index);
    Joint* GetJoint(const std::string& name);
    int GetJointIndex(const std::string& name) const;
    int GetJointCount() const { return joints.size(); }
    
    // Calculate global transforms from local
    void UpdateTransforms();
    void UpdateTransforms(int jointIndex);
    
    // Calculate skinning matrices
    void CalculateSkinningMatrices();
    
    // Bind pose
    void CalculateInverseBindPoses();
    
    // Get root joints
    const std::vector<int>& GetRootJoints() const { return rootJoints; }
    
    // Debug
    void PrintHierarchy() const;
};

// ============================================
// Animation Keyframe
// ============================================
struct Keyframe {
    float time;  // Time in seconds
    Vec3 position;
    Vec3 rotation;
    Vec3 scale;
    
    Keyframe() : time(0), scale(1, 1, 1) {}
};

// ============================================
// Animation Track (for one joint)
// ============================================
struct AnimationTrack {
    int jointIndex;
    std::vector<Keyframe> keyframes;
    
    // Sample at specific time
    void Sample(float time, Vec3& outPos, Vec3& outRot, Vec3& outScale, bool loop = true);
    
    // Get duration
    float GetDuration() const;
};

// ============================================
// Animation Clip
// ============================================
class AnimationClip {
private:
    std::string name;
    std::vector<AnimationTrack> tracks;
    float duration;
    float framesPerSecond;
    
public:
    AnimationClip(const std::string& name = "Animation");
    
    // Track management
    void AddTrack(const AnimationTrack& track);
    AnimationTrack* GetTrack(int jointIndex);
    
    // Sampling
    void Sample(float time, Skeleton& skeleton, bool loop = true);
    
    // Properties
    float GetDuration() const { return duration; }
    void SetDuration(float d) { duration = d; }
    float GetFPS() const { return framesPerSecond; }
    void SetFPS(float fps) { framesPerSecond = fps; }
    const std::string& GetName() const { return name; }
    
    // Create from data
    static AnimationClip* CreateFromKeyframes(const std::string& name, 
                                               const std::vector<AnimationTrack>& tracks);
};

// ============================================
// Animator - Plays animations on skeleton
// ============================================
class Animator {
private:
    Skeleton* skeleton;
    AnimationClip* currentClip;
    AnimationClip* nextClip;  // For blending
    
    float currentTime;
    float blendTime;
    float blendDuration;
    bool isPlaying;
    bool loop;
    float playbackSpeed;
    
public:
    Animator(Skeleton* skeleton = nullptr);
    
    // Playback control
    void Play(AnimationClip* clip, bool loop = true, float speed = 1.0f);
    void Stop();
    void Pause();
    void Resume();
    void Update(float deltaTime);
    
    // Blending
    void CrossFade(AnimationClip* clip, float blendDuration, bool loop = true);
    
    // State
    bool IsPlaying() const { return isPlaying; }
    float GetCurrentTime() const { return currentTime; }
    void SetTime(float time) { currentTime = time; }
    
    // Skeleton
    void SetSkeleton(Skeleton* skel) { skeleton = skel; }
    Skeleton* GetSkeleton() const { return skeleton; }
    
    // Current clip
    AnimationClip* GetCurrentClip() const { return currentClip; }
};

// ============================================
// Skinned Mesh
// ============================================
struct VertexWeight {
    int jointIndex;
    float weight;
    
    VertexWeight() : jointIndex(0), weight(0) {}
    VertexWeight(int joint, float w) : jointIndex(joint), weight(w) {}
};

struct SkinnedVertex {
    Vec3 position;
    Vec3 normal;
    Vec3 texCoord;  // Using Vec3 instead of Vec2 for compatibility
    VertexWeight weights[4];  // Max 4 bone influences
    
    SkinnedVertex() {
        for (int i = 0; i < 4; ++i) {
            weights[i] = VertexWeight(0, 0);
        }
    }
};

class SkinnedMesh {
private:
    std::vector<SkinnedVertex> vertices;
    std::vector<uint32_t> indices;
    Skeleton* skeleton;
    
public:
    SkinnedMesh(Skeleton* skeleton = nullptr);
    
    // Data
    void AddVertex(const SkinnedVertex& v);
    void AddIndex(uint32_t i);
    void SetSkeleton(Skeleton* skel) { skeleton = skel; }
    
    // Skinning
    void UpdateSkinning();
    void ApplySkinning(std::vector<Vec3>& outPositions, std::vector<Vec3>& outNormals);
    
    // Getters
    size_t GetVertexCount() const { return vertices.size(); }
    size_t GetIndexCount() const { return indices.size(); }
    const std::vector<SkinnedVertex>& GetVertices() const { return vertices; }
};

// ============================================
// Animation System
// ============================================
class AnimationSystem {
private:
    std::vector<std::unique_ptr<Skeleton>> skeletons;
    std::vector<std::unique_ptr<AnimationClip>> clips;
    std::vector<std::unique_ptr<Animator>> animators;
    
public:
    AnimationSystem();
    ~AnimationSystem();
    
    // Creation
    Skeleton* CreateSkeleton(const std::string& name = "Skeleton");
    AnimationClip* CreateClip(const std::string& name);
    Animator* CreateAnimator(Skeleton* skeleton);
    
    // Updates
    void Update(float deltaTime);
    
    // Get all animators
    std::vector<Animator*> GetAnimators();
    
    // Stats
    size_t GetSkeletonCount() const { return skeletons.size(); }
    size_t GetClipCount() const { return clips.size(); }
    size_t GetAnimatorCount() const { return animators.size(); }
    
    void Clear();
};

} // namespace vge
