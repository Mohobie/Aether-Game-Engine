#include "rendering/sky/day_night_cycle.h"
#include "core/weather_system.h"
#include "rendering/particles/particle_system.h"
#include "rendering/weather/weather_effects_renderer.h"
#include "rendering/camera.h"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace vge;

// ============================================
// Simple Test Framework
// ============================================
static int testsPassed = 0;
static int testsFailed = 0;

#define EXPECT_TRUE(expr) do { \
    if (!(expr)) { \
        std::cerr << "FAIL: " << #expr << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        testsFailed++; \
    } else { \
        testsPassed++; \
    } \
} while(0)

#define EXPECT_FALSE(expr) EXPECT_TRUE(!(expr))

#define EXPECT_FLOAT_EQ(a, b) do { \
    if (std::fabs((a) - (b)) > 0.001f) { \
        std::cerr << "FAIL: " << #a << " == " << #b << " (" << (a) << " != " << (b) << ") at " << __FILE__ << ":" << __LINE__ << std::endl; \
        testsFailed++; \
    } else { \
        testsPassed++; \
    } \
} while(0)

#define EXPECT_NEAR(a, b, tol) do { \
    if (std::fabs((a) - (b)) > (tol)) { \
        std::cerr << "FAIL: " << #a << " ~= " << #b << " (" << (a) << " != " << (b) << ") at " << __FILE__ << ":" << __LINE__ << std::endl; \
        testsFailed++; \
    } else { \
        testsPassed++; \
    } \
} while(0)

#define EXPECT_EQ(a, b) do { \
    if ((a) != (b)) { \
        std::cerr << "FAIL: " << #a << " == " << #b << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        testsFailed++; \
    } else { \
        testsPassed++; \
    } \
} while(0)

#define EXPECT_GT(a, b) do { \
    if ((a) <= (b)) { \
        std::cerr << "FAIL: " << #a << " > " << #b << " (" << (a) << " <= " << (b) << ") at " << __FILE__ << ":" << __LINE__ << std::endl; \
        testsFailed++; \
    } else { \
        testsPassed++; \
    } \
} while(0)

#define EXPECT_LT(a, b) do { \
    if ((a) >= (b)) { \
        std::cerr << "FAIL: " << #a << " < " << #b << " (" << (a) << " >= " << (b) << ") at " << __FILE__ << ":" << __LINE__ << std::endl; \
        testsFailed++; \
    } else { \
        testsPassed++; \
    } \
} while(0)

#define EXPECT_NE(a, b) do { \
    if ((a) == (b)) { \
        std::cerr << "FAIL: " << #a << " != " << #b << " (" << (a) << " == " << (b) << ") at " << __FILE__ << ":" << __LINE__ << std::endl; \
        testsFailed++; \
    } else { \
        testsPassed++; \
    } \
} while(0)

#define EXPECT_GE(a, b) do { \
    if ((a) < (b)) { \
        std::cerr << "FAIL: " << #a << " >= " << #b << " (" << (a) << " < " << (b) << ") at " << __FILE__ << ":" << __LINE__ << std::endl; \
        testsFailed++; \
    } else { \
        testsPassed++; \
    } \
} while(0)

