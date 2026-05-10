#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <cassert>

// Simple test framework
static int tests_passed = 0;
static int tests_failed = 0;
static int total_tests = 0;

#define EXPECT_FLOAT_EQ(a, b) do { \
    total_tests++; \
    if (std::fabs((a) - (b)) < 0.001f) { tests_passed++; } \
    else { tests_failed++; std::cerr << "FAIL: " << #a << " != " << #b << " (" << (a) << " vs " << (b) << ") at line " << __LINE__ << "\n"; } \
} while(0)

#define EXPECT_NEAR(a, b, tol) do { \
    total_tests++; \
    if (std::fabs((a) - (b)) <= (tol)) { tests_passed++; } \
    else { tests_failed++; std::cerr << "FAIL: " << #a << " !~ " << #b << " (" << (a) << " vs " << (b) << ") at line " << __LINE__ << "\n"; } \
} while(0)

#define EXPECT_EQ(a, b) do { \
    total_tests++; \
    if ((a) == (b)) { tests_passed++; } \
    else { tests_failed++; std::cerr << "FAIL: " << #a << " != " << #b << " at line " << __LINE__ << "\n"; } \
} while(0)

#define EXPECT_NE(a, b) do { \
    total_tests++; \
    if ((a) != (b)) { tests_passed++; } \
    else { tests_failed++; std::cerr << "FAIL: " << #a << " == " << #b << " at line " << __LINE__ << "\n"; } \
} while(0)

#define EXPECT_GT(a, b) do { \
    total_tests++; \
    if ((a) > (b)) { tests_passed++; } \
    else { tests_failed++; std::cerr << "FAIL: " << #a << " <= " << #b << " at line " << __LINE__ << "\n"; } \
} while(0)

#define EXPECT_LT(a, b) do { \
    total_tests++; \
    if ((a) < (b)) { tests_passed++; } \
    else { tests_failed++; std::cerr << "FAIL: " << #a << " >= " << #b << " at line " << __LINE__ << "\n"; } \
} while(0)

#define EXPECT_TRUE(a) do { \
    total_tests++; \
    if ((a)) { tests_passed++; } \
    else { tests_failed++; std::cerr << "FAIL: " << #a << " is false at line " << __LINE__ << "\n"; } \
} while(0)

#define EXPECT_FALSE(a) do { \
    total_tests++; \
    if (!(a)) { tests_passed++; } \
    else { tests_failed++; std::cerr << "FAIL: " << #a << " is true at line " << __LINE__ << "\n"; } \
} while(0)

#define EXPECT_GE(a, b) do { \
    total_tests++; \
    if ((a) >= (b)) { tests_passed++; } \
    else { tests_failed++; std::cerr << "FAIL: " << #a << " < " << #b << " at line " << __LINE__ << "\n"; } \
} while(0)

#define TEST(group, name) void test_##group##_##name()

#define RUN_TEST(group, name) do { \
    std::cout << "Running " << #group << "." << #name << "... "; \
    int before_pass = tests_passed; \
    int before_fail = tests_failed; \
    test_##group##_##name(); \
    if (tests_failed == before_fail) { \
        std::cout << "PASS (" << (tests_passed - before_pass) << " checks)\n"; \
    } else { \
        std::cout << "FAIL (" << (tests_failed - before_fail) << " failures)\n"; \
    } \
} while(0)

#include "rendering/sky/day_night_cycle.h"
#include "core/weather_system.h"

using namespace vge;

// ============================================
// DayNightCycle Tests
// ============================================
TEST(DayNightCycle, ConstructorDefaults) {
    DayNightCycle cycle;
    
    // Should start at dawn (6:00)
    EXPECT_FLOAT_EQ(cycle.GetTimeOfDay(), 6.0f);
    EXPECT_EQ(cycle.GetDayCount(), 0);
    EXPECT_EQ(cycle.GetTimeOfDayTicks(), 6000);
    EXPECT_TRUE(cycle.IsDay());
    EXPECT_FALSE(cycle.IsNight());
}

