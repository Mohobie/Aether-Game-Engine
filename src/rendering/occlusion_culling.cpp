#include "occlusion_culling.h"
#include "core/logger.h"
#include <GL/gl.h>
#include <algorithm>
#include <cmath>

namespace vge {

// ============================================
// OcclusionQuery
// ============================================

OcclusionQuery::OcclusionQuery() : queryID(0), active(false) {}

OcclusionQuery::~OcclusionQuery() {
    Shutdown();
}

void OcclusionQuery::Initialize() {
    if (queryID == 0) {
        glGenQueries(1, &queryID);
    }
}

void OcclusionQuery::Shutdown() {
    if (queryID != 0) {
        glDeleteQueries(1, &queryID);
        queryID = 0;
    }
    active = false;
}

void OcclusionQuery::BeginQuery() {
    if (queryID == 0) Initialize();
    glBeginQuery(GL_OCCLUSION_QUERY, queryID);
    active = true;
}

void OcclusionQuery::EndQuery() {
    glEndQuery(GL_OCCLUSION_QUERY);
    active = false;
}

bool OcclusionQuery::IsVisible() const {
    if (queryID == 0) return true; // Conservative: assume visible
    GLuint samples = 0;
    glGetQueryObjectuiv(queryID, GL_QUERY_RESULT, &samples);
    return samples > 0;
}

uint32_t OcclusionQuery::GetVisiblePixels() const {
    if (queryID == 0) return 0;
    GLuint samples = 0;
    glGetQueryObjectuiv(queryID, GL_QUERY_RESULT, &samples);
    return samples;
}

bool OcclusionQuery::IsResultAvailable() const {
    if (queryID == 0) return false;
    GLint available = 0;
    glGetQueryObjectiv(queryID, GL_QUERY_RESULT_AVAILABLE, &available);
    return available != 0;
}

void OcclusionQuery::WaitForResult() const {
    if (queryID == 0) return;
    GLuint samples = 0;
    glGetQueryObjectuiv(queryID, GL_QUERY_RESULT, &samples);
}

// ============================================
// OcclusionCullingSystem
// ============================================

OcclusionCullingSystem::OcclusionCullingSystem()
    : maxQueries(256)
    , framesToKeepVisible(3)
    , maxQueriesPerFrame(128)
    , queriesUsedThisFrame(0)
    , useHardwareQueries(true)
    , useSoftwareFallback(true)
    , useTemporalCoherence(true)
    , depthBufferWidth(0)
    , depthBufferHeight(0)
{
}

OcclusionCullingSystem::~OcclusionCullingSystem() {
    Shutdown();
}

void OcclusionCullingSystem::Initialize(uint32_t maxQueries_) {
    maxQueries = maxQueries_;
    queryPool.resize(maxQueries);
    queryInUse.resize(maxQueries, false);
    
    for (uint32_t i = 0; i < maxQueries; ++i) {
        queryPool[i].Initialize();
    }
    
    Logger::Info("[OcclusionCulling] Initialized with " + std::to_string(maxQueries) + " queries");
}

void OcclusionCullingSystem::Shutdown() {
    for (auto& query : queryPool) {
        query.Shutdown();
    }
    queryPool.clear();
    queryInUse.clear();
    occluders.clear();
    occludees.clear();
    depthBuffer.clear();
}

void OcclusionCullingSystem::SetMaxQueriesPerFrame(uint32_t max) {
    maxQueriesPerFrame = max;
}

void OcclusionCullingSystem::SetFramesToKeepVisible(int frames) {
    framesToKeepVisible = frames;
}

void OcclusionCullingSystem::EnableHardwareQueries(bool enable) {
    useHardwareQueries = enable;
}

void OcclusionCullingSystem::EnableSoftwareFallback(bool enable) {
    useSoftwareFallback = enable;
}

void OcclusionCullingSystem::EnableTemporalCoherence(bool enable) {
    useTemporalCoherence = enable;
}

int OcclusionCullingSystem::AcquireQuery() {
    for (uint32_t i = 0; i < maxQueries; ++i) {
        if (!queryInUse[i]) {
            queryInUse[i] = true;
            return static_cast<int>(i);
        }
    }
    return -1; // No queries available
}

void OcclusionCullingSystem::ReleaseQuery(int index) {
    if (index >= 0 && index < static_cast<int>(maxQueries)) {
        queryInUse[index] = false;
    }
}

void OcclusionCullingSystem::ReleaseAllQueries() {
    std::fill(queryInUse.begin(), queryInUse.end(), false);
}

void OcclusionCullingSystem::ClearOccluders() {
    occluders.clear();
}

void OcclusionCullingSystem::AddOccluder(const AABB& bounds, float effectiveRadius) {
    Occluder occ;
    occ.bounds = bounds;
    occ.effectiveRadius = effectiveRadius;
    occ.queryID = static_cast<uint32_t>(-1);
    occ.isVisible = true;
    occ.framesSinceVisible = 0;
    occluders.push_back(occ);
}

void OcclusionCullingSystem::UpdateOccluder(int index, const AABB& bounds) {
    if (index >= 0 && index < static_cast<int>(occluders.size())) {
        occluders[index].bounds = bounds;
    }
}

void OcclusionCullingSystem::ClearOccludees() {
    occludees.clear();
}

void OcclusionCullingSystem::AddOccludee(const AABB& bounds, float priority) {
    Occludee occ;
    occ.bounds = bounds;
    occ.queryID = static_cast<uint32_t>(-1);
    occ.isVisible = true; // Conservative: start visible
    occ.framesSinceVisible = 0;
    occ.priority = priority;
    occludees.push_back(occ);
}

void OcclusionCullingSystem::BeginOcclusionPass(const Mat4& viewProj) {
    lastViewProj = viewProj;
    queriesUsedThisFrame = 0;
    ReleaseAllQueries();
}

void OcclusionCullingSystem::RenderOccluders() {
    // Phase 1: Render large occluders to build depth buffer
    // In practice, this would render simplified geometry for large objects
    // For now, we just mark them as visible since they're the occluders
    for (auto& occluder : occluders) {
        occluder.isVisible = true;
        occluder.framesSinceVisible = 0;
    }
}

void OcclusionCullingSystem::TestOccludees() {
    if (useHardwareQueries) {
        // Phase 2a: Hardware occlusion queries
        for (size_t i = 0; i < occludees.size(); ++i) {
            if (queriesUsedThisFrame >= maxQueriesPerFrame) break;
            
            auto& occ = occludees[i];
            
            // Temporal coherence: skip testing if recently visible
            if (useTemporalCoherence && occ.framesSinceVisible < framesToKeepVisible) {
                occ.isVisible = true;
                occ.framesSinceVisible++;
                continue;
            }
            
            // Acquire query
            int queryIdx = AcquireQuery();
            if (queryIdx < 0) break;
            
            occ.queryID = static_cast<uint32_t>(queryIdx);
            
            // Begin query and render bounding box
            queryPool[queryIdx].BeginQuery();
            
            // Render AABB as simplified geometry
            // (In real implementation, use a simple box shader)
            RenderAABB(occ.bounds);
            
            queryPool[queryIdx].EndQuery();
            queriesUsedThisFrame++;
        }
    }
    
    if (useSoftwareFallback) {
        // Phase 2b: Software fallback using depth buffer
        for (size_t i = 0; i < occludees.size(); ++i) {
            auto& occ = occludees[i];
            
            // Skip if already tested with hardware
            if (occ.queryID != static_cast<uint32_t>(-1)) continue;
            
            // Test against depth buffer
            occ.isVisible = TestOccludeeAgainstDepthBuffer(occ);
        }
    }
}

void OcclusionCullingSystem::EndOcclusionPass() {
    // Read back hardware query results
    for (auto& occ : occludees) {
        if (occ.queryID != static_cast<uint32_t>(-1)) {
            int idx = static_cast<int>(occ.queryID);
            if (idx >= 0 && idx < static_cast<int>(maxQueries)) {
                occ.isVisible = queryPool[idx].IsVisible();
                if (occ.isVisible) {
                    occ.framesSinceVisible = 0;
                } else {
                    occ.framesSinceVisible++;
                }
            }
        }
    }
}

std::vector<int> OcclusionCullingSystem::GetVisibleOccludees() const {
    std::vector<int> visible;
    for (size_t i = 0; i < occludees.size(); ++i) {
        if (occludees[i].isVisible) {
            visible.push_back(static_cast<int>(i));
        }
    }
    return visible;
}

std::vector<int> OcclusionCullingSystem::GetVisibleOccluders() const {
    std::vector<int> visible;
    for (size_t i = 0; i < occluders.size(); ++i) {
        if (occluders[i].isVisible) {
            visible.push_back(static_cast<int>(i));
        }
    }
    return visible;
}

bool OcclusionCullingSystem::IsOccludeeVisible(int index) const {
    if (index >= 0 && index < static_cast<int>(occludees.size())) {
        return occludees[index].isVisible;
    }
    return true; // Conservative
}

bool OcclusionCullingSystem::IsOccluderVisible(int index) const {
    if (index >= 0 && index < static_cast<int>(occluders.size())) {
        return occluders[index].isVisible;
    }
    return true;
}

uint32_t OcclusionCullingSystem::GetVisibleCount() const {
    uint32_t count = 0;
    for (const auto& occ : occludees) {
        if (occ.isVisible) count++;
    }
    return count;
}

uint32_t OcclusionCullingSystem::GetCulledCount() const {
    return static_cast<uint32_t>(occludees.size()) - GetVisibleCount();
}

void OcclusionCullingSystem::UpdateDepthBuffer(const std::vector<float>& depths, 
                                                int width, int height, 
                                                const Mat4& viewProj) {
    depthBuffer = depths;
    depthBufferWidth = width;
    depthBufferHeight = height;
    lastViewProj = viewProj;
}

bool OcclusionCullingSystem::TestAABBAgainstDepthBuffer(const AABB& bounds, 
                                                        const Mat4& viewProj) const {
    // Transform AABB corners to screen space
    Vec3 corners[8] = {
        Vec3(bounds.min.x, bounds.min.y, bounds.min.z),
        Vec3(bounds.max.x, bounds.min.y, bounds.min.z),
        Vec3(bounds.min.x, bounds.max.y, bounds.min.z),
        Vec3(bounds.max.x, bounds.max.y, bounds.min.z),
        Vec3(bounds.min.x, bounds.min.y, bounds.max.z),
        Vec3(bounds.max.x, bounds.min.y, bounds.max.z),
        Vec3(bounds.min.x, bounds.max.y, bounds.max.z),
        Vec3(bounds.max.x, bounds.max.y, bounds.max.z)
    };
    
    // Find screen-space bounds
    float minX = 1.0f, minY = 1.0f, maxX = -1.0f, maxY = -1.0f;
    float minZ = 1.0f;
    
    for (int i = 0; i < 8; ++i) {
        // Transform to clip space
        Vec4 clip = viewProj * Vec4(corners[i].x, corners[i].y, corners[i].z, 1.0f);
        if (clip.w > 0) {
            float x = clip.x / clip.w;
            float y = clip.y / clip.w;
            float z = clip.z / clip.w;
            
            minX = std::min(minX, x);
            minY = std::min(minY, y);
            maxX = std::max(maxX, x);
            maxY = std::max(maxY, y);
            minZ = std::min(minZ, z);
        }
    }
    
    // Convert to pixel coordinates
    int pxMin = static_cast<int>((minX * 0.5f + 0.5f) * depthBufferWidth);
    int pxMax = static_cast<int>((maxX * 0.5f + 0.5f) * depthBufferWidth);
    int pyMin = static_cast<int>((minY * 0.5f + 0.5f) * depthBufferHeight);
    int pyMax = static_cast<int>((maxY * 0.5f + 0.5f) * depthBufferHeight);
    
    // Clamp
    pxMin = std::max(0, pxMin);
    pxMax = std::min(depthBufferWidth - 1, pxMax);
    pyMin = std::max(0, pyMin);
    pyMax = std::min(depthBufferHeight - 1, pyMax);
    
    // Check if any pixel in the box is closer than the AABB
    for (int y = pyMin; y <= pyMax; ++y) {
        for (int x = pxMin; x <= pxMax; ++x) {
            int idx = y * depthBufferWidth + x;
            if (idx >= 0 && idx < static_cast<int>(depthBuffer.size())) {
                if (depthBuffer[idx] < minZ) {
                    return true; // Something is in front
                }
            }
        }
    }
    
    return false; // Occluded
}

bool OcclusionCullingSystem::TestOccludeeAgainstDepthBuffer(const Occludee& occludee) const {
    return TestAABBAgainstDepthBuffer(occludee.bounds, lastViewProj);
}

void OcclusionCullingSystem::RenderAABB(const AABB& bounds) {
    // Simplified AABB rendering for occlusion test
    // In practice, use a shader that just writes depth
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_TRUE);
    