#define EXPECT_LE(a, b) do { \
    if ((a) > (b)) { \
        std::cerr << "FAIL: " << #a << " <= " << #b << " (" << (a) << " > " << (b) << ") at " << __FILE__ << ":" << __LINE__ << std::endl; \
        testsFailed++; \
    } else { \
        testsPassed++; \
    } \
} while(0)
// ============================================
void TestDayNightCycle() {
    std::cout << "\n=== DayNightCycle Tests ===" << std::endl;
    
    {
        DayNightCycle cycle;
        EXPECT_FLOAT_EQ(cycle.GetTimeOfDay(), 6.0f);
        EXPECT_EQ(cycle.GetDayCount(), 0);
        EXPECT_EQ(cycle.GetTimeOfDayTicks(), 6000);
        EXPECT_TRUE(cycle.IsDay());
        EXPECT_FALSE(cycle.IsNight());
    }
    
    {
        DayNightCycle cycle;
        cycle.SetDayLength(24.0f);
        EXPECT_FLOAT_EQ(cycle.GetTimeOfDay(), 6.0f);
        cycle.Update(6.0f);
        EXPECT_NEAR(cycle.GetTimeOfDay(), 12.0f, 0.1f);
        EXPECT_TRUE(cycle.IsDay());
        cycle.Update(6.0f);
        EXPECT_NEAR(cycle.GetTimeOfDay(), 18.0f, 0.1f);
        EXPECT_TRUE(cycle.IsDusk());
        cycle.Update(6.0f);
        EXPECT_NEAR(cycle.GetTimeOfDay(), 0.0f, 0.1f);
        EXPECT_EQ(cycle.GetDayCount(), 1);
        EXPECT_TRUE(cycle.IsNight());
    }
    
    {
        DayNightCycle cycle;
        cycle.SetDayLength(24.0f);
        cycle.Update(48.0f);
        EXPECT_EQ(cycle.GetDayCount(), 2);
        EXPECT_NEAR(cycle.GetTimeOfDay(), 6.0f, 0.1f);
    }
    
    {
        DayNightCycle cycle;
        cycle.SetTime(12.0f);
        EXPECT_FLOAT_EQ(cycle.GetTimeOfDay(), 12.0f);
        EXPECT_TRUE(cycle.IsDay());
        cycle.SetTime(0.0f);
        EXPECT_FLOAT_EQ(cycle.GetTimeOfDay(), 0.0f);
        EXPECT_TRUE(cycle.IsNight());
        cycle.SetTimeOfDayTicks(18000);
        EXPECT_NEAR(cycle.GetTimeOfDay(), 18.0f, 0.01f);
    }
    
    {
        DayNightCycle cycle;
        cycle.SkipToDawn();
        EXPECT_FLOAT_EQ(cycle.GetTimeOfDay(), 6.0f);
        EXPECT_TRUE(cycle.IsDawn());
        cycle.SkipToNoon();
        EXPECT_FLOAT_EQ(cycle.GetTimeOfDay(), 12.0f);
        cycle.SkipToDusk();
        EXPECT_FLOAT_EQ(cycle.GetTimeOfDay(), 18.0f);
        EXPECT_TRUE(cycle.IsDusk());
        cycle.SkipToMidnight();
        EXPECT_FLOAT_EQ(cycle.GetTimeOfDay(), 0.0f);
    }
    
    {
        DayNightCycle cycle;
        cycle.SetTime(12.0f);
        EXPECT_TRUE(cycle.IsSunVisible());
        EXPECT_GT(cycle.GetSunIntensity(), 0.5f);
        EXPECT_GT(cycle.GetSunDirection().y, 0.0f);
        cycle.SetTime(0.0f);
        EXPECT_FALSE(cycle.IsSunVisible());
        EXPECT_FLOAT_EQ(cycle.GetSunIntensity(), 0.0f);
    }
    
    {
        DayNightCycle cycle;
        cycle.SetTime(0.0f);
        EXPECT_TRUE(cycle.IsMoonVisible());
        EXPECT_GT(cycle.GetMoonIntensity(), 0.0f);
        cycle.SetTime(12.0f);
        EXPECT_FALSE(cycle.IsMoonVisible());
    }
    
    {
        DayNightCycle cycle;
        cycle.SetDayLength(24.0f);
        EXPECT_EQ(cycle.GetMoonPhase(), MoonPhase::FullMoon);
        EXPECT_FLOAT_EQ(cycle.GetMoonPhaseBrightness(), 1.0f);
        cycle.Update(72.0f);
        EXPECT_EQ(cycle.GetMoonPhase(), MoonPhase::WaningGibbous);
        cycle.Update(72.0f);
        EXPECT_EQ(cycle.GetMoonPhase(), MoonPhase::LastQuarter);
    }
    
    {
        DayNightCycle cycle;
        cycle.SetTime(12.0f);
        Vec3 dayTop = cycle.GetSkyTopColor();
        EXPECT_GT(dayTop.x, 0.3f);
        EXPECT_GT(dayTop.y, 0.5f);
        EXPECT_GT(dayTop.z, 0.7f);
        cycle.SetTime(0.0f);
        Vec3 nightTop = cycle.GetSkyTopColor();
        EXPECT_LT(nightTop.x, 0.3f);
        EXPECT_LT(nightTop.y, 0.4f);
        EXPECT_LT(nightTop.z, 0.6f);
    }
    
    {
        DayNightCycle cycle;
        cycle.SetTime(12.0f);
        EXPECT_GT(cycle.GetAmbientIntensity(), 0.4f);
        cycle.SetTime(0.0f);
        EXPECT_LT(cycle.GetAmbientIntensity(), 0.2f);
    }
    
    {
        DayNightCycle cycle;
        cycle.SetTime(0.0f);
        EXPECT_FLOAT_EQ(cycle.GetStarVisibility(), 1.0f);
        cycle.SetTime(12.0f);
        EXPECT_FLOAT_EQ(cycle.GetStarVisibility(), 0.0f);
        cycle.SetTime(5.5f);
        EXPECT_GT(cycle.GetStarVisibility(), 0.0f);
        EXPECT_LT(cycle.GetStarVisibility(), 1.0f);
    }
    
    {
        DayNightCycle cycle;
        cycle.SetTime(0.0f);
        EXPECT_FLOAT_EQ(cycle.GetDayNightBlend(), 0.0f);
        cycle.SetTime(12.0f);
        EXPECT_FLOAT_EQ(cycle.GetDayNightBlend(), 1.0f);
        cycle.SetTime(6.0f);
        EXPECT_GT(cycle.GetDayNightBlend(), 0.0f);
        EXPECT_LT(cycle.GetDayNightBlend(), 1.0f);
    }
    
    {
        DayNightCycle cycle;
        EXPECT_FALSE(cycle.GetStars().empty());
        EXPECT_GE(cycle.GetStars().size(), 400);
    }
    
    {
        DayNightCycle cycle;
        cycle.SetTime(14.5f);
        EXPECT_EQ(cycle.GetTimeString(), "14:30");
        EXPECT_EQ(cycle.GetDayTimeString(), "Day 1, 14:30");
    }
    
    {
        DayNightCycle cycle;
        cycle.SetDayLength(24.0f);
        cycle.Pause();
        EXPECT_TRUE(cycle.IsPaused());
        float before = cycle.GetTimeOfDay();
        cycle.Update(10.0f);
        EXPECT_FLOAT_EQ(cycle.GetTimeOfDay(), before);
        cycle.Resume();
        EXPECT_FALSE(cycle.IsPaused());
        cycle.Update(1.0f);
        EXPECT_NE(cycle.GetTimeOfDay(), before);
    }
    
    {
        DayNightCycle cycle;
        cycle.SetDayLength(24.0f);
        cycle.SetTimeScale(1.0f);
        cycle.SetTime(6.0f);
        cycle.Update(1.0f);
        EXPECT_NEAR(cycle.GetTimeOfDay(), 7.0f, 0.1f);
        cycle.SetTimeScale(2.0f);
        cycle.SetTime(6.0f);
        cycle.Update(1.0f);
        EXPECT_NEAR(cycle.GetTimeOfDay(), 8.0f, 0.1f);
        cycle.SetTimeScale(0.5f);
        cycle.SetTime(6.0f);
        cycle.Update(1.0f);
        EXPECT_NEAR(cycle.GetTimeOfDay(), 6.5f, 0.1f);
    }
}

