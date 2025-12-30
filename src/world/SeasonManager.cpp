#include "world/SeasonManager.hpp"
#include <cmath>

namespace EcoSim {

void SeasonManager::tick() {
    _currentTick++;
    
    if (_currentTick >= _ticksPerDay) {
        _currentTick = 0;
        advanceDay();
    }
}

void SeasonManager::setTicksPerDay(int ticks) {
    if (ticks > 0) {
        _ticksPerDay = ticks;
    }
}

void SeasonManager::setDaysPerSeason(int days) {
    if (days > 0) {
        _daysPerSeason = days;
    }
}

Season SeasonManager::getCurrentSeason() const {
    return _currentSeason;
}

int SeasonManager::getCurrentDay() const {
    return _currentDay;
}

int SeasonManager::getCurrentYear() const {
    return _currentYear;
}

float SeasonManager::getDayProgress() const {
    if (_ticksPerDay <= 0) return 0.0f;
    return static_cast<float>(_currentTick) / static_cast<float>(_ticksPerDay);
}

float SeasonManager::getSeasonProgress() const {
    if (_daysPerSeason <= 0) return 0.0f;
    
    // Include tick progress within the current day for smoother transitions
    float dayFraction = getDayProgress();
    return (static_cast<float>(_currentDay) + dayFraction) / static_cast<float>(_daysPerSeason);
}

float SeasonManager::getYearProgress() const {
    // Four seasons per year
    constexpr int SEASONS_PER_YEAR = 4;
    
    int seasonIndex = static_cast<int>(_currentSeason);
    float seasonProgress = getSeasonProgress();
    
    return (static_cast<float>(seasonIndex) + seasonProgress) / static_cast<float>(SEASONS_PER_YEAR);
}

float SeasonManager::getBaseSolarIntensity() const {
    // Solar intensity varies smoothly through the year
    // Peaks in summer (yearProgress ~0.25), lowest in winter (yearProgress ~0.75)
    // Year starts in Spring (0.0), then Summer (0.25), Autumn (0.5), Winter (0.75)
    float yearProgress = getYearProgress();
    
    // Using sine to create smooth seasonal variation with peak at Summer
    // At Spring (0.0): sin(0) = 0 → 0.5
    // At Summer (0.25): sin(π/2) = 1 → 0.8 (peak)
    // At Autumn (0.5): sin(π) = 0 → 0.5
    // At Winter (0.75): sin(3π/2) = -1 → 0.2 (minimum)
    constexpr float BASE_INTENSITY = 0.5f;
    constexpr float VARIATION = 0.3f;
    constexpr float TWO_PI = 2.0f * 3.14159265358979f;
    
    return BASE_INTENSITY + VARIATION * std::sin(TWO_PI * yearProgress);
}

float SeasonManager::getBaseTemperatureModifier() const {
    // Temperature modifier in Celsius, varies by season
    // Summer: +10°C, Winter: -10°C, Spring/Autumn: ~0°C
    // Year starts in Spring (0.0), then Summer (0.25), Autumn (0.5), Winter (0.75)
    float yearProgress = getYearProgress();
    
    // Same sine approach as solar intensity for peak at Summer
    // At Spring (0.0): sin(0) = 0 → 0°C
    // At Summer (0.25): sin(π/2) = 1 → +10°C (warmest)
    // At Autumn (0.5): sin(π) = 0 → 0°C
    // At Winter (0.75): sin(3π/2) = -1 → -10°C (coldest)
    constexpr float MAX_VARIATION = 10.0f;  // ±10°C
    constexpr float TWO_PI = 2.0f * 3.14159265358979f;
    
    return MAX_VARIATION * std::sin(TWO_PI * yearProgress);
}

void SeasonManager::save(nlohmann::json& j) const {
    j["ticksPerDay"] = _ticksPerDay;
    j["daysPerSeason"] = _daysPerSeason;
    j["currentTick"] = _currentTick;
    j["currentDay"] = _currentDay;
    j["currentSeason"] = static_cast<int>(_currentSeason);
    j["currentYear"] = _currentYear;
}

void SeasonManager::load(const nlohmann::json& j) {
    _ticksPerDay = j.value("ticksPerDay", 100);
    _daysPerSeason = j.value("daysPerSeason", 30);
    _currentTick = j.value("currentTick", 0);
    _currentDay = j.value("currentDay", 0);
    _currentSeason = static_cast<Season>(j.value("currentSeason", 0));
    _currentYear = j.value("currentYear", 0);
}

void SeasonManager::advanceDay() {
    _currentDay++;
    
    if (_currentDay >= _daysPerSeason) {
        _currentDay = 0;
        advanceSeason();
    }
}

void SeasonManager::advanceSeason() {
    switch (_currentSeason) {
        case Season::Spring:
            _currentSeason = Season::Summer;
            break;
        case Season::Summer:
            _currentSeason = Season::Autumn;
            break;
        case Season::Autumn:
            _currentSeason = Season::Winter;
            break;
        case Season::Winter:
            _currentSeason = Season::Spring;
            _currentYear++;
            break;
    }
}

const char* seasonToString(Season season) {
    switch (season) {
        case Season::Spring: return "Spring";
        case Season::Summer: return "Summer";
        case Season::Autumn: return "Autumn";
        case Season::Winter: return "Winter";
        default: return "Unknown";
    }
}

} // namespace EcoSim