    // Render box faces
    // (Implementation would use a simple box VAO)
    
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

// ============================================
// HierarchicalZBuffer
// ============================================

HierarchicalZBuffer::HierarchicalZBuffer() 
    : baseWidth(0)
    , baseHeight(0)
    , numLevels(0)
{
}

HierarchicalZBuffer::~HierarchicalZBuffer() {
    Shutdown();
}

void HierarchicalZBuffer::Initialize(int width, int height) {
    baseWidth = width;
    baseHeight = height;
    
    // Calculate mip levels
    numLevels = 1;
    int w = width, h = height;
    while (w > 1 || h > 1) {
        w = std::max(1, w / 2);
        h = std::max(1, h / 2);
        numLevels++;
    }
    
    mipChain.resize(numLevels);
    
    // Allocate mip levels
    w = width; h = height;
    for (int i = 0; i < numLevels; ++i) {
        mipChain[i].resize(w * h);
        w = std::max(1, w / 2);
        h = std::max(1, h / 2);
    }
}

void HierarchicalZBuffer::Shutdown() {
    mipChain.clear();
    numLevels = 0;
}

void HierarchicalZBuffer::BuildFromDepthBuffer(const std::vector<float>& depthBuffer, 
                                                int width, int height) {
    if (width != baseWidth || height != baseHeight) {
        Initialize(width, height);
    }
    
    // Copy base level
    mipChain[0] = depthBuffer;
    
    // Build mip chain
    BuildMipChain();
}

void HierarchicalZBuffer::BuildMipChain() {
    int w = baseWidth;
    int h = baseHeight;
    
    for (int level = 1; level < numLevels; ++level) {
        int prevW = w;
        int prevH = h;
        w = std::max(1, w / 2);
        h = std::max(1, h / 2);
        
        auto& prevLevel = mipChain[level - 1];
        auto& currLevel = mipChain[level];
        
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                // Sample 2x2 from previous level, take min (closest depth)
                float d0 = SampleDepth(x * 2, y * 2, level - 1);
                float d1 = SampleDepth(x * 2 + 1, y * 2, level - 1);
                float d2 = SampleDepth(x * 2, y * 2 + 1, level - 1);
                float d3 = SampleDepth(x * 2 + 1, y * 2 + 1, level - 1);
                
                currLevel[y * w + x] = std::min(std::min(d0, d1), std::min(d2, d3));
            }
        }
    }
}