// ============================================
// WeatherSystem Tests
// ============================================
void TestWeatherSystem() {
    std::cout << "\n=== WeatherSystem Tests ===" << std::endl;
    
    {
        WeatherSystem weather;
        EXPECT_EQ(weather.GetCurrentWeather(), WeatherType::Clear);
        EXPECT_FALSE(weather.IsRaining());
        EXPECT_FALSE(weather.IsStorming());
        EXPECT_FALSE(weather.IsSnowing());
        EXPECT_FALSE(weather.IsFoggy());
    }
    
    {
        WeatherSystem weather;
        weather.SetWeather(WeatherType::Rain, 60.0f);
        for (int i = 0; i < 20; i++) {
            weather.Update(1.0f);
        }
        EXPECT_EQ(weather.GetCurrentWeather(), WeatherType::Rain);
        EXPECT_TRUE(weather.IsRaining());
        EXPECT_GT(weather.GetPrecipitationIntensity(), 0.0f);
    }
    
    {
        WeatherSystem weather;
        EXPECT_FLOAT_EQ(weather.GetCloudDensity(), 0.0f);
        EXPECT_FLOAT_EQ(weather.GetPrecipitationIntensity(), 0.0f);
        EXPECT_FLOAT_EQ(weather.GetFogDensity(), 0.0f);
        
        weather.SetWeather(WeatherType::Storm, 60.0f);
        for (int i = 0; i < 20; i++) {
            weather.Update(1.0f);
        }
        EXPECT_GT(weather.GetCloudDensity(), 0.5f);
        EXPECT_GT(weather.GetPrecipitationIntensity(), 0.5f);
        EXPECT_GT(weather.GetFogDensity(), 0.0f);
        EXPECT_GT(weather.GetThunderIntensity(), 0.0f);
        EXPECT_GT(weather.GetWindSpeed(), 5.0f);
    }
    
    {
        WeatherSystem weather;
        weather.SetWeather(WeatherType::Storm, 60.0f);
        for (int i = 0; i < 20; i++) {
            weather.Update(1.0f);
        }
        EXPECT_TRUE(weather.IsRaining());
        EXPECT_TRUE(weather.IsStorming());
        EXPECT_FALSE(weather.IsSnowing());
        
        weather.SetWeather(WeatherType::Snow, 60.0f);
        for (int i = 0; i < 20; i++) {
            weather.Update(1.0f);
        }
        EXPECT_TRUE(weather.IsSnowing());
        EXPECT_FALSE(weather.IsRaining());
        EXPECT_FALSE(weather.IsStorming());
    }
    
    {
        WeatherSystem weather;
        EXPECT_EQ(weather.GetWeatherName(), "Clear");
        weather.SetWeather(WeatherType::Rain, 60.0f);
        for (int i = 0; i < 20; i++) {
            weather.Update(1.0f);
        }
        EXPECT_EQ(weather.GetWeatherName(), "Rain");
    }
}

