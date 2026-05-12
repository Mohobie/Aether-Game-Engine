#include "editor/scene_viewport.h"
#include "debug/debug_renderer.h"
#include "core/logger.h"
#include <GL/gl.h>
#include <cmath>

namespace vge {

// ============================================
// Scene Viewport Implementation
// ============================================

SceneViewport::SceneViewport(ViewportType type)
    : type(type), x(0), y(0), width(800), height(600)
    , orthoSize(10.0f), perspectiveFOV(60.0f), nearPlane(0.1f), farPlane(1000.0f)
    , showGrid(true), showGizmos(true), showSelection(true), showStats(false)
    , active(false), maximized(false)
    , targetPosition(0, 0, 0), orbitDistance(10.0f)
    , orbitYaw(45.0f), orbitPitch(30.0f)
    , isOrbiting(false), isPanning(false), isZooming(false) {}

SceneViewport::~SceneViewport() {}

void SceneViewport::Initialize(int x, int y, int width, int height) {
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    
    UpdateCamera();
    
    Logger::Info("[SceneViewport] Initialized " + GetLabel() + " viewport");
}

void SceneViewport::Update(float deltaTime) {
    (void)deltaTime;
    
    // Update orbit camera if needed
    if (type == ViewportType::Perspective && isOrbiting) {
        Vec3 offset;
        float yawRad = orbitYaw * 3.14159f / 180.0f;
        float pitchRad = orbitPitch * 3.14159f / 180.0f;
        
        offset.x = orbitDistance * std::cos(pitchRad) * std::cos(yawRad);
        offset.y = orbitDistance * std::sin(pitchRad);
        offset.z = orbitDistance * std::cos(pitchRad) * std::sin(yawRad);
        
        camera.SetPosition(targetPosition + offset);
        camera.SetRotation(orbitYaw, -orbitPitch, 0);
    }
}

void SceneViewport::UpdateCamera() {
    float aspect = GetAspectRatio();
    
    switch (type) {
        case ViewportType::Perspective:
            camera.SetRotation(orbitYaw, -orbitPitch, 0);
            break;
        case ViewportType::Top:
            camera.SetPosition(Vec3(targetPosition.x, targetPosition.y + orthoSize, targetPosition.z));
            camera.SetRotation(0, -90, 0);
            break;
        case ViewportType::Front:
            camera.SetPosition(Vec3(targetPosition.x, targetPosition.y, targetPosition.z + orthoSize));
            camera.SetRotation(180, 0, 0);
            break;
        case ViewportType::Side:
            camera.SetPosition(Vec3(targetPosition.x + orthoSize, targetPosition.y, targetPosition.z));
            camera.SetRotation(90, 0, 0);
            break;
        case ViewportType::Isometric:
            camera.SetPosition(Vec3(
                targetPosition.x + orthoSize * 0.8f,
                targetPosition.y + orthoSize * 0.8f,
                targetPosition.z + orthoSize * 0.8f
            ));
            camera.SetRotation(45, -35, 0);
            break;
    }
}

void SceneViewport::SetRect(int x, int y, int width, int height) {
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
}

void SceneViewport::SetSize(int width, int height) {
    this->width = width;
    this->height = height;
}

void SceneViewport::SetCameraPosition(const Vec3& position) {
    camera.SetPosition(position);
}

void SceneViewport::SetCameraTarget(const Vec3& target) {
    targetPosition = target;
    UpdateCamera();
}

void SceneViewport::Orbit(float deltaYaw, float deltaPitch) {
    orbitYaw += deltaYaw;
    orbitPitch += deltaPitch;
    
    // Clamp pitch
    if (orbitPitch > 89.0f) orbitPitch = 89.0f;
    if (orbitPitch < -89.0f) orbitPitch = -89.0f;
    
    UpdateCamera();
}

void SceneViewport::Pan(float deltaX, float deltaY) {
    Vec3 right = camera.GetRight();
    Vec3 up = camera.GetUp();
    
    targetPosition = targetPosition + right * deltaX + up * deltaY;
    UpdateCamera();
}

void SceneViewport::Zoom(float delta) {
    if (type == ViewportType::Perspective) {
        orbitDistance -= delta;
        if (orbitDistance < 0.5f) orbitDistance = 0.5f;
        if (orbitDistance > 500.0f) orbitDistance = 500.0f;
    } else {
        orthoSize -= delta;
        if (orthoSize < 1.0f) orthoSize = 1.0f;
        if (orthoSize > 1000.0f) orthoSize = 1000.0f;
    }
    
    UpdateCamera();
}

void SceneViewport::FocusOnPoint(const Vec3& point) {
    targetPosition = point;
    UpdateCamera();
}

void SceneViewport::FocusOnBounds(const Vec3& min, const Vec3& max) {
    targetPosition = (min + max) * 0.5f;
    
    Vec3 size = max - min;
    float maxSize = std::max(size.x, std::max(size.y, size.z));
    
    if (type == ViewportType::Perspective) {
        orbitDistance = maxSize * 1.5f;
    } else {
        orthoSize = maxSize * 0.6f;
    }
    
    UpdateCamera();
}

void SceneViewport::SetType(ViewportType newType) {
    type = newType;
    UpdateCamera();
}

Vec3 SceneViewport::ScreenToWorldRay(float screenX, float screenY) const {
    // Convert screen coordinates to normalized device coordinates
    float ndcX = (screenX - x) / width * 2.0f - 1.0f;
    float ndcY = 1.0f - (screenY - y) / height * 2.0f;
    
    // For perspective camera, ray direction from camera center through screen point
    Vec3 forward = camera.GetForward();
    Vec3 right = camera.GetRight();
    Vec3 up = camera.GetUp();
    
    float fovRad = perspectiveFOV * 3.14159f / 180.0f;
    float tanFov = std::tan(fovRad * 0.5f);
    
    Vec3 ray = forward + right * ndcX * tanFov * GetAspectRatio() + up * ndcY * tanFov;
    return ray.normalize();
}

Vec3 SceneViewport::ScreenToWorldPoint(float screenX, float screenY, float depth) const {
    Vec3 ray = ScreenToWorldRay(screenX, screenY);
    return camera.GetPosition() + ray * depth;
}

Vec2 SceneViewport::WorldToScreen(const Vec3& worldPos) const {
    // Simplified: assumes camera matrices are available
    // Would need proper projection in real implementation
    Vec3 toPoint = worldPos - camera.GetPosition();
    Vec3 forward = camera.GetForward();
    Vec3 right = camera.GetRight();
    Vec3 up = camera.GetUp();
    
    float dist = toPoint.dot(forward);
    if (dist <= 0) return Vec2(-1, -1); // Behind camera
    
    float xOffset = toPoint.dot(right) / dist;
    float yOffset = toPoint.dot(up) / dist;
    
    float screenX = x + width * 0.5f * (1.0f + xOffset);
    float screenY = y + height * 0.5f * (1.0f - yOffset);
    
    return Vec2(screenX, screenY);
}

bool SceneViewport::IsWorldPointVisible(const Vec3& worldPos) const {
    Vec2 screenPos = WorldToScreen(worldPos);
    return screenPos.x >= x && screenPos.x <= x + width &&
           screenPos.y >= y && screenPos.y <= y + height;
}

void SceneViewport::BeginRender() {
    // Set viewport
    glViewport(x, y, width, height);
    
    // Clear viewport
    glScissor(x, y, width, height);
    glEnable(GL_SCISSOR_TEST);
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);
}

