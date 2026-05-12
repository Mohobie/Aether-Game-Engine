#include "debug/debug_renderer.h"
#include "rendering/shader.h"
#include "rendering/camera.h"
#include "ai/ai_system.h"
#include "physics/collider.h"
#include "physics/rigidbody.h"
#include "rendering/frustum.h"
#include "core/logger.h"
// Use system OpenGL headers (GL/gl.h + GL/glext.h for function pointers)
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <cmath>

// OpenGL function pointers for VAO/VBO (not in GL/gl.h on some systems)
static PFNGLGENVERTEXARRAYSPROC glGenVertexArraysPtr = nullptr;
static PFNGLBINDVERTEXARRAYPROC glBindVertexArrayPtr = nullptr;
static PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArraysPtr = nullptr;
static PFNGLGENBUFFERSPROC glGenBuffersPtr = nullptr;
static PFNGLBINDBUFFERPROC glBindBufferPtr = nullptr;
static PFNGLBUFFERDATAPROC glBufferDataPtr = nullptr;
static PFNGLDELETEBUFFERSPROC glDeleteBuffersPtr = nullptr;
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArrayPtr = nullptr;
static PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointerPtr = nullptr;

typedef void (APIENTRYP PFNGLDRAWARRAYSPROC_TYPE)(GLenum mode, GLint first, GLsizei count);
static PFNGLDRAWARRAYSPROC_TYPE glDrawArraysPtr = nullptr;

#define glGenVertexArrays glGenVertexArraysPtr
#define glBindVertexArray glBindVertexArrayPtr
#define glDeleteVertexArrays glDeleteVertexArraysPtr
#define glGenBuffers glGenBuffersPtr
#define glBindBuffer glBindBufferPtr
#define glBufferData glBufferDataPtr
#define glDeleteBuffers glDeleteBuffersPtr
#define glEnableVertexAttribArray glEnableVertexAttribArrayPtr
#define glVertexAttribPointer glVertexAttribPointerPtr
#define glDrawArrays glDrawArraysPtr

namespace vge {

// ============================================
// Debug Renderer Implementation
// ============================================

DebugRenderer::DebugRenderer()
    : lineVAO(0), lineVBO(0), boxVAO(0), boxVBO(0), sphereVAO(0), sphereVBO(0)
    , initialized(false), debugShader(nullptr) {}

DebugRenderer::~DebugRenderer() {
    Shutdown();
}

bool DebugRenderer::Initialize() {
    if (initialized) return true;
    
    // Create simple shader for debug rendering
    debugShader = new Shader();
    
    // Simple vertex shader
    const char* vertexShader = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        uniform mat4 mvp;
        void main() {
            gl_Position = mvp * vec4(aPos, 1.0);
        }
    )";
    