// ============================================
// ParticleSystem Tests
// ============================================
void TestParticleSystem() {
    std::cout << "\n=== ParticleSystem Tests ===" << std::endl;
    
    {
        ParticleSystem particles;
        EXPECT_EQ(particles.GetActiveCount(), 0);
        EXPECT_FALSE(particles.IsEmitting());
    }
    
    {
        ParticleSystem particles;
        EmitterConfig config;
        config.spawnRate = 10.0f;
        config.maxParticles = 100;
        config.loop = true;
        
        particles.SetConfig(config);
        particles.StartEmission();
        EXPECT_TRUE(particles.IsEmitting());
        
        particles.Update(1.0f);
        EXPECT_GE(particles.GetActiveCount(), 10);
        
        particles.Update(1.0f);
        EXPECT_GE(particles.GetActiveCount(), 10);
        
        particles.StopEmission();
        EXPECT_FALSE(particles.IsEmitting());
    }
    
    {
        ParticleSystem particles;
        EmitterConfig config;
        config.spawnRate = 100.0f;
        config.particleLife = 1.0f;
        config.maxParticles = 100;
        config.loop = true;
        
        particles.SetConfig(config);
        particles.StartEmission();
        particles.Update(0.5f);
        int countAfterSpawn = particles.GetActiveCount();
        EXPECT_GT(countAfterSpawn, 0);
        
        particles.Update(2.0f);
        EXPECT_EQ(particles.GetActiveCount(), 0);
    }
    
    {
        ParticleSystem particles;
        EmitterConfig config;
        config.spawnRate = 1000.0f;
        config.maxParticles = 50;
        config.particleLife = 5.0f;
        config.loop = true;
        
        particles.SetConfig(config);
        particles.StartEmission();
        particles.Update(1.0f);
        EXPECT_LE(particles.GetActiveCount(), 100);
        EXPECT_EQ(particles.GetMaxParticles(), 50);
    }
    
    {
        ParticleSystem particles;
        EmitterConfig config;
        config.burstMode = true;
        config.burstCount = 25;
        config.maxParticles = 100;
        config.loop = false;
        
        particles.SetConfig(config);
        particles.StartEmission();
        EXPECT_EQ(particles.GetActiveCount(), 25);
        EXPECT_FALSE(particles.IsEmitting());
    }
    
    {
        ParticleSystem particles;
        EmitterConfig config;
        config.spawnRate = 100.0f;
        config.maxParticles = 100;
        config.loop = true;
        
        particles.SetConfig(config);
        particles.StartEmission();
        particles.Update(1.0f);
        EXPECT_GT(particles.GetActiveCount(), 0);
        
        particles.Clear();
        EXPECT_EQ(particles.GetActiveCount(), 0);
    }
    
    {
        ParticleSystem particles;
        particles.SetSpawnPosition(Vec3(10.0f, 20.0f, 30.0f));
        const EmitterConfig& config = particles.GetConfig();
        EXPECT_FLOAT_EQ(config.spawnPosition.x, 10.0f);
        EXPECT_FLOAT_EQ(config.spawnPosition.y, 20.0f);
        EXPECT_FLOAT_EQ(config.spawnPosition.z, 30.0f);
    }
}

