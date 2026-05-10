#include "rendering/sky/sky_renderer.h"
#include "rendering/sky/day_night_cycle.h"
#include "rendering/camera.h"
#include "rendering/framebuffer_renderer.h"
#include <cmath>
#include <iostream>

namespace vge {

// ============================================
// Constructor
// ============================================
SkyRenderer::SkyRenderer() : starsDirty(true) {}

// ============================================
// Color Conversion
// ============================================
uint32_t SkyRenderer::Vec3ToColor(const Vec3& color) const {
    uint8_t r = static_cast<uint8_t>(std::min(255.0f, color.x * 255.0f));
    uint8_t g = static_cast<uint8_t>(std::min(255.0f, color.y * 255.0f));
    uint8_t b = static_cast<uint8_t>(std::min(255.0f, color.z * 255.0f));
    return RGB(r, g, b);
}

char SkyRenderer::BrightnessToChar(float brightness) const {
    if (brightness > 0.9f) return '*';
    if (brightness > 0.7f) return '+';
    if (brightness > 0.5f) return '.';
    if (brightness > 0.3f) return '\'';
    return '.';
}

// ============================================
// Direction Projection
// ============================================
Vec2 SkyRenderer::ProjectDirection(const Vec3& direction, const Camera& camera, 
                                    int screenW, int screenH) {
    // Get camera forward vector
    Vec3 camForward = camera.GetForward();
    Vec3 camRight = camera.GetRight();
    Vec3 camUp = camera.GetUp();
    
    // Project direction onto camera basis
    float forwardDot = direction.dot(camForward);
    float rightDot = direction.dot(camRight);
    float upDot = direction.dot(camUp);
    
    // Only render if in front of camera
    if (forwardDot < 0.1f) {
        return Vec2(-1, -1); // Behind camera
    }
    
    // Perspective projection
    float fov = 70.0f * 3.14159f / 180.0f;
    float tanHalfFov = std::tan(fov / 2.0f);
    float aspect = (float)screenW / (float)screenH;
    
    float screenX = (rightDot / (forwardDot * tanHalfFov * aspect)) * screenW / 2 + screenW / 2;
    float screenY = (-upDot / (forwardDot * tanHalfFov)) * screenH / 2 + screenH / 2;
    
    return Vec2(screenX, screenY);
}

// ============================================
// ASCII Sky Rendering
// ============================================
void SkyRenderer::DrawSkyGradientASCII(int screenW, int screenH, const SkyColorSet& sky) {
    // For ASCII, we can't do true gradients, but we can set the background
    // This is handled by clearing with appropriate color hints
    // In a real terminal, we'd use ANSI color codes
    
    // For now, just output a simple sky indicator at top
    std::cout << "\n[Sky: ";
    
    // Approximate sky color as a character
    float avgBrightness = (sky.horizonColor.x + sky.horizonColor.y + sky.horizonColor.z) / 3.0f;
    if (avgBrightness > 0.7f) {
        std::cout << "Bright Day";
    } else if (avgBrightness > 0.4f) {
        std::cout << "Day";
    } else if (avgBrightness > 0.2f) {
        std::cout << "Twilight";
    } else {
        std::cout << "Night";
    }
    
    std::cout << "]\n";
}

void SkyRenderer::DrawSunASCII(int centerX, int centerY, float intensity, const Vec3& color) {
    if (centerX < 0 || centerY < 0) return;
    
    // Draw sun as a bright character
    char sunChar = 'O';
    if (intensity > 0.8f) sunChar = '@';
    else if (intensity > 0.5f) sunChar = 'O';
    else sunChar = 'o';
    
    // Simple 3x3 sun
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int x = centerX + dx;
            int y = centerY + dy;
            if (x >= 0 && y >= 0) {
                // In a real implementation, we'd write to a buffer
                // For now, just print position info
                if (dx == 0 && dy == 0) {
                    // Center
                }
            }
        }
    }
}

