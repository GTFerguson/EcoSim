/**
 * @file test_season_manager.cpp
 * @brief Unit tests for SeasonManager component
 */

#include "world/SeasonManager.hpp"
#include "../genetics/test_framework.hpp"
#include <cmath>

using namespace EcoSim;
using namespace EcoSim::Testing;

namespace {

//==============================================================================
// Test: Default State
//==============================================================================

void test_default_construction() {
    SeasonManager manager;
    
    TEST_ASSERT_EQ(manager.getCurrentSeason(), Season::Spring);
    TEST_ASSERT_EQ(manager.getCurrentDay(), 0);
    TEST_ASSERT_EQ(manager.getCurrentYear(), 0);
    TEST_ASSERT_EQ(manager.getCurrentTick(), 0);
}

void test_default_configuration() {
    SeasonManager manager;
    
    TEST_ASSERT_EQ(manager.getTicksPerDay(), 100);
    TEST_ASSERT_EQ(manager.getDaysPerSeason(), 30);
}

//==============================================================================
// Test: Time Progression
//==============================================================================

void test_tick_increments_tick_count() {
    SeasonManager manager;
    
    manager.tick();
    TEST_ASSERT_EQ(manager.getCurrentTick(), 1);
    
    manager.tick();
    TEST_ASSERT_EQ(manager.getCurrentTick(), 2);
}

void test_tick_wraps_to_new_day() {
    SeasonManager manager;
    manager.setTicksPerDay(10);  // Short days for testing
    
    // Tick 9 times - still day 0
    for (int i = 0; i < 9; ++i) {
        manager.tick();
    }
    TEST_ASSERT_EQ(manager.getCurrentDay(), 0);
    TEST_ASSERT_EQ(manager.getCurrentTick(), 9);
    
    // Tick once more - day should advance
    manager.tick();
    TEST_ASSERT_EQ(manager.getCurrentDay(), 1);
    TEST_ASSERT_EQ(manager.getCurrentTick(), 0);
}

void test_day_wraps_to_new_season() {
    SeasonManager manager;
    manager.setTicksPerDay(10);
    manager.setDaysPerSeason(5);
    
    // Advance through one full season (5 days * 10 ticks = 50 ticks)
    for (int i = 0; i < 50; ++i) {
        manager.tick();
    }
    
    TEST_ASSERT_EQ(manager.getCurrentSeason(), Season::Summer);
    TEST_ASSERT_EQ(manager.getCurrentDay(), 0);
}

void test_season_progression_order() {
    SeasonManager manager;
    manager.setTicksPerDay(1);
    manager.setDaysPerSeason(1);
    
    TEST_ASSERT_EQ(manager.getCurrentSeason(), Season::Spring);
    
    manager.tick();  // End of Spring day 0
    TEST_ASSERT_EQ(manager.getCurrentSeason(), Season::Summer);
    
    manager.tick();  // End of Summer day 0
    TEST_ASSERT_EQ(manager.getCurrentSeason(), Season::Autumn);
    
    manager.tick();  // End of Autumn day 0
    TEST_ASSERT_EQ(manager.getCurrentSeason(), Season::Winter);
    
    manager.tick();  // End of Winter day 0 - new year
    TEST_ASSERT_EQ(manager.getCurrentSeason(), Season::Spring);
}

void test_year_increments_after_winter() {
    SeasonManager manager;
    manager.setTicksPerDay(1);
    manager.setDaysPerSeason(1);
    
    TEST_ASSERT_EQ(manager.getCurrentYear(), 0);
    
    // Advance through 4 seasons (4 ticks with 1 tick/day, 1 day/season)
    for (int i = 0; i < 4; ++i) {
        manager.tick();
    }
    
    TEST_ASSERT_EQ(manager.getCurrentYear(), 1);
    TEST_ASSERT_EQ(manager.getCurrentSeason(), Season::Spring);
}

void test_multiple_years() {
    SeasonManager manager;
    manager.setTicksPerDay(1);
    manager.setDaysPerSeason(1);
    
    // Advance through 3 years (12 seasons)
    for (int i = 0; i < 12; ++i) {
        manager.tick();
    }
    
    TEST_ASSERT_EQ(manager.getCurrentYear(), 3);
    TEST_ASSERT_EQ(manager.getCurrentSeason(), Season::Spring);
}

//==============================================================================
// Test: Configuration
//==============================================================================

void test_setTicksPerDay() {
    SeasonManager manager;
    
    manager.setTicksPerDay(50);
    TEST_ASSERT_EQ(manager.getTicksPerDay(), 50);
}

void test_setTicksPerDay_ignores_invalid() {
    SeasonManager manager;
    
    manager.setTicksPerDay(0);
    TEST_ASSERT_EQ(manager.getTicksPerDay(), 100);  // Default value unchanged
    
    manager.setTicksPerDay(-5);
    TEST_ASSERT_EQ(manager.getTicksPerDay(), 100);  // Default value unchanged
}

void test_setDaysPerSeason() {
    SeasonManager manager;
    
    manager.setDaysPerSeason(15);
    TEST_ASSERT_EQ(manager.getDaysPerSeason(), 15);
}

void test_setDaysPerSeason_ignores_invalid() {
    SeasonManager manager;
    
    manager.setDaysPerSeason(0);
    TEST_ASSERT_EQ(manager.getDaysPerSeason(), 30);  // Default value unchanged
    
    manager.setDaysPerSeason(-10);
    TEST_ASSERT_EQ(manager.getDaysPerSeason(), 30);  // Default value unchanged
}

//==============================================================================
// Test: Progress Calculations
//==============================================================================

void test_getDayProgress_at_start() {
    SeasonManager manager;
    
    float progress = manager.getDayProgress();
    TEST_ASSERT(progress >= 0.0f && progress < 0.01f);
}

void test_getDayProgress_midday() {
    SeasonManager manager;
    manager.setTicksPerDay(100);
    
    for (int i = 0; i < 50; ++i) {
        manager.tick();
    }
    
    float progress = manager.getDayProgress();
    TEST_ASSERT(std::abs(progress - 0.5f) < 0.01f);
}

void test_getDayProgress_near_end() {
    SeasonManager manager;
    manager.setTicksPerDay(100);
    
    for (int i = 0; i < 99; ++i) {
        manager.tick();
    }
    
    float progress = manager.getDayProgress();
    TEST_ASSERT(progress >= 0.99f && progress < 1.0f);
}

void test_getSeasonProgress_at_start() {
    SeasonManager manager;
    
    float progress = manager.getSeasonProgress();
    TEST_ASSERT(progress >= 0.0f && progress < 0.01f);
}

void test_getSeasonProgress_midseason() {
    SeasonManager manager;
    manager.setTicksPerDay(10);
    manager.setDaysPerSeason(10);
    
    // Advance 5 days (50 ticks)
    for (int i = 0; i < 50; ++i) {
        manager.tick();
    }
    
    float progress = manager.getSeasonProgress();
    TEST_ASSERT(std::abs(progress - 0.5f) < 0.01f);
}

void test_getYearProgress_at_start() {
    SeasonManager manager;
    
    float progress = manager.getYearProgress();
    TEST_ASSERT(progress >= 0.0f && progress < 0.01f);
}

void test_getYearProgress_at_summer() {
    SeasonManager manager;
    manager.setTicksPerDay(1);
    manager.setDaysPerSeason(1);
    
    // Advance to Summer (1 season)
    manager.tick();
    
    float progress = manager.getYearProgress();
    // At start of Summer: 1/4 = 0.25
    TEST_ASSERT(std::abs(progress - 0.25f) < 0.01f);
}

void test_getYearProgress_at_autumn() {
    SeasonManager manager;
    manager.setTicksPerDay(1);
    manager.setDaysPerSeason(1);
    
    // Advance to Autumn (2 seasons)
    manager.tick();
    manager.tick();
    
    float progress = manager.getYearProgress();
    // At start of Autumn: 2/4 = 0.5
    TEST_ASSERT(std::abs(progress - 0.5f) < 0.01f);
}

void test_getYearProgress_at_winter() {
    SeasonManager manager;
    manager.setTicksPerDay(1);
    manager.setDaysPerSeason(1);
    
    // Advance to Winter (3 seasons)
    manager.tick();
    manager.tick();
    manager.tick();
    
    float progress = manager.getYearProgress();
    // At start of Winter: 3/4 = 0.75
    TEST_ASSERT(std::abs(progress - 0.75f) < 0.01f);
}

//==============================================================================
// Test: Seasonal Properties
//==============================================================================

void test_getBaseSolarIntensity_varies_by_season() {
    SeasonManager manager;
    manager.setTicksPerDay(1);
    manager.setDaysPerSeason(1);
    
    // Spring (year progress ~0.0)
    float springIntensity = manager.getBaseSolarIntensity();
    
    // Summer (year progress ~0.25)
    manager.tick();
    float summerIntensity = manager.getBaseSolarIntensity();
    
    // Autumn (year progress ~0.5)
    manager.tick();
    float autumnIntensity = manager.getBaseSolarIntensity();
    
    // Winter (year progress ~0.75)
    manager.tick();
    float winterIntensity = manager.getBaseSolarIntensity();
    
    // Summer should have highest intensity, winter lowest
    TEST_ASSERT(summerIntensity > springIntensity);
    TEST_ASSERT(summerIntensity > autumnIntensity);
    TEST_ASSERT(winterIntensity < springIntensity);
    TEST_ASSERT(winterIntensity < autumnIntensity);
}

void test_getBaseSolarIntensity_in_valid_range() {
    SeasonManager manager;
    manager.setTicksPerDay(1);
    manager.setDaysPerSeason(1);
    
    // Check all seasons
    for (int i = 0; i < 4; ++i) {
        float intensity = manager.getBaseSolarIntensity();
        TEST_ASSERT(intensity >= 0.0f && intensity <= 1.0f);
        manager.tick();
    }
}

void test_getBaseTemperatureModifier_varies_by_season() {
    SeasonManager manager;
    manager.setTicksPerDay(1);
    manager.setDaysPerSeason(1);
    
    // Spring (year progress ~0.0)
    float springMod = manager.getBaseTemperatureModifier();
    
    // Summer (year progress ~0.25)
    manager.tick();
    float summerMod = manager.getBaseTemperatureModifier();
    
    // Autumn (year progress ~0.5)
    manager.tick();
    float autumnMod = manager.getBaseTemperatureModifier();
    
    // Winter (year progress ~0.75)
    manager.tick();
    float winterMod = manager.getBaseTemperatureModifier();
    
    // Summer should be warmer, winter colder
    TEST_ASSERT(summerMod > springMod);
    TEST_ASSERT(summerMod > autumnMod);
    TEST_ASSERT(winterMod < springMod);
    TEST_ASSERT(winterMod < autumnMod);
}

void test_getBaseTemperatureModifier_reasonable_range() {
    SeasonManager manager;
    manager.setTicksPerDay(1);
    manager.setDaysPerSeason(1);
    
    // Check all seasons - modifier should be between -15 and +15
    for (int i = 0; i < 4; ++i) {
        float modifier = manager.getBaseTemperatureModifier();
        TEST_ASSERT(modifier >= -15.0f && modifier <= 15.0f);
        manager.tick();
    }
}

//==============================================================================
// Test: Serialization
//==============================================================================

void test_save_and_load() {
    SeasonManager original;
    original.setTicksPerDay(50);
    original.setDaysPerSeason(20);
    
    // Advance to a specific state
    for (int i = 0; i < 175; ++i) {  // Get to a non-trivial state
        original.tick();
    }
    
    // Save
    nlohmann::json j;
    original.save(j);
    
    // Load into new manager
    SeasonManager loaded;
    loaded.load(j);
    
    // Verify all state matches
    TEST_ASSERT_EQ(loaded.getTicksPerDay(), original.getTicksPerDay());
    TEST_ASSERT_EQ(loaded.getDaysPerSeason(), original.getDaysPerSeason());
    TEST_ASSERT_EQ(loaded.getCurrentTick(), original.getCurrentTick());
    TEST_ASSERT_EQ(loaded.getCurrentDay(), original.getCurrentDay());
    TEST_ASSERT_EQ(loaded.getCurrentSeason(), original.getCurrentSeason());
    TEST_ASSERT_EQ(loaded.getCurrentYear(), original.getCurrentYear());
}

void test_load_with_defaults() {
    SeasonManager manager;
    
    // Load from empty JSON (should use defaults)
    nlohmann::json j = nlohmann::json::object();
    manager.load(j);
    
    TEST_ASSERT_EQ(manager.getTicksPerDay(), 100);
    TEST_ASSERT_EQ(manager.getDaysPerSeason(), 30);
    TEST_ASSERT_EQ(manager.getCurrentTick(), 0);
    TEST_ASSERT_EQ(manager.getCurrentDay(), 0);
    TEST_ASSERT_EQ(manager.getCurrentSeason(), Season::Spring);
    TEST_ASSERT_EQ(manager.getCurrentYear(), 0);
}

//==============================================================================
// Test: Utility Functions
//==============================================================================

void test_seasonToString() {
    TEST_ASSERT_EQ(std::string(seasonToString(Season::Spring)), "Spring");
    TEST_ASSERT_EQ(std::string(seasonToString(Season::Summer)), "Summer");
    TEST_ASSERT_EQ(std::string(seasonToString(Season::Autumn)), "Autumn");
    TEST_ASSERT_EQ(std::string(seasonToString(Season::Winter)), "Winter");
}

} // anonymous namespace