float HierarchicalZBuffer::SampleDepth(int x, int y, int level) const {
    int w = GetWidth(level);
    int h = GetHeight(level);
    x = std::max(0, std::min(x, w - 1));
    y = std::max(0, std::min(y, h - 1));
    return mipChain[level][y * w + x];
}

bool HierarchicalZBuffer::TestAABB(const AABB& bounds, const Mat4& viewProj) const {
    // Transform to screen space
    Vec3 corners[8];
    // ... (similar to OcclusionCullingSystem)
    
    // Find appropriate mip level based on screen size
    float screenSize = 0.0f; // Calculate from corners
    int mipLevel = 0;
    while (screenSize < 4.0f && mipLevel < numLevels - 1) {
        screenSize *= 2.0f;
        mipLevel++;
    }
    
    // Test against HZB at chosen mip level
    // (Conservative test)
    return true; // Placeholder
}

float HierarchicalZBuffer::GetDepth(int x, int y, int mipLevel) const {
    return SampleDepth(x, y, mipLevel);
}

int HierarchicalZBuffer::GetWidth(int mipLevel) const {
    return std::max(1, baseWidth >> mipLevel);
}

int HierarchicalZBuffer::GetHeight(int mipLevel) const {
    return std::max(1, baseHeight >> mipLevel);
}