    // Simple fragment shader
    const char* fragmentShader = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 color;
        void main() {
            FragColor = vec4(color, 1.0);
        }
    )";
    
    // Load OpenGL function pointers
    glGenVertexArraysPtr = (PFNGLGENVERTEXARRAYSPROC)glXGetProcAddress((const GLubyte*)"glGenVertexArrays");
    glBindVertexArrayPtr = (PFNGLBINDVERTEXARRAYPROC)glXGetProcAddress((const GLubyte*)"glBindVertexArray");
    glDeleteVertexArraysPtr = (PFNGLDELETEVERTEXARRAYSPROC)glXGetProcAddress((const GLubyte*)"glDeleteVertexArrays");
    glGenBuffersPtr = (PFNGLGENBUFFERSPROC)glXGetProcAddress((const GLubyte*)"glGenBuffers");
    glBindBufferPtr = (PFNGLBINDBUFFERPROC)glXGetProcAddress((const GLubyte*)"glBindBuffer");
    glBufferDataPtr = (PFNGLBUFFERDATAPROC)glXGetProcAddress((const GLubyte*)"glBufferData");
    glDeleteBuffersPtr = (PFNGLDELETEBUFFERSPROC)glXGetProcAddress((const GLubyte*)"glDeleteBuffers");
    glEnableVertexAttribArrayPtr = (PFNGLENABLEVERTEXATTRIBARRAYPROC)glXGetProcAddress((const GLubyte*)"glEnableVertexAttribArray");
    glVertexAttribPointerPtr = (PFNGLVERTEXATTRIBPOINTERPROC)glXGetProcAddress((const GLubyte*)"glVertexAttribPointer");
    glDrawArraysPtr = (PFNGLDRAWARRAYSPROC_TYPE)glXGetProcAddress((const GLubyte*)"glDrawArrays");
    
    if (!glGenVertexArraysPtr || !glBindVertexArrayPtr || !glGenBuffersPtr) {
        Logger::Error("[DebugRenderer] Failed to load OpenGL function pointers");
        delete debugShader;
        debugShader = nullptr;
        return false;
    }
    
    if (!debugShader->LoadFromSource(vertexShader, fragmentShader)) {
        Logger::Error("[DebugRenderer] Failed to load debug shader");
        delete debugShader;
        debugShader = nullptr;
        return false;
    }
    
    InitializeBuffers();
    
    initialized = true;
    Logger::Info("[DebugRenderer] Initialized");
    return true;
}

void DebugRenderer::Shutdown() {
    if (!initialized) return;
    
    CleanupBuffers();
    
    if (debugShader) {
        delete debugShader;
        debugShader = nullptr;
    }
    
    initialized = false;
}

void DebugRenderer::InitializeBuffers() {
    // Line buffer
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);
    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Box buffer (for wireframe box)
    glGenVertexArrays(1, &boxVAO);
    glGenBuffers(1, &boxVBO);
    glBindVertexArray(boxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, boxVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Sphere buffer
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
}

void DebugRenderer::CleanupBuffers() {
    if (lineVAO) glDeleteVertexArrays(1, &lineVAO);
    if (lineVBO) glDeleteBuffers(1, &lineVBO);
    if (boxVAO) glDeleteVertexArrays(1, &boxVAO);
    if (boxVBO) glDeleteBuffers(1, &boxVBO);
    if (sphereVAO) glDeleteVertexArrays(1, &sphereVAO);
    if (sphereVBO) glDeleteBuffers(1, &sphereVBO);
    
    lineVAO = lineVBO = boxVAO = boxVBO = sphereVAO = sphereVBO = 0;
}

// ============================================
// Immediate Mode Drawing
// ============================================

void DebugRenderer::DrawLine(const Vec3& start, const Vec3& end, const Vec3& color) {
    DebugRenderCommand cmd;
    cmd.type = DebugRenderCommand::Type::Line;
    cmd.start = start;
    cmd.end = end;
    cmd.color = color;
    cmd.duration = 0.0f;
    commands.push_back(cmd);
}

void DebugRenderer::DrawBox(const Vec3& min, const Vec3& max, const Vec3& color) {
    DebugRenderCommand cmd;
    cmd.type = DebugRenderCommand::Type::Box;
    cmd.start = min;
    cmd.end = max;
    cmd.color = color;
    cmd.duration = 0.0f;
    commands.push_back(cmd);
}

void DebugRenderer::DrawSphere(const Vec3& center, float radius, const Vec3& color) {
    DebugRenderCommand cmd;
    cmd.type = DebugRenderCommand::Type::Sphere;
    cmd.start = center;
    cmd.radius = radius;
    cmd.color = color;
    cmd.duration = 0.0f;
    commands.push_back(cmd);
}

void DebugRenderer::DrawCross(const Vec3& position, float size, const Vec3& color) {
    DebugRenderCommand cmd;
    cmd.type = DebugRenderCommand::Type::Cross;
    cmd.start = position;
    cmd.size = size;
    cmd.color = color;
    cmd.duration = 0.0f;
    commands.push_back(cmd);
}

