#pragma once
#include "math/vec3.h"
#include "rendering/camera.h"
#include <vector>
#include <string>

namespace vge {

// ============================================
// Debug Render Command
// ============================================
struct DebugRenderCommand {
    enum class Type {
        Line,
        Box,
        Sphere,
        Cross,
        Circle,
        Arrow,
        Text,
        Grid
    };
    
    Type type;
    Vec3 start;
    Vec3 end;
    Vec3 color;
    float radius;
    float size;
    std::string text;
    float duration;
    float timeRemaining;
    
    DebugRenderCommand() 
        : type(Type::Line), radius(1.0f), size(1.0f), duration(0.0f), timeRemaining(0.0f) {}
};

// ============================================
// Debug Renderer - Immediate Mode Debug Drawing
// ============================================
class DebugRenderer {
private:
    std::vector<DebugRenderCommand> commands;
    std::vector<DebugRenderCommand> persistentCommands;
    
    // OpenGL resources
    uint32_t lineVAO;
    uint32_t lineVBO;
    uint32_t boxVAO;
    uint32_t boxVBO;
    uint32_t sphereVAO;
    uint32_t sphereVBO;
    
    bool initialized;
    
    // Shader for debug rendering
    class Shader* debugShader;
    
    void InitializeBuffers();
    void CleanupBuffers();
    
    void RenderLine(const DebugRenderCommand& cmd, const Camera& camera);
    void RenderBox(const DebugRenderCommand& cmd, const Camera& camera);
    void RenderSphere(const DebugRenderCommand& cmd, const Camera& camera);
    void RenderCross(const DebugRenderCommand& cmd, const Camera& camera);
    void RenderCircle(const DebugRenderCommand& cmd, const Camera& camera);
    void RenderArrow(const DebugRenderCommand& cmd, const Camera& camera);
    void RenderText(const DebugRenderCommand& cmd, const Camera& camera);
    void RenderGrid(const DebugRenderCommand& cmd, const Camera& camera);
    
public:
    DebugRenderer();
    ~DebugRenderer();
    
    bool Initialize();
    void Shutdown();
    
    // Immediate mode drawing (one frame)
    void DrawLine(const Vec3& start, const Vec3& end, const Vec3& color);
    void DrawBox(const Vec3& min, const Vec3& max, const Vec3& color);
    void DrawSphere(const Vec3& center, float radius, const Vec3& color);
    void DrawCross(const Vec3& position, float size, const Vec3& color);
    void DrawCircle(const Vec3& center, float radius, const Vec3& normal, const Vec3& color);
    void DrawArrow(const Vec3& start, const Vec3& end, const Vec3& color, float headSize = 0.1f);
    void DrawText(const Vec3& position, const std::string& text, const Vec3& color);
    void DrawGrid(const Vec3& center, float size, float step, const Vec3& color);
    
    // Persistent drawing (multiple frames)
    void DrawLinePersistent(const Vec3& start, const Vec3& end, const Vec3& color, float duration);
    void DrawBoxPersistent(const Vec3& min, const Vec3& max, const Vec3& color, float duration);
    void DrawSpherePersistent(const Vec3& center, float radius, const Vec3& color, float duration);
    void DrawTextPersistent(const Vec3& position, const std::string& text, const Vec3& color, float duration);
    
    // Specialized debug visualizations
    void DrawNavMesh(const class NavigationMesh& navMesh, const Vec3& color = Vec3(0, 1, 0));
    void DrawPath(const std::vector<Vec3>& path, const Vec3& color = Vec3(1, 1, 0));
    void DrawCollider(const class Collider& collider, const Vec3& color = Vec3(0, 1, 1));
    void DrawRigidBody(const class RigidBody& body, const Vec3& color = Vec3(1, 0.5f, 0));
    void DrawFrustum(const class Frustum& frustum, const Vec3& color = Vec3(1, 0, 1));
    void DrawChunkBorder(int chunkX, int chunkY, int chunkZ, const Vec3& color = Vec3(0.5f, 0.5f, 0.5f));
    void DrawEntityBounds(const Vec3& min, const Vec3& max, const Vec3& color = Vec3(0, 0.8f, 1));
    void DrawAIState(const Vec3& position, const std::string& state, const Vec3& color = Vec3(1, 1, 1));
    void DrawRaycast(const Vec3& origin, const Vec3& direction, float distance, bool hit, const Vec3& hitPoint);
    void DrawLightVolume(const Vec3& position, float range, const Vec3& color = Vec3(1, 1, 0));
    void DrawSelectionBox(const Vec3& min, const Vec3& max, const Vec3& color = Vec3(1, 0.8f, 0));
    void DrawGizmoTranslate(const Vec3& position, float size = 1.0f);
    void DrawGizmoRotate(const Vec3& position, float size = 1.0f);
    void DrawGizmoScale(const Vec3& position, float size = 1.0f);
    
    // Render all debug commands
    void Render(const Camera& camera);
    void Update(float deltaTime);
    
    // Clear immediate commands
    void Clear();
    
    // Clear all commands including persistent
    void ClearAll();
    
    bool IsInitialized() const { return initialized; }
};

// Global accessor for easy debug drawing
DebugRenderer& GetDebugRenderer();

} // namespace vge
