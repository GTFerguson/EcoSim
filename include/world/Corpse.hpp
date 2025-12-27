#pragma once
#include <string>

namespace world {

class Corpse {
public:
    //============================================================================
    //  Public Constants (for balance analysis and external tools)
    //============================================================================
    static constexpr float NUTRITION_PER_SIZE = 50.0f;  // Size 2.0 = 100 calories (reduced from 100 to fix cannibalism exploit)
    static constexpr float DECAY_TIME_PER_SIZE = 1000.0f; // Size 2.0 = 2000 ticks
    
    Corpse(float x, float y, float size, const std::string& speciesName, float bodyCondition = 0.5f);
    
    // Position
    float getX() const { return _x; }
    float getY() const { return _y; }
    int getTileX() const { return static_cast<int>(_x); }
    int getTileY() const { return static_cast<int>(_y); }
    
    // Nutrition
    float getNutritionalValue() const;
    float extractNutrition(float amount);  // Returns actual amount extracted
    bool isExhausted() const { return _remainingNutrition <= 0; }
    
    // Decay
    void tick();  // Advance decay
    float getDecayProgress() const;  // 0.0 = fresh, 1.0 = gone
    bool isFullyDecayed() const { return _decayTimer >= _maxDecayTime; }
    
    // Toxicity from decay (for scavengers)
    float getToxicity() const;
    
    // Getters
    float getOriginalSize() const { return _originalSize; }
    const std::string& getSpeciesName() const { return _speciesName; }
    
private:
    float _x, _y;
    float _originalSize;
    std::string _speciesName;
    
    float _baseNutrition;         // Original nutritional value
    float _remainingNutrition;    // Current available nutrition
    float _decayTimer;            // Ticks since death
    float _maxDecayTime;          // Total decay time based on size
};

} // namespace world
