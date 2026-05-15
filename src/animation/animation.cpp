#include "animation.h"
#include <iostream>
#include <algorithm>

namespace vge {

// ============================================
// Skeleton Implementation
// ============================================

Skeleton::Skeleton() {}

int Skeleton::AddJoint(const std::string& name, int parentIndex) {
    Joint joint;
    joint.name = name;
    joint.parentIndex = parentIndex;
    
    int index = static_cast<int>(joints.size());
    joints.push_back(joint);
    
    if (parentIndex == -1) {
        rootJoints.push_back(index);
    }
    
    return index;
}

Joint* Skeleton::GetJoint(int index) {
    if (index >= 0 && index < static_cast<int>(joints.size())) {
        return &joints[index];
    }
    return nullptr;
}

Joint* Skeleton::GetJoint(const std::string& name) {
    for (auto& joint : joints) {
        if (joint.name == name) {
            return &joint;
        }
    }
    return nullptr;
}

int Skeleton::GetJointIndex(const std::string& name) const {
    for (int i = 0; i < static_cast<int>(joints.size()); ++i) {
        if (joints[i].name == name) {
            return i;
        }
    }
    return -1;
}

void Skeleton::UpdateTransforms() {
    for (int rootIndex : rootJoints) {
        UpdateTransforms(rootIndex);
    }
}

void Skeleton::UpdateTransforms(int jointIndex) {
    Joint& joint = joints[jointIndex];
    
    // Build local transform matrix
    // Simplified: just use position for now
    joint.localTransform = Mat4();
    // Would build full TRS matrix here
    
    // Calculate global transform
    if (joint.parentIndex >= 0) {
        Joint& parent = joints[joint.parentIndex];
        joint.globalTransform = parent.globalTransform.Multiply(joint.localTransform);
    } else {
        joint.globalTransform = joint.localTransform;
    }
    
    // Update children
    for (int i = 0; i < static_cast<int>(joints.size()); ++i) {
        if (joints[i].parentIndex == jointIndex) {
            UpdateTransforms(i);
        }
    }
}

void Skeleton::CalculateInverseBindPoses() {
    UpdateTransforms();
    for (auto& joint : joints) {
        // For now, just use identity as inverse (would need proper matrix inversion)
        joint.inverseBindPose = Mat4::Identity();
    }
}

void Skeleton::CalculateSkinningMatrices() {
    UpdateTransforms();
    for (auto& joint : joints) {
        joint.skinningMatrix = joint.globalTransform.Multiply(joint.inverseBindPose);
    }
}

void Skeleton::PrintHierarchy() const {
    std::cout << "=== Skeleton Hierarchy ===" << std::endl;
    for (int i = 0; i < static_cast<int>(joints.size()); ++i) {
        const Joint& joint = joints[i];
        std::cout << "[" << i << "] " << joint.name;
        if (joint.parentIndex >= 0) {
            std::cout << " (parent: " << joints[joint.parentIndex].name << ")";
        } else {
            std::cout << " (root)";
        }
        std::cout << std::endl;
    }
}

// ============================================
// Animation Track Implementation
// ============================================

void AnimationTrack::Sample(float time, Vec3& outPos, Vec3& outRot, Vec3& outScale, bool loop) {
    if (keyframes.empty()) {
        outPos = Vec3(0, 0, 0);
        outRot = Vec3(0, 0, 0);
        outScale = Vec3(1, 1, 1);
        return;
    }
    
    float duration = GetDuration();
    if (loop && duration > 0) {
        time = fmod(time, duration);
    }
    
    // Find keyframes to interpolate between
    int frameIndex = 0;
    for (int i = 0; i < static_cast<int>(keyframes.size()) - 1; ++i) {
        if (time >= keyframes[i].time && time <= keyframes[i + 1].time) {
            frameIndex = i;
            break;
        }
    }
    
    const Keyframe& k1 = keyframes[frameIndex];
    const Keyframe& k2 = keyframes[(frameIndex + 1) % keyframes.size()];
    
    // Calculate interpolation factor
    float t = 0.0f;
    if (k2.time > k1.time) {
        t = (time - k1.time) / (k2.time - k1.time);
    }
    
    // Lerp
    outPos = k1.position + (k2.position - k1.position) * t;
    outRot = k1.rotation + (k2.rotation - k1.rotation) * t;
    outScale = k1.scale + (k2.scale - k1.scale) * t;
}

float AnimationTrack::GetDuration() const {
    if (keyframes.empty()) return 0.0f;
    return keyframes.back().time;
}

// ============================================
// Animation Clip Implementation
// ============================================

AnimationClip::AnimationClip(const std::string& name) 
    : name(name), duration(0), framesPerSecond(30) {}

void AnimationClip::AddTrack(const AnimationTrack& track) {
    tracks.push_back(track);
    // Update duration
    float trackDuration = track.GetDuration();
    if (trackDuration > duration) {
        duration = trackDuration;
    }
}

AnimationTrack* AnimationClip::GetTrack(int jointIndex) {
    for (auto& track : tracks) {
        if (track.jointIndex == jointIndex) {
            return &track;
        }
    }
    return nullptr;
}

void AnimationClip::Sample(float time, Skeleton& skeleton, bool loop) {
    for (auto& track : tracks) {
        Vec3 pos, rot, scale;
        track.Sample(time, pos, rot, scale, loop);
        
        Joint* joint = skeleton.GetJoint(track.jointIndex);
        if (joint) {
            joint->localPosition = pos;
            joint->localRotation = rot;
            joint->localScale = scale;
        }
    }
}

AnimationClip* AnimationClip::CreateFromKeyframes(const std::string& name, 
                                                   const std::vector<AnimationTrack>& tracks) {
    AnimationClip* clip = new AnimationClip(name);
    for (const auto& track : tracks) {
        clip->AddTrack(track);
    }
    return clip;
}

// ============================================
// Animator Implementation
// ============================================

Animator::Animator(Skeleton* skeleton)
    : skeleton(skeleton)
    , currentClip(nullptr)
    , nextClip(nullptr)
    , currentTime(0)
    , blendTime(0)
    , blendDuration(0)
    , isPlaying(false)
    , loop(true)
    , playbackSpeed(1.0f) {}

void Animator::Play(AnimationClip* clip, bool shouldLoop, float speed) {
    currentClip = clip;
    loop = shouldLoop;
    playbackSpeed = speed;
    currentTime = 0;
    isPlaying = true;
    nextClip = nullptr;
    blendTime = 0;
    
    std::cout << "[Animator] Playing: " << (clip ? clip->GetName() : "null") << std::endl;
}

void Animator::Stop() {
    isPlaying = false;
    currentTime = 0;
}

void Animator::Pause() {
    isPlaying = false;
}

void Animator::Resume() {
    isPlaying = true;
}

void Animator::Update(float deltaTime) {
    if (!isPlaying || !currentClip || !skeleton) return;
    
    currentTime += deltaTime * playbackSpeed;
    
    // Handle blending
    if (nextClip && blendDuration > 0) {
        blendTime += deltaTime;
        float blendFactor = std::min(blendTime / blendDuration, 1.0f);
        
        // Sample both clips and blend
        // For now, just switch when blend is complete
        if (blendFactor >= 1.0f) {
            currentClip = nextClip;
            nextClip = nullptr;
            blendTime = 0;
            blendDuration = 0;
        }
    }
    
    // Sample current animation
    currentClip->Sample(currentTime, *skeleton, loop);
    skeleton->UpdateTransforms();
    skeleton->CalculateSkinningMatrices();
}

void Animator::CrossFade(AnimationClip* clip, float duration, bool shouldLoop) {
    if (!currentClip) {
        Play(clip, shouldLoop);
        return;
    }
    
    nextClip = clip;
    blendDuration = duration;
    blendTime = 0;
    loop = shouldLoop;
    
    std::cout << "[Animator] Cross-fading to: " << clip->GetName() << " (" << duration << "s)" << std::endl;
}

// ============================================
// Skinned Mesh Implementation
// ============================================

SkinnedMesh::SkinnedMesh(Skeleton* skeleton) : skeleton(skeleton) {}

void SkinnedMesh::AddVertex(const SkinnedVertex& v) {
    vertices.push_back(v);
}

void SkinnedMesh::AddIndex(uint32_t i) {
    indices.push_back(i);
}

void SkinnedMesh::UpdateSkinning() {
    if (!skeleton) return;
    
    // Ensure skinning matrices are calculated
    skeleton->CalculateSkinningMatrices();
}

void SkinnedMesh::ApplySkinning(std::vector<Vec3>& outPositions, std::vector<Vec3>& outNormals) {
    outPositions.resize(vertices.size());
    outNormals.resize(vertices.size());
    
    for (size_t i = 0; i < vertices.size(); ++i) {
        const SkinnedVertex& vert = vertices[i];
        Vec3 skinnedPos(0, 0, 0);
        Vec3 skinnedNormal(0, 0, 0);
        
        for (int w = 0; w < 4; ++w) {
            const VertexWeight& weight = vert.weights[w];
            if (weight.weight <= 0) continue;
            
            Joint* joint = skeleton->GetJoint(weight.jointIndex);
            if (!joint) continue;
            
            // Apply skinning matrix
            const Mat4& skinMatrix = joint->skinningMatrix;
            
            // Transform position
            Vec3 transformedPos = skinMatrix.TransformPoint(vert.position);
            skinnedPos = skinnedPos + transformedPos * weight.weight;
            
            // Transform normal (use rotation only)
            Vec3 transformedNormal = skinMatrix.TransformPoint(vert.normal);
            skinnedNormal = skinnedNormal + transformedNormal * weight.weight;
        }
        
        outPositions[i] = skinnedPos;
        outNormals[i] = skinnedNormal.normalize();
    }
}

// ============================================
// Animation System Implementation
// ============================================

AnimationSystem::AnimationSystem() {}

AnimationSystem::~AnimationSystem() {
    Clear();
}

Skeleton* AnimationSystem::CreateSkeleton(const std::string& name) {
    auto skeleton = std::make_unique<Skeleton>();
    Skeleton* ptr = skeleton.get();
    skeletons.push_back(std::move(skeleton));
    std::cout << "[AnimationSystem] Created skeleton: " << name << std::endl;
    return ptr;
}

AnimationClip* AnimationSystem::CreateClip(const std::string& name) {
    auto clip = std::make_unique<AnimationClip>(name);
    AnimationClip* ptr = clip.get();
    clips.push_back(std::move(clip));
    return ptr;
}

Animator* AnimationSystem::CreateAnimator(Skeleton* skeleton) {
    auto animator = std::make_unique<Animator>(skeleton);
    Animator* ptr = animator.get();
    animators.push_back(std::move(animator));
    return ptr;
}

void AnimationSystem::Update(float deltaTime) {
    for (auto& animator : animators) {
        animator->Update(deltaTime);
    }
}

std::vector<Animator*> AnimationSystem::GetAnimators() {
    std::vector<Animator*> result;
    for (auto& animator : animators) {
        result.push_back(animator.get());
    }
    return result;
}

void AnimationSystem::Clear() {
    animators.clear();
    clips.clear();
    skeletons.clear();
}

} // namespace vge