void SkyRenderer::DrawMoonASCII(int centerX, int centerY, float intensity, MoonPhase phase) {
    if (centerX < 0 || centerY < 0) return;
    
    // Moon characters based on phase
    char moonChar = 'O';
    switch (phase) {
        case MoonPhase::NewMoon: moonChar = ' '; break;
        case MoonPhase::WaxingCrescent: moonChar = ')'; break;
        case MoonPhase::FirstQuarter: moonChar = 'D'; break;
        case MoonPhase::WaxingGibbous: moonChar = 'O'; break;
        case MoonPhase::FullMoon: moonChar = '@'; break;
        case MoonPhase::WaningGibbous: moonChar = 'O'; break;
        case MoonPhase::LastQuarter: moonChar = 'C'; break;
        case MoonPhase::WaningCrescent: moonChar = '('; break;
    }
    
    (void)intensity; // Suppress unused warning for now
    (void)moonChar;  // Will be used in full implementation
}

void SkyRenderer::DrawStarsASCII(int screenW, int screenH, float visibility, 
                                  const DayNightCycle& cycle) {
    if (visibility <= 0.01f) return;
    
    const auto& stars = cycle.GetStars();
    
    // For each star, project to screen and draw
    for (const auto& star : stars) {
        float twinkle = 0.7f + 0.3f * std::sin(star.twinklePhase);
        float finalBrightness = star.brightness * twinkle * visibility;
        
        if (finalBrightness < 0.1f) continue;
        
        // Random-ish position for ASCII (simplified)
        int x = static_cast<int>(star.direction.x * 1000) % screenW;
        int y = static_cast<int>(star.direction.y * 1000) % (screenH / 2);
        
        if (x < 0) x += screenW;
        if (y < 0) y += screenH / 2;
        
        // In full implementation, write to framebuffer
        char starChar = BrightnessToChar(finalBrightness);
        (void)starChar;
    }
}

void SkyRenderer::RenderSkyASCII(const DayNightCycle& cycle, int screenW, int screenH) {
    // Draw sky background info
    DrawSkyGradientASCII(screenW, screenH, cycle.GetCurrentSky());
    
    // Draw sun if visible
    if (cycle.IsSunVisible()) {
        // Simplified: sun at top during day
        int sunX = screenW / 2 + static_cast<int>(cycle.GetSunDirection().x * screenW * 0.3f);
        int sunY = screenH / 4 - static_cast<int>(cycle.GetSunDirection().y * screenH * 0.2f);
        DrawSunASCII(sunX, sunY, cycle.GetSunIntensity(), cycle.GetSunColor());
    }
    
    // Draw moon if visible
    if (cycle.IsMoonVisible()) {
        int moonX = screenW / 2 + static_cast<int>(cycle.GetMoonDirection().x * screenW * 0.3f);
        int moonY = screenH / 4 - static_cast<int>(cycle.GetMoonDirection().y * screenH * 0.2f);
        DrawMoonASCII(moonX, moonY, cycle.GetMoonIntensity(), cycle.GetMoonPhase());
    }
    
    // Draw stars
    float starVis = cycle.GetStarVisibility();
    if (starVis > 0.0f) {
        DrawStarsASCII(screenW, screenH, starVis, cycle);
    }
    
    // Draw time info
    std::cout << "[Time: " << cycle.GetDayTimeString() 
              << " | Phase: " << cycle.GetMoonPhaseName() 
              << " | Ambient: " << static_cast<int>(cycle.GetAmbientIntensity() * 100) << "%]\n";
}