// ============================================
// GPUCullingSystem
// ============================================

GPUCullingSystem::GPUCullingSystem()
    : cullComputeShader(0)
    , cullProgram(0)
    , objectDataSSBO(0)
    , drawCommandsSSBO(0)
    , cullDataSSBO(0)
    , indirectDrawBuffer(0)
    , initialized(false)
    , maxObjects(65536)
{
}

GPUCullingSystem::~GPUCullingSystem() {
    Shutdown();
}

void GPUCullingSystem::Initialize(uint32_t maxObjects_) {
    maxObjects = maxObjects_;
    
    // Create compute shader program
    // (Would compile compute shader here)
    
    // Create SSBOs
    glGenBuffers(1, &objectDataSSBO);
    glGenBuffers(1, &drawCommandsSSBO);
    glGenBuffers(1, &cullDataSSBO);
    glGenBuffers(1, &indirectDrawBuffer);
    
    // Allocate object data buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, objectDataSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 
                 maxObjects * sizeof(GPUObjectData), 
                 nullptr, GL_DYNAMIC_DRAW);
    
    // Allocate draw commands buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, drawCommandsSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 maxObjects * sizeof(GPUDrawCommand),
                 nullptr, GL_DYNAMIC_DRAW);
    
    initialized = true;
    Logger::Info("[GPUCulling] Initialized with max " + std::to_string(maxObjects) + " objects");
}

