#pragma once

#include <random>

namespace EcoSim {
namespace Genetics {

/**
 * @brief Centralized thread-safe random number generation utility
 * 
 * Provides a thread-local random engine and common distribution helpers.
 * Each thread gets its own engine, seeded independently, ensuring thread safety
 * without synchronization overhead.
 * 
 * This utility eliminates DRY violations where thread-local RNG engines were
 * duplicated across Gene, GeneDefinition, and Chromosome classes.
 */
class RandomEngine {
public:
    /**
     * @brief Get the thread-local random engine
     * @return Reference to thread-local mt19937 engine
     */
    static std::mt19937& get() {
        thread_local static std::mt19937 engine(std::random_device{}());
        return engine;
    }
    
    /**
     * @brief Generate random float in range [min, max]
     * @param min Minimum value (inclusive)
     * @param max Maximum value (inclusive)
     * @return Random float in range
     */
    static float randomFloat(float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(get());
    }
    
    /**
     * @brief Generate random int in range [min, max]
     * @param min Minimum value (inclusive)
     * @param max Maximum value (inclusive)
     * @return Random int in range
     */
    static int randomInt(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(get());
    }
    
    /**
     * @brief Generate random float in range [0, 1]
     * @return Random probability value
     */
    static float randomProbability() {
        return randomFloat(0.0f, 1.0f);
    }
    
    /**
     * @brief Check if random roll succeeds against probability
     * @param probability Chance of success (0.0 to 1.0)
     * @return true if roll succeeds
     */
    static bool rollProbability(float probability) {
        return randomProbability() < probability;
    }
};

/**
 * @brief Convenience function to get thread-local RNG
 * 
 * Shorter alias for RandomEngine::get() for backward compatibility
 * and ease of migration from the old getRandomEngine() pattern.
 */
inline std::mt19937& getThreadLocalRNG() {
    return RandomEngine::get();
}

} // namespace Genetics
} // namespace EcoSim