// ============================================
// Framebuffer Sky Rendering
// ============================================
void SkyRenderer::DrawSkyGradientFB(int screenW, int screenH, const SkyColorSet& sky,
                                     FramebufferRenderer* fb) {
    if (!fb || !fb->IsInitialized()) return;
    
    // Draw vertical gradient
    for (int y = 0; y < screenH; y++) {
        float t = static_cast<float>(y) / static_cast<float>(screenH);
        
        // Interpolate between top and bottom colors
        Vec3 color;
        if (t < 0.5f) {
            // Top half: top -> horizon
            float localT = t * 2.0f;
            color = Vec3(
                sky.topColor.x + (sky.horizonColor.x - sky.topColor.x) * localT,
                sky.topColor.y + (sky.horizonColor.y - sky.topColor.y) * localT,
                sky.topColor.z + (sky.horizonColor.z - sky.topColor.z) * localT
            );
        } else {
            // Bottom half: horizon -> bottom
            float localT = (t - 0.5f) * 2.0f;
            color = Vec3(
                sky.horizonColor.x + (sky.bottomColor.x - sky.horizonColor.x) * localT,
                sky.horizonColor.y + (sky.bottomColor.y - sky.horizonColor.y) * localT,
                sky.horizonColor.z + (sky.bottomColor.z - sky.horizonColor.z) * localT
            );
        }
        
        uint32_t pixelColor = Vec3ToColor(color);
        
        // Draw horizontal line
        for (int x = 0; x < screenW; x++) {
            fb->SetPixel(x, y, pixelColor);
        }
    }
}

void SkyRenderer::DrawSunFB(int centerX, int centerY, float intensity, const Vec3& color,
                             FramebufferRenderer* fb) {
    if (!fb || !fb->IsInitialized()) return;
    if (centerX < 0 || centerY < 0) return;
    
    int size = 8 + static_cast<int>(intensity * 8);
    uint32_t sunColor = Vec3ToColor(color * intensity);
    uint32_t glowColor = Vec3ToColor(color * intensity * 0.5f);
    
    // Draw glow
    int glowSize = size + 4;
    for (int dy = -glowSize; dy <= glowSize; dy++) {
        for (int dx = -glowSize; dx <= glowSize; dx++) {
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist <= glowSize) {
                float alpha = 1.0f - (dist / glowSize);
                alpha *= alpha; // Quadratic falloff
                
                int px = centerX + dx;
                int py = centerY + dy;
                if (px >= 0 && py >= 0) {
                    // Simple blend (in real implementation, proper alpha blend)
                    if (alpha > 0.3f) {
                        fb->SetPixel(px, py, glowColor);
                    }
                }
            }
        }
    }
    
    // Draw sun core
    for (int dy = -size; dy <= size; dy++) {
        for (int dx = -size; dx <= size; dx++) {
            if (dx * dx + dy * dy <= size * size) {
                fb->SetPixel(centerX + dx, centerY + dy, sunColor);
            }
        }
    }
}

void SkyRenderer::DrawMoonFB(int centerX, int centerY, float intensity, MoonPhase phase,
                              FramebufferRenderer* fb) {
    if (!fb || !fb->IsInitialized()) return;
    if (centerX < 0 || centerY < 0) return;
    if (phase == MoonPhase::NewMoon) return;
    
    int size = 6;
    uint32_t moonColor = Vec3ToColor(Vec3(0.8f, 0.85f, 1.0f) * intensity);
    uint32_t shadowColor = Vec3ToColor(Vec3(0.1f, 0.12f, 0.15f));
    
    for (int dy = -size; dy <= size; dy++) {
        for (int dx = -size; dx <= size; dx++) {
            if (dx * dx + dy * dy <= size * size) {
                int px = centerX + dx;
                int py = centerY + dy;
                if (px >= 0 && py >= 0) {
                    // Simple phase rendering
                    bool lit = true;
                    switch (phase) {
                        case MoonPhase::WaxingCrescent:
                            lit = (dx > -size / 2);
                            break;
                        case MoonPhase::FirstQuarter:
                            lit = (dx > 0);
                            break;
                        case MoonPhase::WaxingGibbous:
                            lit = (dx > -size / 4);
                            break;
                        case MoonPhase::FullMoon:
                            lit = true;
                            break;
                        case MoonPhase::WaningGibbous:
                            lit = (dx < size / 4);
                            break;
                        case MoonPhase::LastQuarter:
                            lit = (dx < 0);
                            break;
                        case MoonPhase::WaningCrescent:
                            lit = (dx < size / 2);
                            break;
                        default:
                            lit = true;
                    }
                    
                    fb->SetPixel(px, py, lit ? moonColor : shadowColor);
                }
            }
        }
    }
}

