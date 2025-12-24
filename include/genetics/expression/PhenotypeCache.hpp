#pragma once

#include <unordered_map>
#include <string>
#include <functional>

namespace EcoSim {
namespace Genetics {

// Forward declarations
struct EnvironmentState;
struct OrganismState;

// Manages phenotype caching (SRP - single concern)
class PhenotypeCache {
public:
    using ComputeFunc = std::function<float()>;
    
    PhenotypeCache() = default;
    
    // Get cached value or compute if not present/invalid
    float getOrCompute(const std::string& trait_id, ComputeFunc compute_func);
    
    // Invalidate specific trait
    void invalidate(const std::string& trait_id);
    
    // Invalidate all cached values
    void invalidateAll();
    
    // Check if cache should be invalidated based on state changes
    void checkInvalidation(const EnvironmentState& env, const OrganismState& org);
    
    // Get cache hit rate for diagnostics
    float getCacheHitRate() const;
    
private:
    struct CacheEntry {
        float value;
        bool valid = false;
    };
    
    std::unordered_map<std::string, CacheEntry> cache_;
    unsigned int cache_hits_ = 0;
    unsigned int cache_misses_ = 0;
    
    // Stored states for change detection
    float last_age_ = -1.0f;
    float last_temperature_ = -999.0f;
};

} // namespace Genetics
} // namespace EcoSim