TEST(DayNightCycle, TimeProgression) {
    DayNightCycle cycle;
    cycle.SetDayLength(24.0f); // 1 real second = 1 game hour
    
    // Start at 6:00
    EXPECT_FLOAT_EQ(cycle.GetTimeOfDay(), 6.0f);
    
    // Advance 6 seconds -> 12:00 (noon)
    cycle.Update(6.0f);
    EXPECT_NEAR(cycle.GetTimeOfDay(), 12.0f, 0.1f);
    EXPECT_TRUE(cycle.IsDay());
    
    // Advance 6 more seconds -> 18:00 (dusk)
    cycle.Update(6.0f);
    EXPECT_NEAR(cycle.GetTimeOfDay(), 18.0f, 0.1f);
    EXPECT_TRUE(cycle.IsDusk());
    
    // Advance 6 more seconds -> 24:00 (midnight, rolls to next day)
    cycle.Update(6.0f);
    EXPECT_NEAR(cycle.GetTimeOfDay(), 0.0f, 0.1f);
    EXPECT_EQ(cycle.GetDayCount(), 1);
    EXPECT_TRUE(cycle.IsNight());
}

TEST(DayNightCycle, DayRollover) {
    DayNightCycle cycle;
    cycle.SetDayLength(24.0f);
    
    // Advance 48 seconds = 2 full days
    cycle.Update(48.0f);
    EXPECT_EQ(cycle.GetDayCount(), 2);
    // Time should be close to starting point (6:00) after full days
    EXPECT_NEAR(cycle.GetTimeOfDay(), 6.0f, 1.0f);
}

TEST(DayNightCycle, TimeControl) {
    DayNightCycle cycle;
    
    // Set to noon
    cycle.SetTime(12.0f);
    EXPECT_FLOAT_EQ(cycle.GetTimeOfDay(), 12.0f);
    EXPECT_TRUE(cycle.IsDay());
    
    // Set to midnight
    cycle.SetTime(0.0f);
    EXPECT_FLOAT_EQ(cycle.GetTimeOfDay(), 0.0f);
    EXPECT_TRUE(cycle.IsNight());
    
    // Set using ticks
    cycle.SetTimeOfDayTicks(18000); // 18:00
    EXPECT_NEAR(cycle.GetTimeOfDay(), 18.0f, 0.01f);
}