//==============================================================================
// Test Runner
//==============================================================================

void runSeasonManagerTests() {
    BEGIN_TEST_GROUP("SeasonManager - Default State");
    RUN_TEST(test_default_construction);
    RUN_TEST(test_default_configuration);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("SeasonManager - Time Progression");
    RUN_TEST(test_tick_increments_tick_count);
    RUN_TEST(test_tick_wraps_to_new_day);
    RUN_TEST(test_day_wraps_to_new_season);
    RUN_TEST(test_season_progression_order);
    RUN_TEST(test_year_increments_after_winter);
    RUN_TEST(test_multiple_years);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("SeasonManager - Configuration");
    RUN_TEST(test_setTicksPerDay);
    RUN_TEST(test_setTicksPerDay_ignores_invalid);
    RUN_TEST(test_setDaysPerSeason);
    RUN_TEST(test_setDaysPerSeason_ignores_invalid);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("SeasonManager - Progress Calculations");
    RUN_TEST(test_getDayProgress_at_start);
    RUN_TEST(test_getDayProgress_midday);
    RUN_TEST(test_getDayProgress_near_end);
    RUN_TEST(test_getSeasonProgress_at_start);
    RUN_TEST(test_getSeasonProgress_midseason);
    RUN_TEST(test_getYearProgress_at_start);
    RUN_TEST(test_getYearProgress_at_summer);
    RUN_TEST(test_getYearProgress_at_autumn);
    RUN_TEST(test_getYearProgress_at_winter);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("SeasonManager - Seasonal Properties");
    RUN_TEST(test_getBaseSolarIntensity_varies_by_season);
    RUN_TEST(test_getBaseSolarIntensity_in_valid_range);
    RUN_TEST(test_getBaseTemperatureModifier_varies_by_season);
    RUN_TEST(test_getBaseTemperatureModifier_reasonable_range);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("SeasonManager - Serialization");
    RUN_TEST(test_save_and_load);
    RUN_TEST(test_load_with_defaults);
    END_TEST_GROUP();
    
    BEGIN_TEST_GROUP("SeasonManager - Utility Functions");
    RUN_TEST(test_seasonToString);
    END_TEST_GROUP();
}
