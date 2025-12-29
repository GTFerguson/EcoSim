#include "genetics/core/GeneDefinition.hpp"
#include "genetics/core/RandomEngine.hpp"
#include <algorithm>

namespace EcoSim {
namespace Genetics {

GeneDefinition::GeneDefinition(const std::string& id,
                               ChromosomeType chromosome,
                               const GeneLimits& limits,
                               DominanceType dominance)
    : id_(id), chromosome_(chromosome), limits_(limits), dominance_(dominance) {
}

void GeneDefinition::addEffect(const EffectBinding& effect) {
    effects_.push_back(effect);
}

GeneValue GeneDefinition::createRandomValue() const {
    auto& rng = getThreadLocalRNG();
    std::uniform_real_distribution<float> dist(limits_.min_value, limits_.max_value);
    return GeneValue(dist(rng));
}

float GeneDefinition::clampValue(float value) const {
    return std::clamp(value, limits_.min_value, limits_.max_value);
}

float GeneDefinition::applyCreep(float value) const {
    auto& rng = getThreadLocalRNG();
    std::uniform_real_distribution<float> creep(-limits_.creep_amount, limits_.creep_amount);
    
    float result = value + creep(rng);
    return clampValue(result);
}

float GeneDefinition::calculateMaintenanceCost(float expressionLevel) const {
    float clampedExpression = std::clamp(expressionLevel, 0.0f, 1.0f);
    return maintenanceCost_ * std::pow(clampedExpression, costScaling_);
}

} // namespace Genetics
} // namespace EcoSim