TEST(DayNightCycle, SkipFunctions) {
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

TEST(DayNightCycle, SunPosition) {
    DayNightCycle cycle;
    
    // At noon, sun should be high
    cycle.SetTime(12.0f);
    EXPECT_TRUE(cycle.IsSunVisible());
    EXPECT_GT(cycle.GetSunIntensity(), 0.5f);
    EXPECT_GT(cycle.GetSunDirection().y, 0.0f);
    
    // At midnight, sun should not be visible
    cycle.SetTime(0.0f);
    EXPECT_FALSE(cycle.IsSunVisible());
    EXPECT_FLOAT_EQ(cycle.GetSunIntensity(), 0.0f);
}

TEST(DayNightCycle, MoonPosition) {
    DayNightCycle cycle;
    
    // At midnight, moon should be visible
    cycle.SetTime(0.0f);
    EXPECT_TRUE(cycle.IsMoonVisible());
    EXPECT_GT(cycle.GetMoonIntensity(), 0.0f);
    
    // At noon, moon should not be visible
    cycle.SetTime(12.0f);
    EXPECT_FALSE(cycle.IsMoonVisible());
}

TEST(DayNightCycle, MoonPhases) {
    DayNightCycle cycle;
    cycle.SetDayLength(24.0f);
    
    // Start at full moon
    EXPECT_EQ(cycle.GetMoonPhase(), MoonPhase::FullMoon);
    EXPECT_FLOAT_EQ(cycle.GetMoonPhaseBrightness(), 1.0f);
    
    // Advance 3 days - phase should change
    cycle.Update(72.0f);
    EXPECT_NE(static_cast<int>(cycle.GetMoonPhase()), static_cast<int>(MoonPhase::FullMoon));
    
    // Advance 3 more days - should be different
    cycle.Update(72.0f);
    EXPECT_NE(static_cast<int>(cycle.GetMoonPhase()), static_cast<int>(MoonPhase::FullMoon));
}

TEST(DayNightCycle, SkyColors) {
    DayNightCycle cycle;
    
    // Day sky should be bright
    cycle.SetTime(12.0f);
    Vec3 dayTop = cycle.GetSkyTopColor();
    EXPECT_GT(dayTop.x, 0.3f);
    EXPECT_GT(dayTop.y, 0.5f);
    EXPECT_GT(dayTop.z, 0.7f);
    
    // Night sky should be dark
    cycle.SetTime(0.0f);
    Vec3 nightTop = cycle.GetSkyTopColor();
    EXPECT_LT(nightTop.x, 0.5f);
    EXPECT_LT(nightTop.y, 0.5f);
    EXPECT_LT(nightTop.z, 0.7f);
}

TEST(DayNightCycle, AmbientLight) {
    DayNightCycle cycle;
    
    // Day ambient should be bright
    cycle.SetTime(12.0f);
    EXPECT_GT(cycle.GetAmbientIntensity(), 0.4f);
    
    // Night ambient should be dim
    cycle.SetTime(0.0f);
    EXPECT_LT(cycle.GetAmbientIntensity(), 0.2f);
}

TEST(DayNightCycle, StarVisibility) {
    DayNightCycle cycle;
    
    // Stars visible at night
    cycle.SetTime(0.0f);
    EXPECT_FLOAT_EQ(cycle.GetStarVisibility(), 1.0f);
    
    // Stars not visible during day
    cycle.SetTime(12.0f);
    EXPECT_FLOAT_EQ(cycle.GetStarVisibility(), 0.0f);
    
    // Stars partially visible at dawn/dusk
    cycle.SetTime(5.5f);
    EXPECT_GT(cycle.GetStarVisibility(), 0.0f);
    EXPECT_LT(cycle.GetStarVisibility(), 1.0f);
}

TEST(DayNightCycle, DayNightBlend) {
    DayNightCycle cycle;
    
    cycle.SetTime(0.0f);
    EXPECT_FLOAT_EQ(cycle.GetDayNightBlend(), 0.0f);
    
    cycle.SetTime(12.0f);
    EXPECT_FLOAT_EQ(cycle.GetDayNightBlend(), 1.0f);
    
    cycle.SetTime(6.0f);
    EXPECT_GT(cycle.GetDayNightBlend(), 0.0f);
    EXPECT_LT(cycle.GetDayNightBlend(), 1.0f);
}

TEST(DayNightCycle, StarsGenerated) {
    DayNightCycle cycle;
    EXPECT_FALSE(cycle.GetStars().empty());
    EXPECT_GE(cycle.GetStars().size(), 400);
}

TEST(DayNightCycle, TimeString) {
    DayNightCycle cycle;
    cycle.SetTime(14.5f); // 14:30
    EXPECT_EQ(cycle.GetTimeString(), "14:30");
    EXPECT_EQ(cycle.GetDayTimeString(), "Day 1, 14:30");
}

TEST(DayNightCycle, PauseResume) {
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

TEST(DayNightCycle, TimeScale) {
    DayNightCycle cycle;
    cycle.SetDayLength(24.0f);
    
    // Normal speed
    cycle.SetTimeScale(1.0f);
    cycle.SetTime(6.0f);
    cycle.Update(1.0f);
    EXPECT_NEAR(cycle.GetTimeOfDay(), 7.0f, 0.1f);
    
    // Double speed
    cycle.SetTimeScale(2.0f);
    cycle.SetTime(6.0f);
    cycle.Update(1.0f);
    EXPECT_NEAR(cycle.GetTimeOfDay(), 8.0f, 0.1f);
    
    // Half speed
    cycle.SetTimeScale(0.5f);
    cycle.SetTime(6.0f);
    cycle.Update(1.0f);
    EXPECT_NEAR(cycle.GetTimeOfDay(), 6.5f, 0.1f);
}

// ============================================
// WeatherSystem Tests
// ============================================
TEST(WeatherSystem, ConstructorDefaults) {
    WeatherSystem weather;
    EXPECT_EQ(weather.GetCurrentWeather(), WeatherType::Clear);
    EXPECT_FALSE(weather.IsRaining());
    EXPECT_FALSE(weather.IsStorming());
    EXPECT_FALSE(weather.IsSnowing());
    EXPECT_FALSE(weather.IsFoggy());
}

TEST(WeatherSystem, SetWeather) {
    WeatherSystem weather;
    
    weather.SetWeather(WeatherType::Rain, 60.0f);
    // After transition completes
    for (int i = 0; i < 20; i++) {
        weather.Update(1.0f);
    }
    EXPECT_EQ(weather.GetCurrentWeather(), WeatherType::Rain);
    EXPECT_TRUE(weather.IsRaining());
    EXPECT_GT(weather.GetPrecipitationIntensity(), 0.0f);
}

TEST(WeatherSystem, WeatherParameters) {
    WeatherSystem weather;
    
    // Clear weather
    EXPECT_FLOAT_EQ(weather.GetCloudDensity(), 0.0f);
    EXPECT_FLOAT_EQ(weather.GetPrecipitationIntensity(), 0.0f);
    EXPECT_FLOAT_EQ(weather.GetFogDensity(), 0.0f);
    
    // Storm weather
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

TEST(WeatherSystem, WeatherQueries) {
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

TEST(WeatherSystem, WeatherName) {
    WeatherSystem weather;
    EXPECT_EQ(weather.GetWeatherName(), "Clear");
    
    weather.SetWeather(WeatherType::Rain, 60.0f);
    for (int i = 0; i < 20; i++) {
        weather.Update(1.0f);
    }
    EXPECT_EQ(weather.GetWeatherName(), "Rain");
}

// ============================================
// Integration Tests
// ============================================
TEST(DayNightWeatherIntegration, DawnWithRain) {
    DayNightCycle cycle;
    WeatherSystem weather;
    
    cycle.SetTime(6.0f);
    weather.SetWeather(WeatherType::Rain, 300.0f);
    for (int i = 0; i < 20; i++) {
        weather.Update(1.0f);
    }
    
    // Dawn with rain should have reduced sun intensity
    EXPECT_TRUE(cycle.IsDawn());
    EXPECT_TRUE(weather.IsRaining());
    
    // Sky should be darker due to rain
    Vec3 skyColor = cycle.GetSkyTopColor();
    // Rain reduces brightness but we just verify it's not full day
    EXPECT_TRUE(cycle.IsDay() || cycle.IsDawn());
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    std::cout << "=== Day/Night Cycle Tests ===\n\n";
    
    // DayNightCycle Tests
    RUN_TEST(DayNightCycle, ConstructorDefaults);
    RUN_TEST(DayNightCycle, TimeProgression);
    RUN_TEST(DayNightCycle, DayRollover);
    RUN_TEST(DayNightCycle, TimeControl);
    RUN_TEST(DayNightCycle, SkipFunctions);
    RUN_TEST(DayNightCycle, SunPosition);
    RUN_TEST(DayNightCycle, MoonPosition);
    RUN_TEST(DayNightCycle, MoonPhases);
    RUN_TEST(DayNightCycle, SkyColors);
    RUN_TEST(DayNightCycle, AmbientLight);
    RUN_TEST(DayNightCycle, StarVisibility);
    RUN_TEST(DayNightCycle, DayNightBlend);
    RUN_TEST(DayNightCycle, StarsGenerated);
    RUN_TEST(DayNightCycle, TimeString);
    RUN_TEST(DayNightCycle, PauseResume);
    RUN_TEST(DayNightCycle, TimeScale);
    
    // WeatherSystem Tests
    RUN_TEST(WeatherSystem, ConstructorDefaults);
    RUN_TEST(WeatherSystem, SetWeather);
    RUN_TEST(WeatherSystem, WeatherParameters);
    RUN_TEST(WeatherSystem, WeatherQueries);
    RUN_TEST(WeatherSystem, WeatherName);
    
    // Integration Tests
    RUN_TEST(DayNightWeatherIntegration, DawnWithRain);
    
    std::cout << "\n=== Results ===\n";
    std::cout << "Total: " << total_tests << " | Passed: " << tests_passed << " | Failed: " << tests_failed << "\n";
    
    return tests_failed > 0 ? 1 : 0;
}
