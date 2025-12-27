#include "world/Corpse.hpp"
#include <algorithm>

namespace world {

Corpse::Corpse(float x, float y, float size, const std::string& speciesName, float bodyCondition)
    : _x(x)
    , _y(y)
    , _originalSize(size)
    , _speciesName(speciesName)
    , _decayTimer(0.0f)
{
    // Clamp bodyCondition to valid range
    bodyCondition = std::max(0.0f, std::min(1.0f, bodyCondition));
    
    // Base nutrition scales with creature size AND body condition
    // Well-fed creature (1.0 energy) gives 100% nutrition
    // Starved creature (0 energy) gives 50% nutrition
    float conditionMultiplier = 0.5f + bodyCondition * 0.5f;
    _baseNutrition = size * NUTRITION_PER_SIZE * conditionMultiplier;
    _remainingNutrition = _baseNutrition;
    _maxDecayTime = size * DECAY_TIME_PER_SIZE;
}

void Corpse::tick() {
    _decayTimer += 1.0f;
}

float Corpse::getDecayProgress() const {
    return std::min(1.0f, _decayTimer / _maxDecayTime);
}

float Corpse::getNutritionalValue() const {
    float decayProgress = getDecayProgress();
    // Decay reduces nutritional value:
    // At 0% decay: 100% nutrition
    // At 50% decay: ~55% nutrition
    // At 100% decay: 10% nutrition (minimum)
    float decayMultiplier = 1.0f - decayProgress * 0.9f;
    return _remainingNutrition * decayMultiplier;
}

float Corpse::extractNutrition(float amount) {
    // Can't extract more than available
    float available = std::min(amount, _remainingNutrition);
    
    // Apply decay penalty to extracted nutrition
    float decayProgress = getDecayProgress();
    float decayMultiplier = 1.0f - decayProgress * 0.9f;
    float actualNutrition = available * decayMultiplier;
    
    // Reduce remaining nutrition
    _remainingNutrition -= available;
    
    return actualNutrition;
}

float Corpse::getToxicity() const {
    // Rotting meat becomes increasingly toxic
    // Fresh: 0 toxicity
    // Fully decayed: 10 toxicity
    float decayProgress = getDecayProgress();
    return decayProgress * 10.0f;
}

} // namespace world