void GPUCullingSystem::Shutdown() {
    if (objectDataSSBO) glDeleteBuffers(1, &objectDataSSBO);
    if (drawCommandsSSBO) glDeleteBuffers(1, &drawCommandsSSBO);
    if (cullDataSSBO) glDeleteBuffers(1, &cullDataSSBO);
    if (indirectDrawBuffer) glDeleteBuffers(1, &indirectDrawBuffer);
    if (cullProgram) glDeleteProgram(cullProgram);
    
    objectDataSSBO = 0;
    drawCommandsSSBO = 0;
    cullDataSSBO = 0;
    indirectDrawBuffer = 0;
    cullProgram = 0;
    initialized = false;
}

void GPUCullingSystem::ClearObjects() {
    objectData.clear();
    drawCommands.clear();
}

void GPUCullingSystem::AddObject(const Vec3& center, float radius, 
                                  const AABB& bounds, 
                                  uint32_t indexCount, 
                                  uint32_t firstIndex) {
    GPUObjectData data;
    data.center = center;
    data.radius = radius;
    data.aabbMin = bounds.min;
    data.aabbMax = bounds.max;
    objectData.push_back(data);
    
    GPUDrawCommand cmd;
    cmd.indexCount = indexCount;
    cmd.instanceCount = 1;
    cmd.firstIndex = firstIndex;
    cmd.baseVertex = 0;
    cmd.baseInstance = 0;
    cmd.objectID = static_cast<uint32_t>(objectData.size()) - 1;
    drawCommands.push_back(cmd);
}

void GPUCullingSystem::UpdateObject(int index, const Vec3& center, float radius) {
    if (index >= 0 && index < static_cast<int>(objectData.size())) {
        objectData[index].center = center;
        objectData[index].radius = radius;
    }
}