void SceneViewport::EndRender() {
    // Render viewport border if active
    if (active) {
        RenderViewportBorder();
    }
}

void SceneViewport::RenderGrid() {
    if (!showGrid) return;
    
    DebugRenderer& debug = GetDebugRenderer();
    
    // Draw grid at origin
    debug.DrawGrid(Vec3(0, 0, 0), 50.0f, 1.0f, Vec3(0.3f, 0.3f, 0.3f));
    
    // Draw axes (using cross for origin)
    debug.DrawCross(Vec3(0, 0, 0), 2.0f, Vec3(1, 1, 1));
}

void SceneViewport::RenderGizmos() {
    if (!showGizmos) return;
    
    // Gizmos are rendered by the editor system
    // This is called after the main scene render
}

void SceneViewport::RenderSelection() {
    if (!showSelection) return;
    
    // Selection is rendered by the editor system
    // This is called after the main scene render
}

void SceneViewport::RenderStats() {
    if (!showStats) return;
    
    // Stats overlay would be rendered here
    // For now, this is handled by ImGui
}

void SceneViewport::RenderViewportBorder() {
    DebugRenderer& debug = GetDebugRenderer();
    
    // Draw border around active viewport
    Vec3 color(1.0f, 0.5f, 0.0f); // Orange border for active viewport
    
    Vec3 min(x, y, 0);
    Vec3 max(x + width, y + height, 0);
    
    // Draw rectangle border
    debug.DrawLine(Vec3(x, y, 0), Vec3(x + width, y, 0), color);
    debug.DrawLine(Vec3(x + width, y, 0), Vec3(x + width, y + height, 0), color);
    debug.DrawLine(Vec3(x + width, y + height, 0), Vec3(x, y + height, 0), color);
    debug.DrawLine(Vec3(x, y + height, 0), Vec3(x, y, 0), color);
}

