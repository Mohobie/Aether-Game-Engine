#include "rendering/weather/weather_effects_renderer.h"
#include "core/weather_system.h"
#include "rendering/sky/day_night_cycle.h"
#include "rendering/framebuffer_renderer.h"
#include "rendering/camera.h"
#include <cmath>
#include <algorithm>

namespace vge {

// ============================================
// Constructor
// ============================================
WeatherEffectsRenderer::WeatherEffectsRenderer(WeatherSystem* weather, DayNightCycle* dayNight)
    : weatherSystem(weather)
    , dayNightCycle(dayNight)
    , stormTimer(0.0f)
    , nextLightningTime(5.0f)
    , windPhase(0.0f)
    , maxScreenDrops(200)
    , screenDropSpawnRate(50.0f)
    , screenDropAccumulator(0.0f)
{
    rainParticles = std::make_unique<ParticleSystem>();
    snowParticles = std::make_unique<ParticleSystem>();
    stormParticles = std::make_unique<ParticleSystem>();
    
    screenDrops.reserve(maxScreenDrops);
}

// ============================================
// Emitter Configurations
// ============================================
EmitterConfig WeatherEffectsRenderer::GetRainConfig(const Camera& camera) const {
    EmitterConfig config;
    
    // Rain falls from above camera
    Vec3 camPos = camera.GetPosition();
    config.spawnPosition = Vec3(camPos.x, camPos.y + 20.0f, camPos.z);
    config.spawnArea = Vec3(30.0f, 0.0f, 30.0f); // Wide area around player
    
    // Rain falls straight down with slight wind influence
    Vec3 windDir = wind.currentDirection;
    float windStrength = wind.currentSpeed * 0.3f;
    config.spawnVelocity = Vec3(windDir.x * windStrength, -15.0f, windDir.z * windStrength);
    config.velocityRandomness = Vec3(0.5f, 2.0f, 0.5f);
    
    config.spawnRate = 500.0f * weatherSystem->GetPrecipitationIntensity();
    config.particleLife = 1.5f;
    config.lifeRandomness = 0.3f;
    config.particleSize = 0.05f;
    config.sizeRandomness = 0.01f;
    // Rain drops - blue-gray
    
    config.fadeOverLife = true;
    config.shrinkOverLife = false;
    config.gravity = Vec3(0, 0, 0); // Velocity already has gravity
    config.drag = 0.0f;
    config.maxParticles = 2000;
    config.loop = true;
    config.worldSpace = true;
    
    return config;
}

EmitterConfig WeatherEffectsRenderer::GetSnowConfig(const Camera& camera) const {
    EmitterConfig config;
    
    Vec3 camPos = camera.GetPosition();
    config.spawnPosition = Vec3(camPos.x, camPos.y + 20.0f, camPos.z);
    config.spawnArea = Vec3(30.0f, 0.0f, 30.0f);
    
    // Snow falls slowly with more wind influence
    Vec3 windDir = wind.currentDirection;
    float windStrength = wind.currentSpeed * 0.5f;
    config.spawnVelocity = Vec3(windDir.x * windStrength, -3.0f, windDir.z * windStrength);
    config.velocityRandomness = Vec3(1.0f, 0.5f, 1.0f);
    
    config.spawnRate = 300.0f * weatherSystem->GetPrecipitationIntensity();
    config.particleLife = 5.0f;
    config.lifeRandomness = 1.0f;
    config.particleSize = 0.15f;
    config.sizeRandomness = 0.05f;
    config.sizeOverLife = 0.5f;
    // Snow flakes - white
    
    config.fadeOverLife = true;
    config.shrinkOverLife = true;
    config.rotate = true;
    config.rotationSpeed = 2.0f;
    config.rotationRandomness = 2.0f;
    config.gravity = Vec3(0, -2.0f, 0); // Light gravity
    config.drag = 0.5f; // High drag for drifting
    config.maxParticles = 1500;
    config.loop = true;
    config.worldSpace = true;
    
    return config;
}

EmitterConfig WeatherEffectsRenderer::GetStormConfig(const Camera& camera) const {
    EmitterConfig config;
    
    Vec3 camPos = camera.GetPosition();
    config.spawnPosition = Vec3(camPos.x, camPos.y + 25.0f, camPos.z);
    config.spawnArea = Vec3(40.0f, 0.0f, 40.0f);
    
    // Storm rain is heavier and windier
    Vec3 windDir = wind.currentDirection;
    float windStrength = wind.currentSpeed * 0.5f;
    config.spawnVelocity = Vec3(windDir.x * windStrength, -20.0f, windDir.z * windStrength);
    config.velocityRandomness = Vec3(1.0f, 3.0f, 1.0f);
    
    config.spawnRate = 800.0f * weatherSystem->GetPrecipitationIntensity();
    config.particleLife = 1.2f;
    config.lifeRandomness = 0.2f;
    config.particleSize = 0.08f;
    config.sizeRandomness = 0.02f;
    // Storm rain - darker
    
    config.fadeOverLife = true;
    config.shrinkOverLife = false;
    config.gravity = Vec3(0, 0, 0);
    config.drag = 0.0f;
    config.maxParticles = 3000;
    config.loop = true;
    config.worldSpace = true;
    
    return config;
}

// ============================================
// Update Methods
// ============================================
void WeatherEffectsRenderer::Update(float deltaTime, const Camera& camera) {
    if (!weatherSystem) return;
    
    // Update wind (always active)
    UpdateWind(deltaTime);
    
    // Update fog (always active, intensity varies)
    UpdateFog(deltaTime);
    
    // Update sky overlay
    UpdateSkyOverlay(deltaTime);
    
    // Update precipitation based on weather type
    WeatherType current = weatherSystem->GetCurrentWeather();
    
    switch (current) {
        case WeatherType::Clear:
            rainParticles->StopEmission();
            snowParticles->StopEmission();
            stormParticles->StopEmission();
            break;
            
        case WeatherType::Rain:
            UpdateRain(deltaTime, camera);
            snowParticles->StopEmission();
            stormParticles->StopEmission();
            break;
            
        case WeatherType::Snow:
            rainParticles->StopEmission();
            UpdateSnow(deltaTime, camera);
            stormParticles->StopEmission();
            break;
            
        case WeatherType::Storm:
            rainParticles->StopEmission();
            snowParticles->StopEmission();
            UpdateStorm(deltaTime, camera);
            UpdateLightning(deltaTime);
            break;
            
        case WeatherType::Fog:
            rainParticles->StopEmission();
            snowParticles->StopEmission();
            stormParticles->StopEmission();
            break;
    }
    
    // Update all particle systems
    rainParticles->Update(deltaTime);
    snowParticles->Update(deltaTime);
    stormParticles->Update(deltaTime);
    
    // Update screen drops for ASCII rendering
    if (weatherSystem->IsRaining() || weatherSystem->IsStorming()) {
        screenDropAccumulator += screenDropSpawnRate * weatherSystem->GetPrecipitationIntensity() * deltaTime;
    }
}

void WeatherEffectsRenderer::UpdateRain(float deltaTime, const Camera& camera) {
    (void)deltaTime;
    
    EmitterConfig config = GetRainConfig(camera);
    rainParticles->SetConfig(config);
    rainParticles->SetSpawnPosition(config.spawnPosition);
    rainParticles->StartEmission();
}

void WeatherEffectsRenderer::UpdateSnow(float deltaTime, const Camera& camera) {
    (void)deltaTime;
    
    EmitterConfig config = GetSnowConfig(camera);
    snowParticles->SetConfig(config);
    snowParticles->SetSpawnPosition(config.spawnPosition);
    snowParticles->StartEmission();
}

void WeatherEffectsRenderer::UpdateStorm(float deltaTime, const Camera& camera) {
    (void)deltaTime;
    
    EmitterConfig config = GetStormConfig(camera);
    stormParticles->SetConfig(config);
    stormParticles->SetSpawnPosition(config.spawnPosition);
    stormParticles->StartEmission();
}

void WeatherEffectsRenderer::UpdateFog(float deltaTime) {
    // Animate fog phase
    fogLayer.phase += deltaTime * fogLayer.turbulenceSpeed;
    
    // Update fog density based on weather
    float targetDensity = weatherSystem->GetFogDensity();
    fogLayer.density = fogLayer.density + (targetDensity - fogLayer.density) * deltaTime * 0.5f;
    
    // Update fog color based on weather
    Vec3 targetColor = weatherSystem->GetFogColor();
    fogLayer.color = Vec3(
        fogLayer.color.x + (targetColor.x - fogLayer.color.x) * deltaTime * 0.5f,
        fogLayer.color.y + (targetColor.y - fogLayer.color.y) * deltaTime * 0.5f,
        fogLayer.color.z + (targetColor.z - fogLayer.color.z) * deltaTime * 0.5f
    );
    
    // Fog thickness varies with density
    fogLayer.thickness = 10.0f + fogLayer.density * 30.0f;
}

void WeatherEffectsRenderer::UpdateWind(float deltaTime) {
    windPhase += deltaTime * 0.5f;
    
    // Animate wind speed with gusts
    float gust = std::sin(windPhase * 3.0f) * 0.5f + std::sin(windPhase * 7.0f) * 0.3f;
    float targetSpeed = wind.baseSpeed + gust * wind.gustiness * wind.baseSpeed;
    wind.currentSpeed = wind.currentSpeed + (targetSpeed - wind.currentSpeed) * deltaTime;
    
    // Animate wind direction with turbulence
    float turbX = std::sin(windPhase * 2.0f) * wind.turbulence;
    float turbZ = std::cos(windPhase * 1.5f) * wind.turbulence;
    Vec3 targetDir = Vec3(
        wind.direction.x + turbX,
        wind.direction.y,
        wind.direction.z + turbZ
    );
    // Normalize
    float len = targetDir.length();
    if (len > 0.0f) {
        targetDir = targetDir / len;
    }
    
    wind.currentDirection = Vec3(
        wind.currentDirection.x + (targetDir.x - wind.currentDirection.x) * deltaTime,
        wind.currentDirection.y + (targetDir.y - wind.currentDirection.y) * deltaTime,
        wind.currentDirection.z + (targetDir.z - wind.currentDirection.z) * deltaTime
    );
}

void WeatherEffectsRenderer::UpdateSkyOverlay(float deltaTime) {
    (void)deltaTime;
    
    WeatherType weather = weatherSystem->GetCurrentWeather();
    float intensity = weatherSystem->GetCloudDensity();
    
    // Set target overlay based on weather
    Vec3 targetColor;
    float targetStrength = 0.0f;
    float targetDarkening = 0.0f;
    float targetStormDarkness = 0.0f;
    
    switch (weather) {
        case WeatherType::Clear:
            targetColor = Vec3(0.5f, 0.6f, 0.7f);
            targetStrength = 0.0f;
            targetDarkening = 0.0f;
            break;
            
        case WeatherType::Rain:
            targetColor = Vec3(0.4f, 0.45f, 0.5f);
            targetStrength = 0.2f * intensity;
            targetDarkening = 0.3f * intensity;
            break;
            
        case WeatherType::Storm:
            targetColor = Vec3(0.2f, 0.22f, 0.25f);
            targetStrength = 0.4f * intensity;
            targetDarkening = 0.6f * intensity;
            targetStormDarkness = 0.3f * intensity;
            break;
            
        case WeatherType::Snow:
            targetColor = Vec3(0.8f, 0.85f, 0.9f);
            targetStrength = 0.1f * intensity;
            targetDarkening = 0.1f * intensity;
            break;
            
        case WeatherType::Fog:
            targetColor = Vec3(0.6f, 0.65f, 0.7f);
            targetStrength = 0.3f * intensity;
            targetDarkening = 0.2f * intensity;
            break;
    }
    
    // Smooth transition
    float t = deltaTime * 0.5f;
    skyOverlay.overlayColor = Vec3(
        skyOverlay.overlayColor.x + (targetColor.x - skyOverlay.overlayColor.x) * t,
        skyOverlay.overlayColor.y + (targetColor.y - skyOverlay.overlayColor.y) * t,
        skyOverlay.overlayColor.z + (targetColor.z - skyOverlay.overlayColor.z) * t
    );
    skyOverlay.overlayStrength = skyOverlay.overlayStrength + (targetStrength - skyOverlay.overlayStrength) * t;
    skyOverlay.cloudDarkening = skyOverlay.cloudDarkening + (targetDarkening - skyOverlay.cloudDarkening) * t;
    skyOverlay.stormDarkness = skyOverlay.stormDarkness + (targetStormDarkness - skyOverlay.stormDarkness) * t;
}

void WeatherEffectsRenderer::UpdateLightning(float deltaTime) {
    stormTimer += deltaTime;
    
    // Random lightning during storms
    if (stormTimer >= nextLightningTime) {
        if (weatherSystem->GetThunderIntensity() > 0.3f) {
            SpawnLightning();
        }
        stormTimer = 0.0f;
        nextLightningTime = 2.0f + static_cast<float>(rand()) / RAND_MAX * 8.0f;
    }
    
    // Update active lightning
    if (lightning.active) {
        lightning.timer -= deltaTime;
        if (lightning.timer <= 0.0f) {
            lightning.active = false;
            lightning.intensity = 0.0f;
        } else {
            // Flash decay
            float flashProgress = lightning.timer / lightning.duration;
            lightning.intensity = lightning.intensity * flashProgress;
        }
    }
}

void WeatherEffectsRenderer::SpawnLightning() {
    lightning.intensity = 0.8f + static_cast<float>(rand()) / RAND_MAX * 0.2f;
    lightning.duration = 0.1f + static_cast<float>(rand()) / RAND_MAX * 0.2f;
    lightning.timer = lightning.duration;
    lightning.active = true;
}

void WeatherEffectsRenderer::TriggerLightning() {
    SpawnLightning();
}

// ============================================
// Render ASCII
// ============================================
char WeatherEffectsRenderer::GetRainChar(float intensity) const {
    if (intensity > 0.8f) return '|';  // Heavy rain
    if (intensity > 0.5f) return '/';  // Medium rain
    return '.';                         // Light rain
}

char WeatherEffectsRenderer::GetSnowChar() const {
    return '*';
}

void WeatherEffectsRenderer::RenderASCII(int screenW, int screenH) {
    if (!weatherSystem) return;
    
    WeatherType weather = weatherSystem->GetCurrentWeather();
    float intensity = weatherSystem->GetPrecipitationIntensity();
    
    // Render precipitation as screen drops
    if (weather == WeatherType::Rain || weather == WeatherType::Storm) {
        // Spawn new drops
        while (screenDropAccumulator >= 1.0f && static_cast<int>(screenDrops.size()) < maxScreenDrops) {
            ScreenDrop drop;
            drop.x = rand() % screenW;
            drop.y = rand() % screenH;
            drop.speed = 5.0f + intensity * 10.0f;
            drop.length = 1.0f + intensity * 2.0f;
            drop.character = GetRainChar(intensity);
            drop.life = 1.0f;
            screenDrops.push_back(drop);
            screenDropAccumulator -= 1.0f;
        }
        
        // Update and render drops
        for (auto it = screenDrops.begin(); it != screenDrops.end(); ) {
            it->y += static_cast<int>(it->speed);
            it->life -= 0.05f;
            
            if (it->y >= screenH || it->life <= 0.0f) {
                it = screenDrops.erase(it);
            } else {
                // In a real implementation, write to screen buffer
                // For now, just track the drops
                ++it;
            }
        }
    } else if (weather == WeatherType::Snow) {
        // Spawn snow flakes
        while (screenDropAccumulator >= 1.0f && static_cast<int>(screenDrops.size()) < maxScreenDrops / 2) {
            ScreenDrop drop;
            drop.x = rand() % screenW;
            drop.y = rand() % screenH;
            drop.speed = 1.0f + intensity * 2.0f;
            drop.length = 1.0f;
            drop.character = GetSnowChar();
            drop.life = 3.0f;
            screenDrops.push_back(drop);
            screenDropAccumulator -= 1.0f;
        }
        
        // Update snow (drifts with wind)
        for (auto it = screenDrops.begin(); it != screenDrops.end(); ) {
            it->y += static_cast<int>(it->speed);
            it->x += static_cast<int>(wind.currentSpeed * 0.5f);
            it->life -= 0.02f;
            
            if (it->y >= screenH || it->life <= 0.0f || it->x >= screenW) {
                it = screenDrops.erase(it);
            } else {
                ++it;
            }
        }
    } else {
        screenDrops.clear();
    }
    
    // Render fog overlay (dim the screen)
    if (weather == WeatherType::Fog || fogLayer.density > 0.05f) {
        // In ASCII, fog is represented by reduced visibility
        // This would be handled by the main renderer
    }
    
    // Lightning flash
    if (lightning.active) {
        // Flash would invert/brighten screen briefly
        // Handled by main renderer
    }
}

// ============================================
// Render Framebuffer
// ============================================
uint32_t WeatherEffectsRenderer::GetRainColor(float intensity) const {
    uint8_t r = static_cast<uint8_t>(std::min(255.0f, 100.0f + intensity * 50.0f));
    uint8_t g = static_cast<uint8_t>(std::min(255.0f, 120.0f + intensity * 60.0f));
    uint8_t b = static_cast<uint8_t>(std::min(255.0f, 150.0f + intensity * 80.0f));
    return RGB(r, g, b);
}

uint32_t WeatherEffectsRenderer::GetSnowColor(float intensity) const {
    uint8_t val = static_cast<uint8_t>(std::min(255.0f, 200.0f + intensity * 55.0f));
    return RGB(val, val, val);
}

void WeatherEffectsRenderer::RenderFB(FramebufferRenderer* fb, const Camera& camera) {
    if (!fb || !fb->IsInitialized()) return;
    
    int screenW = fb->GetWidth();
    int screenH = fb->GetHeight();
    
    // Render precipitation particles
    WeatherType weather = weatherSystem->GetCurrentWeather();
    
    if (weather == WeatherType::Rain) {
        rainParticles->RenderFB(fb, camera);
    } else if (weather == WeatherType::Snow) {
        snowParticles->RenderFB(fb, camera);
    } else if (weather == WeatherType::Storm) {
        stormParticles->RenderFB(fb, camera);
    }
    
    // Render fog overlay
    if (fogLayer.density > 0.01f) {
        // Draw fog as semi-transparent overlay at bottom of screen
        int fogHeight = static_cast<int>(fogLayer.thickness * 2.0f);
        if (fogHeight > screenH) fogHeight = screenH;
        
        uint8_t fr = static_cast<uint8_t>(fogLayer.color.x * 255.0f);
        uint8_t fg = static_cast<uint8_t>(fogLayer.color.y * 255.0f);
        uint8_t fb_ = static_cast<uint8_t>(fogLayer.color.z * 255.0f);
        
        for (int y = screenH - fogHeight; y < screenH; y++) {
            float fogT = static_cast<float>(y - (screenH - fogHeight)) / fogHeight;
            float alpha = fogLayer.density * fogT * 0.3f;
            
            for (int x = 0; x < screenW; x++) {
                // Simple blend - in real implementation, read existing pixel
                if (alpha > 0.5f) {
                    fb->SetPixel(x, y, RGB(fr, fg, fb_));
                }
            }
        }
    }
    
    // Render lightning flash
    if (lightning.active && lightning.intensity > 0.0f) {
        // Flash the entire screen white with intensity
        uint8_t flashVal = static_cast<uint8_t>(lightning.intensity * 255.0f);
        uint32_t flashColor = RGB(flashVal, flashVal, flashVal);
        
        // Draw lightning bolt (simple vertical line with branches)
        int boltX = screenW / 2 + static_cast<int>((static_cast<float>(rand()) / RAND_MAX - 0.5f) * screenW * 0.5f);
        int boltY = 0;
        
        while (boltY < screenH / 2) {
            fb->SetPixel(boltX, boltY, flashColor);
            fb->SetPixel(boltX - 1, boltY, flashColor);
            fb->SetPixel(boltX + 1, boltY, flashColor);
            
            boltY += 2 + rand() % 5;
            boltX += (rand() % 5) - 2;
            
            // Branch occasionally
            if (rand() % 10 == 0) {
                int branchX = boltX;
                int branchY = boltY;
                for (int i = 0; i < 10 && branchY < screenH / 2; i++) {
                    fb->SetPixel(branchX, branchY, flashColor);
                    branchY += 2;
                    branchX += (rand() % 3) - 1;
                }
            }
        }
    }
}

// ============================================
// Modified Sky Colors
// ============================================
Vec3 WeatherEffectsRenderer::GetModifiedSkyTopColor() const {
    if (!dayNightCycle) return Vec3(0.5f, 0.7f, 1.0f);
    
    Vec3 baseColor = dayNightCycle->GetSkyTopColor();
    
    // Apply cloud darkening
    baseColor = baseColor * (1.0f - skyOverlay.cloudDarkening);
    
    // Apply storm darkness
    baseColor = baseColor * (1.0f - skyOverlay.stormDarkness);
    
    // Apply overlay color blend
    baseColor = Vec3(
        baseColor.x + (skyOverlay.overlayColor.x - baseColor.x) * skyOverlay.overlayStrength,
        baseColor.y + (skyOverlay.overlayColor.y - baseColor.y) * skyOverlay.overlayStrength,
        baseColor.z + (skyOverlay.overlayColor.z - baseColor.z) * skyOverlay.overlayStrength
    );
    
    // Apply lightning flash
    if (lightning.active) {
        float flash = lightning.intensity * 0.3f;
        baseColor = Vec3(
            std::min(1.0f, baseColor.x + flash),
            std::min(1.0f, baseColor.y + flash),
            std::min(1.0f, baseColor.z + flash)
        );
    }
    
    return baseColor;
}

float WeatherEffectsRenderer::GetModifiedFogDensity() const {
    return fogLayer.density;
}

} // namespace vge