void DebugRenderer::DrawCircle(const Vec3& center, float radius, const Vec3& normal, const Vec3& color) {
    DebugRenderCommand cmd;
    cmd.type = DebugRenderCommand::Type::Circle;
    cmd.start = center;
    cmd.end = normal;
    cmd.radius = radius;
    cmd.color = color;
    cmd.duration = 0.0f;
    commands.push_back(cmd);
}

void DebugRenderer::DrawArrow(const Vec3& start, const Vec3& end, const Vec3& color, float headSize) {
    DebugRenderCommand cmd;
    cmd.type = DebugRenderCommand::Type::Arrow;
    cmd.start = start;
    cmd.end = end;
    cmd.color = color;
    cmd.size = headSize;
    cmd.duration = 0.0f;
    commands.push_back(cmd);
}

void DebugRenderer::DrawText(const Vec3& position, const std::string& text, const Vec3& color) {
    DebugRenderCommand cmd;
    cmd.type = DebugRenderCommand::Type::Text;
    cmd.start = position;
    cmd.text = text;
    cmd.color = color;
    cmd.duration = 0.0f;
    commands.push_back(cmd);
}

void DebugRenderer::DrawGrid(const Vec3& center, float size, float step, const Vec3& color) {
    DebugRenderCommand cmd;
    cmd.type = DebugRenderCommand::Type::Grid;
    cmd.start = center;
    cmd.size = size;
    cmd.radius = step;
    cmd.color = color;
    cmd.duration = 0.0f;
    commands.push_back(cmd);
}

// ============================================
// Persistent Drawing
// ============================================

void DebugRenderer::DrawLinePersistent(const Vec3& start, const Vec3& end, const Vec3& color, float duration) {
    DebugRenderCommand cmd;
    cmd.type = DebugRenderCommand::Type::Line;
    cmd.start = start;
    cmd.end = end;
    cmd.color = color;
    cmd.duration = duration;
    cmd.timeRemaining = duration;
    persistentCommands.push_back(cmd);
}

void DebugRenderer::DrawBoxPersistent(const Vec3& min, const Vec3& max, const Vec3& color, float duration) {
    DebugRenderCommand cmd;
    cmd.type = DebugRenderCommand::Type::Box;
    cmd.start = min;
    cmd.end = max;
    cmd.color = color;
    cmd.duration = duration;
    cmd.timeRemaining = duration;
    persistentCommands.push_back(cmd);
}

void DebugRenderer::DrawSpherePersistent(const Vec3& center, float radius, const Vec3& color, float duration) {
    DebugRenderCommand cmd;
    cmd.type = DebugRenderCommand::Type::Sphere;
    cmd.start = center;
    cmd.radius = radius;
    cmd.color = color;
    cmd.duration = duration;
    cmd.timeRemaining = duration;
    persistentCommands.push_back(cmd);
}

void DebugRenderer::DrawTextPersistent(const Vec3& position, const std::string& text, const Vec3& color, float duration) {
    DebugRenderCommand cmd;
    cmd.type = DebugRenderCommand::Type::Text;
    cmd.start = position;
    cmd.text = text;
    cmd.color = color;
    cmd.duration = duration;
    cmd.timeRemaining = duration;
    persistentCommands.push_back(cmd);
}

// ============================================
// Specialized Debug Visualizations
// ============================================

void DebugRenderer::DrawNavMesh(const NavigationMesh& navMesh, const Vec3& color) {
    int nodeCount = navMesh.GetNodeCount();
    
    for (int i = 0; i < nodeCount; ++i) {
        // GetNode is non-const, so cast away const
        NavNode* node = const_cast<NavigationMesh&>(navMesh).GetNode(i);
        if (!node) continue;
        
        // Draw node position
        if (node->walkable) {
            DrawCross(node->position, 0.2f, color);
        } else {
            DrawCross(node->position, 0.2f, Vec3(1, 0, 0));
        }
        
        // Draw connections
        for (int neighborIdx : node->neighbors) {
            NavNode* neighbor = const_cast<NavigationMesh&>(navMesh).GetNode(neighborIdx);
            if (neighbor) {
                DrawLine(node->position, neighbor->position, Vec3(0, 0.5f, 0));
            }
        }
    }
}