std::string SceneViewport::GetLabel() const {
    switch (type) {
        case ViewportType::Perspective: return "Perspective";
        case ViewportType::Top: return "Top";
        case ViewportType::Front: return "Front";
        case ViewportType::Side: return "Side";
        case ViewportType::Isometric: return "Isometric";
        default: return "Unknown";
    }
}

// ============================================
// Viewport Manager Implementation
// ============================================

ViewportManager::ViewportManager()
    : activeViewportIndex(0), multiViewportMode(false)
    , screenWidth(1920), screenHeight(1080) {}

ViewportManager::~ViewportManager() {
    Shutdown();
}

void ViewportManager::Initialize(int screenWidth, int screenHeight) {
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
    
    // Create default single viewport
    SetLayoutSingle();
    
    Logger::Info("[ViewportManager] Initialized with screen size " + std::to_string(screenWidth) + "x" + std::to_string(screenHeight));
}

void ViewportManager::Shutdown() {
    ClearViewports();
}

SceneViewport* ViewportManager::CreateViewport(ViewportType type) {
    viewports.emplace_back(type);
    return &viewports.back();
}

void ViewportManager::RemoveViewport(int index) {
    if (index >= 0 && index < (int)viewports.size()) {
        viewports.erase(viewports.begin() + index);
        
        if (activeViewportIndex >= (int)viewports.size()) {
            activeViewportIndex = viewports.size() - 1;
        }
    }
}

void ViewportManager::ClearViewports() {
    viewports.clear();
    activeViewportIndex = 0;
}

void ViewportManager::SetLayoutSingle() {
    ClearViewports();
    
    SceneViewport* vp = CreateViewport(ViewportType::Perspective);
    vp->Initialize(0, 0, screenWidth, screenHeight);
    vp->SetActive(true);
    
    activeViewportIndex = 0;
    multiViewportMode = false;
}

void ViewportManager::SetLayoutQuad() {
    ClearViewports();
    
    int halfW = screenWidth / 2;
    int halfH = screenHeight / 2;
    
    SceneViewport* vp1 = CreateViewport(ViewportType::Perspective);
    vp1->Initialize(0, 0, halfW, halfH);
    
    SceneViewport* vp2 = CreateViewport(ViewportType::Top);
    vp2->Initialize(halfW, 0, halfW, halfH);
    
    SceneViewport* vp3 = CreateViewport(ViewportType::Front);
    vp3->Initialize(0, halfH, halfW, halfH);
    
    SceneViewport* vp4 = CreateViewport(ViewportType::Side);
    vp4->Initialize(halfW, halfH, halfW, halfH);
    
    activeViewportIndex = 0;
    vp1->SetActive(true);
    multiViewportMode = true;
}