// ============================================
// WeatherEffectsRenderer Tests
// ============================================
void TestWeatherEffectsRenderer() {
    std::cout << "\n=== WeatherEffectsRenderer Tests ===" << std::endl;
    
    {
        WeatherSystem weather;
        DayNightCycle dayNight;
        WeatherEffectsRenderer renderer(&weather, &dayNight);
        EXPECT_FALSE(renderer.IsLightningActive());
        EXPECT_FLOAT_EQ(renderer.GetLightningIntensity(), 0.0f);
    }
    
    {
        WeatherSystem weather;
        DayNightCycle dayNight;
        WeatherEffectsRenderer renderer(&weather, &dayNight);
        Camera camera;
        
        weather.SetWeather(WeatherType::Storm, 60.0f);
        for (int i = 0; i < 20; i++) {
            weather.Update(1.0f);
        }
        
        renderer.Update(1.0f, camera);
        const WindField& wind = renderer.GetWind();
        EXPECT_GT(wind.currentSpeed, 0.0f);
    }
    
    {
        WeatherSystem weather;
        DayNightCycle dayNight;
        WeatherEffectsRenderer renderer(&weather, &dayNight);
        Camera camera;
        
        weather.SetWeather(WeatherType::Fog, 60.0f);
        for (int i = 0; i < 20; i++) {
            weather.Update(1.0f);
        }
        
        renderer.Update(1.0f, camera);
        const FogLayer& fog = renderer.GetFog();
        EXPECT_GT(fog.density, 0.0f);
    }
    
    {
        WeatherSystem weather;
        DayNightCycle dayNight;
        WeatherEffectsRenderer renderer(&weather, &dayNight);
        Camera camera;
        
        renderer.Update(1.0f, camera);
        const SkyOverlay& overlay = renderer.GetSkyOverlay();
        EXPECT_LT(overlay.overlayStrength, 0.1f);
    }
    
    {
        WeatherSystem weather;
        DayNightCycle dayNight;
        WeatherEffectsRenderer renderer(&weather, &dayNight);
        Camera camera;
        
        weather.SetWeather(WeatherType::Storm, 60.0f);
        for (int i = 0; i < 20; i++) {
            weather.Update(1.0f);
        }
        
        renderer.Update(1.0f, camera);
        const SkyOverlay& overlay = renderer.GetSkyOverlay();
        EXPECT_GT(overlay.overlayStrength, 0.1f);
        EXPECT_GT(overlay.cloudDarkening, 0.1f);
    }
    
    {
        WeatherSystem weather;
        DayNightCycle dayNight;
        WeatherEffectsRenderer renderer(&weather, &dayNight);
        
        EXPECT_FALSE(renderer.IsLightningActive());
        renderer.TriggerLightning();
        EXPECT_TRUE(renderer.IsLightningActive());
        EXPECT_GT(renderer.GetLightningIntensity(), 0.0f);
    }
    
    {
        WeatherSystem weather;
        DayNightCycle dayNight;
        WeatherEffectsRenderer renderer(&weather, &dayNight);
        
        dayNight.SetTime(12.0f);
        Vec3 clearTop = renderer.GetModifiedSkyTopColor();
        
        weather.SetWeather(WeatherType::Storm, 60.0f);
        for (int i = 0; i < 20; i++) {
            weather.Update(1.0f);
        }
        
        Vec3 stormTop = renderer.GetModifiedSkyTopColor();
        EXPECT_LE(stormTop.x, clearTop.x);
        EXPECT_LE(stormTop.y, clearTop.y);
        EXPECT_LE(stormTop.z, clearTop.z);
    }
    
    {
        WeatherSystem weather;
        DayNightCycle dayNight;
        WeatherEffectsRenderer renderer(&weather, &dayNight);
        
        float clearFog = renderer.GetModifiedFogDensity();
        
        weather.SetWeather(WeatherType::Fog, 60.0f);
        for (int i = 0; i < 20; i++) {
            weather.Update(1.0f);
        }
        
        float fogDensity = renderer.GetModifiedFogDensity();
        EXPECT_GE(fogDensity, clearFog);
    }
}