void DebugRenderer::DrawPath(const std::vector<Vec3>& path, const Vec3& color) {
    if (path.size() < 2) return;
    
    // Draw path lines
    for (size_t i = 0; i < path.size() - 1; ++i) {
        DrawLine(path[i], path[i + 1], color);
    }
    
    // Draw waypoints
    for (size_t i = 0; i < path.size(); ++i) {
        DrawSphere(path[i], 0.1f, Vec3(1, 1, 0));
    }
    
    // Draw start and end markers
    if (!path.empty()) {
        DrawCross(path.front(), 0.3f, Vec3(0, 1, 0));
        DrawCross(path.back(), 0.3f, Vec3(1, 0, 0));
    }
}

void DebugRenderer::DrawCollider(const Collider& collider, const Vec3& color) {
    AABB bounds = collider.getBounds();
    DrawBox(bounds.min, bounds.max, color);
    
    // Draw center point
    Vec3 center = (bounds.min + bounds.max) * 0.5f;
    DrawCross(center, 0.1f, color);
}

void DebugRenderer::DrawRigidBody(const RigidBody& body, const Vec3& color) {
    AABB bounds = body.GetBounds();
    DrawBox(bounds.min, bounds.max, color);
    
    // Draw velocity arrow
    Vec3 center = (bounds.min + bounds.max) * 0.5f;
    Vec3 velocity = body.GetVelocity();
    if (velocity.length() > 0.01f) {
        DrawArrow(center, center + velocity, Vec3(1, 0.5f, 0));
    }
}

void DebugRenderer::DrawFrustum(const Frustum& frustum, const Vec3& color) {
    // Draw frustum planes intersection
    // Simplified: draw frustum corners
    // Would need frustum corner extraction
    
    // Draw camera position
    // frustum doesn't store position directly, would need camera reference
}

void DebugRenderer::DrawChunkBorder(int chunkX, int chunkY, int chunkZ, const Vec3& color) {
    float minX = chunkX * 16.0f;
    float minY = chunkY * 16.0f;
    float minZ = chunkZ * 16.0f;
    float maxX = minX + 16.0f;
    float maxY = minY + 16.0f;
    float maxZ = minZ + 16.0f;
    
    DrawBox(Vec3(minX, minY, minZ), Vec3(maxX, maxY, maxZ), color);
}

void DebugRenderer::DrawEntityBounds(const Vec3& min, const Vec3& max, const Vec3& color) {
    DrawBox(min, max, color);
}

void DebugRenderer::DrawAIState(const Vec3& position, const std::string& state, const Vec3& color) {
    DrawText(position + Vec3(0, 1.5f, 0), state, color);
    DrawCross(position, 0.2f, color);
}

void DebugRenderer::DrawRaycast(const Vec3& origin, const Vec3& direction, float distance, bool hit, const Vec3& hitPoint) {
    Vec3 end = origin + direction * distance;
    
    if (hit) {
        // Draw ray to hit point (green)
        DrawLine(origin, hitPoint, Vec3(0, 1, 0));
        // Draw hit marker
        DrawSphere(hitPoint, 0.1f, Vec3(0, 1, 0));
        // Draw remaining ray (red)
        DrawLine(hitPoint, end, Vec3(1, 0, 0));
    } else {
        // Draw full ray (yellow)
        DrawLine(origin, end, Vec3(1, 1, 0));
    }
    
    // Draw origin marker
    DrawCross(origin, 0.1f, Vec3(1, 1, 1));
}

