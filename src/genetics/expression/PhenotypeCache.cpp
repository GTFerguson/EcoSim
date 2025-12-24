#include "genetics/expression/PhenotypeCache.hpp"
#include "genetics/expression/EnvironmentState.hpp"
#include "genetics/expression/OrganismState.hpp"
#include <cmath>

namespace EcoSim {
namespace Genetics {

float PhenotypeCache::getOrCompute(const std::string& trait_id, ComputeFunc compute_func) {
    auto it = cache_.find(trait_id);
    
    if (it != cache_.end() && it->second.valid) {
        cache_hits_++;
        return it->second.value;
    }
    
    cache_misses_++;
    float value = compute_func();
    cache_[trait_id] = {value, true};
    return value;
}

void PhenotypeCache::invalidate(const std::string& trait_id) {
    auto it = cache_.find(trait_id);
    if (it != cache_.end()) {
        it->second.valid = false;
    }
}

void PhenotypeCache::invalidateAll() {
    for (auto& [key, entry] : cache_) {
        entry.valid = false;
    }
}

void PhenotypeCache::checkInvalidation(const EnvironmentState& env, const OrganismState& org) {
    // Invalidate if age changed significantly (expression changes with age)
    if (std::abs(org.age_normalized - last_age_) > 0.01f) {
        invalidateAll();
        last_age_ = org.age_normalized;
    }
    
    // Invalidate if temperature changed significantly
    if (std::abs(env.temperature - last_temperature_) > 5.0f) {
        invalidateAll();
        last_temperature_ = env.temperature;
    }
}

float PhenotypeCache::getCacheHitRate() const {
    unsigned int total = cache_hits_ + cache_misses_;
    if (total == 0) return 0.0f;
    return static_cast<float>(cache_hits_) / static_cast<float>(total);
}

} // namespace Genetics
} // namespace EcoSim
