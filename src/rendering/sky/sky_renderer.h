#pragma once
#include "math/vec3.h"
#include "rendering/sky/day_night_cycle.h"
#include "math/vec3.h"
#include <vector>

namespace vge {

// ============================================
// Sky Renderer
// Renders sky dome, sun, moon, stars
// ============================================
class SkyRenderer {
private:
    // Cached star positions (screen-space)
    struct ScreenStar {
        int x, y;
        float brightness;
        uint8_t colorIndex;
    };
    
    std::vector<ScreenStar> screenStars;
    bool starsDirty;
    
    // Sun/Moon rendering helpers
    void DrawSunASCII(int centerX, int centerY, float intensity, const Vec3& color);
    void DrawMoonASCII(int centerX, int centerY, float intensity, MoonPhase phase);
    void DrawStarsASCII(int screenW, int screenH, float visibility, const DayNightCycle& cycle);
    void DrawSkyGradientASCII(int screenW, int screenH, const SkyColorSet& sky);
    
    // Framebuffer rendering helpers
    void DrawSunFB(int centerX, int centerY, float intensity, const Vec3& color, 
                   class FramebufferRenderer* fb);
    void DrawMoonFB(int centerX, int centerY, float intensity, MoonPhase phase,
                    class FramebufferRenderer* fb);
    void DrawStarsFB(int screenW, int screenH, float visibility, const DayNightCycle& cycle,
                     class FramebufferRenderer* fb);
    void DrawSkyGradientFB(int screenW, int screenH, const SkyColorSet& sky,
                           class FramebufferRenderer* fb);
    
    // Color conversion
    uint32_t Vec3ToColor(const Vec3& color) const;
    char BrightnessToChar(float brightness) const;
    
public:
    SkyRenderer();
    
    // Render sky to ASCII/console
    void RenderSkyASCII(const DayNightCycle& cycle, int screenW, int screenH);
    
    // Render sky to framebuffer (Linux direct pixel)
    void RenderSkyFB(const DayNightCycle& cycle, class FramebufferRenderer* fb);
    
    // Project celestial body direction to screen coordinates
    static Vec3 ProjectDirection(const Vec3& direction, const class Camera& camera, 
                                  int screenW, int screenH);
};

} // namespace vge
