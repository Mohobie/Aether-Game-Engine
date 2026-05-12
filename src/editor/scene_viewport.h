#pragma once
#include "math/vec3.h"
#include "math/vec2.h"
#include "rendering/camera.h"
#include <string>
#include <vector>

namespace vge {

// ============================================
// Viewport Type
// ============================================
enum class ViewportType {
    Perspective,
    Top,
    Front,
    Side,
    Isometric
};

// ============================================
// Scene Viewport
// ============================================
class SceneViewport {
private:
    ViewportType type;
    Camera camera;
    
    // Viewport rectangle (in screen coordinates)
    int x, y;
    int width, height;
    
    // Camera settings
    float orthoSize;
    float perspectiveFOV;
    float nearPlane;
    float farPlane;
    
    // View settings
    bool showGrid;
    bool showGizmos;
    bool showSelection;
    bool showStats;
    bool active;
    bool maximized;
    
    // Camera control
    Vec3 targetPosition;
    float orbitDistance;
    float orbitYaw;
    float orbitPitch;
    bool isOrbiting;
    bool isPanning;
    bool isZooming;
    
public:
    SceneViewport(ViewportType type = ViewportType::Perspective);
    ~SceneViewport();
    
    // Initialize
    void Initialize(int x, int y, int width, int height);
    
    // Update
    void Update(float deltaTime);
    void UpdateCamera();
    
    // Resize
    void SetRect(int x, int y, int width, int height);
    void SetSize(int width, int height);
    
    // Camera control
    void SetCameraPosition(const Vec3& position);
    void SetCameraTarget(const Vec3& target);
    void Orbit(float deltaYaw, float deltaPitch);
    void Pan(float deltaX, float deltaY);
    void Zoom(float delta);
    void FocusOnPoint(const Vec3& point);
    void FocusOnBounds(const Vec3& min, const Vec3& max);
    
    // View settings
    void SetShowGrid(bool show) { showGrid = show; }
    void SetShowGizmos(bool show) { showGizmos = show; }
    void SetShowSelection(bool show) { showSelection = show; }
    void SetShowStats(bool show) { showStats = show; }
    bool IsShowingGrid() const { return showGrid; }
    bool IsShowingGizmos() const { return showGizmos; }
    bool IsShowingSelection() const { return showSelection; }
    bool IsShowingStats() const { return showStats; }
    
    // Active state
    void SetActive(bool active) { this->active = active; }
    bool IsActive() const { return active; }
    
    // Maximize
    void SetMaximized(bool maximized) { this->maximized = maximized; }
    bool IsMaximized() const { return maximized; }
    
    // Getters
    Camera* GetCamera() { return &camera; }
    ViewportType GetType() const { return type; }
    void SetType(ViewportType type);
    
    // Viewport rect
    int GetX() const { return x; }
    int GetY() const { return y; }
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    float GetAspectRatio() const { return width > 0 ? (float)width / (float)height : 1.0f; }
    
    // Screen to world
    Vec3 ScreenToWorldRay(float screenX, float screenY) const;
    Vec3 ScreenToWorldPoint(float screenX, float screenY, float depth) const;
    
    // World to screen
    Vec2 WorldToScreen(const Vec3& worldPos) const;
    bool IsWorldPointVisible(const Vec3& worldPos) const;
    
    // Render
    void BeginRender();
    void EndRender();
    void RenderGrid();
    void RenderGizmos();
    void RenderSelection();
    void RenderStats();
    void RenderViewportBorder();
    
    // Label
    std::string GetLabel() const;
};

// ============================================
// Viewport Manager
// ============================================
class ViewportManager {
private:
    std::vector<SceneViewport> viewports;
    int activeViewportIndex;
    bool multiViewportMode;
    int screenWidth;
    int screenHeight;
    
public:
    ViewportManager();
    ~ViewportManager();
    
    void Initialize(int screenWidth, int screenHeight);
    void Shutdown();
    
    // Viewport management
    SceneViewport* CreateViewport(ViewportType type);
    void RemoveViewport(int index);
    void ClearViewports();
    
    // Layout
    void SetLayoutSingle();
    void SetLayoutQuad();
    void SetLayoutTriple();
    void SetLayoutSplitHorizontal();
    void SetLayoutSplitVertical();
    void UpdateLayout();
    
    // Access
    SceneViewport* GetViewport(int index);
    SceneViewport* GetActiveViewport();
    int GetViewportCount() const { return viewports.size(); }
    
    // Active viewport
    void SetActiveViewport(int index);
    int GetActiveViewportIndex() const { return activeViewportIndex; }
    
    // Multi-viewport mode
    void SetMultiViewportMode(bool enabled) { multiViewportMode = enabled; }
    bool IsMultiViewportMode() const { return multiViewportMode; }
    
    // Screen size
    void SetScreenSize(int width, int height);
    
    // Update all
    void Update(float deltaTime);
    void Render();
    
    // Focus all cameras on point
    void FocusAllOnPoint(const Vec3& point);
    void FocusAllOnBounds(const Vec3& min, const Vec3& max);
};

} // namespace vge