void DebugRenderer::DrawLightVolume(const Vec3& position, float range, const Vec3& color) {
    DrawSphere(position, range, color);
    DrawCross(position, 0.2f, color);
}

void DebugRenderer::DrawSelectionBox(const Vec3& min, const Vec3& max, const Vec3& color) {
    DrawBox(min, max, color);
}

void DebugRenderer::DrawGizmoTranslate(const Vec3& position, float size) {
    // X axis (red)
    DrawArrow(position, position + Vec3(size, 0, 0), Vec3(1, 0, 0), size * 0.1f);
    // Y axis (green)
    DrawArrow(position, position + Vec3(0, size, 0), Vec3(0, 1, 0), size * 0.1f);
    // Z axis (blue)
    DrawArrow(position, position + Vec3(0, 0, size), Vec3(0, 0, 1), size * 0.1f);
}

void DebugRenderer::DrawGizmoRotate(const Vec3& position, float size) {
    // X axis circle (red)
    DrawCircle(position, size, Vec3(1, 0, 0), Vec3(1, 0, 0));
    // Y axis circle (green)
    DrawCircle(position, size, Vec3(0, 1, 0), Vec3(0, 1, 0));
    // Z axis circle (blue)
    DrawCircle(position, size, Vec3(0, 0, 1), Vec3(0, 0, 1));
}

void DebugRenderer::DrawGizmoScale(const Vec3& position, float size) {
    // X axis (red)
    DrawLine(position, position + Vec3(size, 0, 0), Vec3(1, 0, 0));
    DrawBox(position + Vec3(size - 0.05f, -0.05f, -0.05f), 
            position + Vec3(size + 0.05f, 0.05f, 0.05f), Vec3(1, 0, 0));
    
    // Y axis (green)
    DrawLine(position, position + Vec3(0, size, 0), Vec3(0, 1, 0));
    DrawBox(position + Vec3(-0.05f, size - 0.05f, -0.05f), 
            position + Vec3(0.05f, size + 0.05f, 0.05f), Vec3(0, 1, 0));
    
    // Z axis (blue)
    DrawLine(position, position + Vec3(0, 0, size), Vec3(0, 0, 1));
    DrawBox(position + Vec3(-0.05f, -0.05f, size - 0.05f), 
            position + Vec3(0.05f, 0.05f, size + 0.05f), Vec3(0, 0, 1));
}

// ============================================
// Rendering
// ============================================

void DebugRenderer::Render(const Camera& camera) {
    if (!initialized || !debugShader) return;
    
    // Save OpenGL state
    GLboolean depthTest = glIsEnabled(GL_DEPTH_TEST);
    GLboolean blend = glIsEnabled(GL_BLEND);
    
    // Setup for debug rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(2.0f);
    
    // Use debug shader
    debugShader->Bind();
    
    // Set view-projection matrix
    Mat4 proj = camera.GetProjectionMatrix();
    Mat4 view = camera.GetViewMatrix();
    Mat4 viewProj = proj.Multiply(view);
    debugShader->SetMat4("mvp", viewProj.data);
    
    // Render all immediate commands
    for (const auto& cmd : commands) {
        switch (cmd.type) {
            case DebugRenderCommand::Type::Line:
                RenderLine(cmd, camera);
                break;
            case DebugRenderCommand::Type::Box:
                RenderBox(cmd, camera);
                break;
            case DebugRenderCommand::Type::Sphere:
                RenderSphere(cmd, camera);
                break;
            case DebugRenderCommand::Type::Cross:
                RenderCross(cmd, camera);
                break;
            case DebugRenderCommand::Type::Circle:
                RenderCircle(cmd, camera);
                break;
            case DebugRenderCommand::Type::Arrow:
                RenderArrow(cmd, camera);
                break;
            case DebugRenderCommand::Type::Text:
                RenderText(cmd, camera);
                break;
            case DebugRenderCommand::Type::Grid:
                RenderGrid(cmd, camera);
                break;
        }
    }
    
    // Render persistent commands
    for (const auto& cmd : persistentCommands) {
        switch (cmd.type) {
            case DebugRenderCommand::Type::Line:
                RenderLine(cmd, camera);
                break;
            case DebugRenderCommand::Type::Box:
                RenderBox(cmd, camera);
                break;
            case DebugRenderCommand::Type::Sphere:
                RenderSphere(cmd, camera);
                break;
            case DebugRenderCommand::Type::Cross:
                RenderCross(cmd, camera);
                break;
            case DebugRenderCommand::Type::Circle:
                RenderCircle(cmd, camera);
                break;
            case DebugRenderCommand::Type::Arrow:
                RenderArrow(cmd, camera);
                break;
            case DebugRenderCommand::Type::Text:
                RenderText(cmd, camera);
                break;
            case DebugRenderCommand::Type::Grid:
                RenderGrid(cmd, camera);
                break;
        }
    }
    
    // Restore OpenGL state
    if (!depthTest) glDisable(GL_DEPTH_TEST); else glEnable(GL_DEPTH_TEST);
    if (!blend) glDisable(GL_BLEND); else glEnable(GL_BLEND);
    glLineWidth(1.0f);
}