void ViewportManager::SetLayoutTriple() {
    ClearViewports();
    
    int thirdW = screenWidth / 3;
    int halfH = screenHeight / 2;
    
    SceneViewport* vp1 = CreateViewport(ViewportType::Perspective);
    vp1->Initialize(0, 0, thirdW * 2, screenHeight);
    
    SceneViewport* vp2 = CreateViewport(ViewportType::Top);
    vp2->Initialize(thirdW * 2, 0, thirdW, halfH);
    
    SceneViewport* vp3 = CreateViewport(ViewportType::Front);
    vp3->Initialize(thirdW * 2, halfH, thirdW, halfH);
    
    activeViewportIndex = 0;
    vp1->SetActive(true);
    multiViewportMode = true;
}

void ViewportManager::SetLayoutSplitHorizontal() {
    ClearViewports();
    
    int halfW = screenWidth / 2;
    
    SceneViewport* vp1 = CreateViewport(ViewportType::Perspective);
    vp1->Initialize(0, 0, halfW, screenHeight);
    
    SceneViewport* vp2 = CreateViewport(ViewportType::Top);
    vp2->Initialize(halfW, 0, halfW, screenHeight);
    
    activeViewportIndex = 0;
    vp1->SetActive(true);
    multiViewportMode = true;
}

void ViewportManager::SetLayoutSplitVertical() {
    ClearViewports();
    
    int halfH = screenHeight / 2;
    
    SceneViewport* vp1 = CreateViewport(ViewportType::Perspective);
    vp1->Initialize(0, 0, screenWidth, halfH);
    
    SceneViewport* vp2 = CreateViewport(ViewportType::Top);
    vp2->Initialize(0, halfH, screenWidth, halfH);
    
    activeViewportIndex = 0;
    vp1->SetActive(true);
    multiViewportMode = true;
}

void ViewportManager::UpdateLayout() {
    // Recalculate viewport positions based on current layout
    if (viewports.size() == 1) {
        viewports[0].SetRect(0, 0, screenWidth, screenHeight);
    } else if (viewports.size() == 4) {
        int halfW = screenWidth / 2;
        int halfH = screenHeight / 2;
        viewports[0].SetRect(0, 0, halfW, halfH);
        viewports[1].SetRect(halfW, 0, halfW, halfH);
        viewports[2].SetRect(0, halfH, halfW, halfH);
        viewports[3].SetRect(halfW, halfH, halfW, halfH);
    }
}

SceneViewport* ViewportManager::GetViewport(int index) {
    if (index >= 0 && index < (int)viewports.size()) {
        return &viewports[index];
    }
    return nullptr;
}

SceneViewport* ViewportManager::GetActiveViewport() {
    if (activeViewportIndex >= 0 && activeViewportIndex < (int)viewports.size()) {
        return &viewports[activeViewportIndex];
    }
    return nullptr;
}

void ViewportManager::SetActiveViewport(int index) {
    if (index >= 0 && index < (int)viewports.size()) {
        // Deactivate current
        if (activeViewportIndex >= 0 && activeViewportIndex < (int)viewports.size()) {
            viewports[activeViewportIndex].SetActive(false);
        }
        
        // Activate new
        activeViewportIndex = index;
        viewports[activeViewportIndex].SetActive(true);
    }
}

void ViewportManager::SetScreenSize(int width, int height) {
    screenWidth = width;
    screenHeight = height;
    UpdateLayout();
}

void ViewportManager::Update(float deltaTime) {
    for (auto& viewport : viewports) {
        viewport.Update(deltaTime);
    }
}

void ViewportManager::Render() {
    for (auto& viewport : viewports) {
        viewport.BeginRender();
        viewport.RenderGrid();
        // Main scene rendering would happen here
        viewport.RenderGizmos();
        viewport.RenderSelection();
        viewport.RenderStats();
        viewport.EndRender();
    }
}

void ViewportManager::FocusAllOnPoint(const Vec3& point) {
    for (auto& viewport : viewports) {
        viewport.FocusOnPoint(point);
    }
}

void ViewportManager::FocusAllOnBounds(const Vec3& min, const Vec3& max) {
    for (auto& viewport : viewports) {
        viewport.FocusOnBounds(min, max);
    }
}

} // namespace vge