void SkyRenderer::DrawStarsFB(int screenW, int screenH, float visibility,
                               const DayNightCycle& cycle, FramebufferRenderer* fb) {
    if (!fb || !fb->IsInitialized()) return;
    if (visibility <= 0.01f) return;
    
    const auto& stars = cycle.GetStars();
    
    for (const auto& star : stars) {
        float twinkle = 0.7f + 0.3f * std::sin(star.twinklePhase);
        float finalBrightness = star.brightness * twinkle * visibility;
        
        if (finalBrightness < 0.05f) continue;
        
        // Project to screen (simplified - just map to upper portion)
        int x = static_cast<int>((star.direction.x * 0.5f + 0.5f) * screenW);
        int y = static_cast<int>((1.0f - star.direction.y) * screenH * 0.6f);
        
        if (x < 0 || x >= screenW || y < 0 || y >= screenH) continue;
        
        // Star color based on color index
        Vec3 starColor;
        switch (star.colorIndex) {
            case 0: starColor = Vec3(0.7f, 0.8f, 1.0f); break; // Bluish
            case 1: starColor = Vec3(1.0f, 1.0f, 1.0f); break; // White
            case 2: starColor = Vec3(1.0f, 0.95f, 0.8f); break; // Yellowish
            case 3: starColor = Vec3(1.0f, 0.7f, 0.6f); break; // Reddish
            default: starColor = Vec3(1.0f, 1.0f, 1.0f);
        }
        
        uint32_t color = Vec3ToColor(starColor * finalBrightness);
        fb->SetPixel(x, y, color);
        
        // Bright stars get a small cross
        if (finalBrightness > 0.7f) {
            if (x > 0) fb->SetPixel(x - 1, y, color);
            if (x < screenW - 1) fb->SetPixel(x + 1, y, color);
            if (y > 0) fb->SetPixel(x, y - 1, color);
            if (y < screenH - 1) fb->SetPixel(x, y + 1, color);
        }
    }
}

void SkyRenderer::RenderSkyFB(const DayNightCycle& cycle, FramebufferRenderer* fb) {
    if (!fb || !fb->IsInitialized()) return;
    
    int screenW = fb->GetWidth();
    int screenH = fb->GetHeight();
    
    // Draw sky gradient
    DrawSkyGradientFB(screenW, screenH, cycle.GetCurrentSky(), fb);
    
    // Draw sun if visible
    if (cycle.IsSunVisible()) {
        int sunX = screenW / 2 + static_cast<int>(cycle.GetSunDirection().x * screenW * 0.3f);
        int sunY = screenH / 3 - static_cast<int>(cycle.GetSunDirection().y * screenH * 0.25f);
        DrawSunFB(sunX, sunY, cycle.GetSunIntensity(), cycle.GetSunColor(), fb);
    }
    
    // Draw moon if visible
    if (cycle.IsMoonVisible()) {
        int moonX = screenW / 2 + static_cast<int>(cycle.GetMoonDirection().x * screenW * 0.3f);
        int moonY = screenH / 3 - static_cast<int>(cycle.GetMoonDirection().y * screenH * 0.25f);
        DrawMoonFB(moonX, moonY, cycle.GetMoonIntensity(), cycle.GetMoonPhase(), fb);
    }
    
    // Draw stars
    float starVis = cycle.GetStarVisibility();
    if (starVis > 0.0f) {
        DrawStarsFB(screenW, screenH, starVis, cycle, fb);
    }
}

} // namespace vge