void GPUCullingSystem::PerformCulling(const CullData& cullData) {
    if (!initialized || objectData.empty()) return;
    
    // Update object data buffer
    UpdateObjectDataBuffer();
    UpdateDrawCommandsBuffer();
    
    // Update cull data buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cullDataSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(CullData), &cullData, GL_DYNAMIC_DRAW);
    
    // Bind SSBOs
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, objectDataSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, drawCommandsSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, cullDataSSBO);
    
    // Dispatch compute shader
    // (Would use actual compute program)
    uint32_t numGroups = (static_cast<uint32_t>(objectData.size()) + 255) / 256;
    // glDispatchCompute(numGroups, 1, 1);
    
    // Barrier to ensure writes are visible
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_COMMAND_BARRIER_BIT);
}

uint32_t GPUCullingSystem::GetVisibleCount() const {
    // Would read back from GPU
    return static_cast<uint32_t>(objectData.size());
}

std::vector<GPUDrawCommand> GPUCullingSystem::GetVisibleCommands() const {
    // Would read back from GPU
    return drawCommands;
}

void GPUCullingSystem::ExecuteIndirectDraw() const {
    if (!initialized) return;
    
    // Bind indirect draw buffer
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectDrawBuffer);
    
    // Execute indirect draw commands
    // (Would iterate over visible commands)
}

void GPUCullingSystem::UpdateObjectDataBuffer() {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, objectDataSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 
                    objectData.size() * sizeof(GPUObjectData),
                    objectData.data());
}

void GPUCullingSystem::UpdateDrawCommandsBuffer() {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, drawCommandsSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,
                    drawCommands.size() * sizeof(GPUDrawCommand),
                    drawCommands.data());
}

const char* GPUCullingSystem::GetCullComputeShaderSource() {
    return R"(
        #version 430 core
        
        struct ObjectData {
            vec3 center;
            float radius;
            vec3 aabbMin;
            float pad1;
            vec3 aabbMax;
            float pad2;
        };
        
        struct DrawCommand {
            uint indexCount;
            uint instanceCount;
            uint firstIndex;
            int baseVertex;
            uint baseInstance;
            uint objectID;
        };
        
        struct CullData {
            mat4 viewMatrix;
            mat4 projMatrix;
            mat4 viewProjMatrix;
            vec3 cameraPosition;
            float cameraNear;
            float cameraFar;
            float fov;
            float aspectRatio;
            uint numObjects;
            uint frameIndex;
        };
        
        layout(std430, binding = 0) readonly buffer ObjectBuffer {
            ObjectData objects[];
        } objectBuffer;
        
        layout(std430, binding = 1) buffer DrawCommandBuffer {
            DrawCommand commands[];
        } commandBuffer;
        
        layout(std430, binding = 2) readonly buffer CullDataBuffer {
            CullData cullData;
        } cullDataBuffer;
        
        layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;
        
        bool SphereInFrustum(vec3 center, float radius, mat4 viewProj) {
            vec4 clip = viewProj * vec4(center, 1.0);
            float r = radius * max(viewProj[0][0], viewProj[1][1]);
            
            return clip.x >= -clip.w - r && clip.x <= clip.w + r &&
                   clip.y >= -clip.w - r && clip.y <= clip.w + r &&
                   clip.z >= -clip.w - r && clip.z <= clip.w + r;
        }
        
        void main() {
            uint idx = gl_GlobalInvocationID.x;
            if (idx >= cullDataBuffer.cullData.numObjects) return;
            
            ObjectData obj = objectBuffer.objects[idx];
            DrawCommand cmd = commandBuffer.commands[idx];
            
            // Frustum cull
            bool visible = SphereInFrustum(obj.center, obj.radius, 
                                          cullDataBuffer.cullData.viewProjMatrix);
            
            // Set instance count to 0 if culled
            if (!visible) {
                cmd.instanceCount = 0;
            }
            
            commandBuffer.commands[idx] = cmd;
        }
    )";
}

} // namespace vge