// ============================================
// Integration Tests
// ============================================
void TestIntegration() {
    std::cout << "\n=== Integration Tests ===" << std::endl;
    
    {
        DayNightCycle cycle;
        WeatherSystem weather;
        
        cycle.SetTime(6.0f);
        weather.SetWeather(WeatherType::Rain, 300.0f);
        for (int i = 0; i < 20; i++) {
            weather.Update(1.0f);
        }
        
        EXPECT_TRUE(cycle.IsDawn());
        EXPECT_TRUE(weather.IsRaining());
        Vec3 skyColor = cycle.GetSkyTopColor();
        EXPECT_TRUE(cycle.IsDay() || cycle.IsDawn());
    }
    
    {
        WeatherSystem weather;
        DayNightCycle dayNight;
        WeatherEffectsRenderer renderer(&weather, &dayNight);
        Camera camera;
        
        weather.SetWeather(WeatherType::Rain, 60.0f);
        for (int i = 0; i < 20; i++) {
            weather.Update(1.0f);
        }
        
        renderer.SetWindSpeed(5.0f);
        renderer.SetWindDirection(Vec3(1.0f, 0.0f, 0.0f));
        renderer.Update(1.0f, camera);
        
        const WindField& wind = renderer.GetWind();
        EXPECT_FLOAT_EQ(wind.baseSpeed, 5.0f);
    }
    
    {
        WeatherSystem weather;
        DayNightCycle dayNight;
        WeatherEffectsRenderer renderer(&weather, &dayNight);
        Camera camera;
        
        weather.SetWeather(WeatherType::Clear, 10.0f);
        for (int i = 0; i < 20; i++) {
            weather.Update(1.0f);
        }
        renderer.Update(1.0f, camera);
        EXPECT_FALSE(weather.IsStorming());
        
        weather.SetWeather(WeatherType::Storm, 60.0f);
        for (int i = 0; i < 20; i++) {
            weather.Update(1.0f);
        }
        renderer.Update(1.0f, camera);
        
        EXPECT_TRUE(weather.IsStorming());
        EXPECT_GT(weather.GetThunderIntensity(), 0.0f);
        
        Vec3 stormSky = renderer.GetModifiedSkyTopColor();
        EXPECT_LT(stormSky.x, 0.5f);
        EXPECT_LT(stormSky.y, 0.5f);
        EXPECT_LE(stormSky.z, 0.6f);
    }
}

// ============================================
// Main
// ============================================
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  Aether Game Engine - Weather Tests    " << std::endl;
    std::cout << "========================================" << std::endl;
    
    TestDayNightCycle();
    TestWeatherSystem();
    TestParticleSystem();
    TestWeatherEffectsRenderer();
    TestIntegration();
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "  Results: " << testsPassed << " passed, " << testsFailed << " failed" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return testsFailed > 0 ? 1 : 0;
}
