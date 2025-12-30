#pragma once

#include <nlohmann/json.hpp>

namespace EcoSim {

enum class Season { Spring, Summer, Autumn, Winter };

/**
 * Manages time/calendar progression and seasonal state.
 * 
 * Provides a tick-based time system where:
 * - Multiple ticks make up a day
 * - Multiple days make up a season  
 * - Four seasons make up a year
 * 
 * Also calculates season-dependent properties like solar intensity
 * and base temperature modifiers for use by EnvironmentSystem.
 */
class SeasonManager {
public:
    SeasonManager() = default;
    
    // Time progression
    void tick();
    void setTicksPerDay(int ticks);
    void setDaysPerSeason(int days);
    
    // Current state queries
    Season getCurrentSeason() const;
    int getCurrentDay() const;
    int getCurrentYear() const;
    float getDayProgress() const;     // 0.0 to 1.0 within day
    float getSeasonProgress() const;  // 0.0 to 1.0 within season
    float getYearProgress() const;    // 0.0 to 1.0 within year
    
    // Season properties (for EnvironmentSystem to use)
    float getBaseSolarIntensity() const;        // Varies by season (0.0-1.0)
    float getBaseTemperatureModifier() const;   // Seasonal temp adjustment in Celsius
    
    // Serialization
    void save(nlohmann::json& j) const;
    void load(const nlohmann::json& j);
    
    // Configuration getters (for testing and serialization)
    int getTicksPerDay() const { return _ticksPerDay; }
    int getDaysPerSeason() const { return _daysPerSeason; }
    int getCurrentTick() const { return _currentTick; }
    
private:
    int _ticksPerDay = 100;
    int _daysPerSeason = 30;
    int _currentTick = 0;
    int _currentDay = 0;
    Season _currentSeason = Season::Spring;
    int _currentYear = 0;
    
    void advanceDay();
    void advanceSeason();
};

// Utility function for season name
const char* seasonToString(Season season);

} // namespace EcoSim