void DebugRenderer::RenderLine(const DebugRenderCommand& cmd, const Camera& camera) {
    float vertices[] = {
        cmd.start.x, cmd.start.y, cmd.start.z,
        cmd.end.x, cmd.end.y, cmd.end.z
    };
    
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    
    debugShader->SetVec3("color", cmd.color);
    
    glBindVertexArray(lineVAO);
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);
}

void DebugRenderer::RenderBox(const DebugRenderCommand& cmd, const Camera& camera) {
    Vec3 min = cmd.start;
    Vec3 max = cmd.end;
    
    float vertices[] = {
        min.x, min.y, min.z,  max.x, min.y, min.z,
        max.x, min.y, min.z,  max.x, max.y, min.z,
        max.x, max.y, min.z,  min.x, max.y, min.z,
        min.x, max.y, min.z,  min.x, min.y, min.z,
        min.x, min.y, max.z,  max.x, min.y, max.z,
        max.x, min.y, max.z,  max.x, max.y, max.z,
        max.x, max.y, max.z,  min.x, max.y, max.z,
        min.x, max.y, max.z,  min.x, min.y, max.z,
        min.x, min.y, min.z,  min.x, min.y, max.z,
        max.x, min.y, min.z,  max.x, min.y, max.z,
        max.x, max.y, min.z,  max.x, max.y, max.z,
        min.x, max.y, min.z,  min.x, max.y, max.z
    };
    
    glBindBuffer(GL_ARRAY_BUFFER, boxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    
    debugShader->SetVec3("color", cmd.color);
    
    glBindVertexArray(boxVAO);
    glDrawArrays(GL_LINES, 0, 24);
    glBindVertexArray(0);
}

void DebugRenderer::RenderSphere(const DebugRenderCommand& cmd, const Camera& camera) {
    // Simplified: draw wireframe sphere using circles
    Vec3 center = cmd.start;
    float radius = cmd.radius;
    
    // XY circle
    DrawCircle(center, radius, Vec3(0, 0, 1), cmd.color);
    // XZ circle
    DrawCircle(center, radius, Vec3(0, 1, 0), cmd.color);
    // YZ circle
    DrawCircle(center, radius, Vec3(1, 0, 0), cmd.color);
}

void DebugRenderer::RenderCross(const DebugRenderCommand& cmd, const Camera& camera) {
    Vec3 pos = cmd.start;
    float s = cmd.size;
    
    // X line
    DrawLine(pos - Vec3(s, 0, 0), pos + Vec3(s, 0, 0), cmd.color);
    // Y line
    DrawLine(pos - Vec3(0, s, 0), pos + Vec3(0, s, 0), cmd.color);
    // Z line
    DrawLine(pos - Vec3(0, 0, s), pos + Vec3(0, 0, s), cmd.color);
}

void DebugRenderer::RenderCircle(const DebugRenderCommand& cmd, const Camera& camera) {
    Vec3 center = cmd.start;
    float radius = cmd.radius;
    Vec3 normal = cmd.end;
    
    // Generate circle points
    const int segments = 32;
    Vec3 prevPoint;
    bool first = true;
    
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * 3.14159f * i / segments;
        
        // Create basis vectors perpendicular to normal
        Vec3 tangent, bitangent;
        if (std::abs(normal.y) < 0.99f) {
            tangent = normal.cross(Vec3(0, 1, 0)).normalize();
        } else {
            tangent = normal.cross(Vec3(1, 0, 0)).normalize();
        }
        bitangent = normal.cross(tangent);
        
        Vec3 point = center + tangent * std::cos(angle) * radius + bitangent * std::sin(angle) * radius;
        
        if (!first) {
            DrawLine(prevPoint, point, cmd.color);
        }
        
        prevPoint = point;
        first = false;
    }
}

void DebugRenderer::RenderArrow(const DebugRenderCommand& cmd, const Camera& camera) {
    Vec3 start = cmd.start;
    Vec3 end = cmd.end;
    float headSize = cmd.size;
    
    // Draw line
    DrawLine(start, end, cmd.color);
    
    // Draw arrow head
    Vec3 dir = (end - start).normalize();
    Vec3 up(0, 1, 0);
    if (std::abs(dir.dot(up)) > 0.99f) {
        up = Vec3(1, 0, 0);
    }
    Vec3 right = dir.cross(up).normalize();
    up = right.cross(dir);
    
    Vec3 headBase = end - dir * headSize * 3;
    
    DrawLine(end, headBase + right * headSize, cmd.color);
    DrawLine(end, headBase - right * headSize, cmd.color);
    DrawLine(end, headBase + up * headSize, cmd.color);
    DrawLine(end, headBase - up * headSize, cmd.color);
}

void DebugRenderer::RenderText(const DebugRenderCommand& cmd, const Camera& camera) {
    // Text rendering requires a font atlas and text rendering system
    // For now, draw a cross at the position to indicate text location
    DrawCross(cmd.start, 0.1f, cmd.color);
}

void DebugRenderer::RenderGrid(const DebugRenderCommand& cmd, const Camera& camera) {
    Vec3 center = cmd.start;
    float size = cmd.size;
    float step = cmd.radius;
    
    int lines = static_cast<int>(size / step);
    
    for (int i = -lines; i <= lines; ++i) {
        float offset = i * step;
        
        // X lines
        DrawLine(Vec3(center.x - size, center.y, center.z + offset),
                 Vec3(center.x + size, center.y, center.z + offset), cmd.color);
        
        // Z lines
        DrawLine(Vec3(center.x + offset, center.y, center.z - size),
                 Vec3(center.x + offset, center.y, center.z + size), cmd.color);
    }
}

// ============================================
// Update
// ============================================

void DebugRenderer::Update(float deltaTime) {
    // Update persistent commands
    for (auto it = persistentCommands.begin(); it != persistentCommands.end();) {
        it->timeRemaining -= deltaTime;
        if (it->timeRemaining <= 0.0f) {
            it = persistentCommands.erase(it);
        } else {
            ++it;
        }
    }
}

void DebugRenderer::Clear() {
    commands.clear();
}

void DebugRenderer::ClearAll() {
    commands.clear();
    persistentCommands.clear();
}

// ============================================
// Global Accessor
// ============================================

DebugRenderer& GetDebugRenderer() {
    static DebugRenderer instance;
    return instance;
}

} // namespace vge
