#pragma once

#include "genetics/core/GeneRegistry.hpp"
#include "genetics/core/Gene.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/defaults/UniversalGenes.hpp"

namespace EcoSim {
namespace Genetics {

/**
 * @brief Creates and registers default gene definitions (DEPRECATED)
 *
 * @deprecated Use UniversalGenes instead for new code.
 *
 * DefaultGenes provides the original 12 genes for backward compatibility.
 * UniversalGenes provides 67+ genes including all DefaultGenes functionality
 * plus advanced features for coevolution, plant interactions, and more.
 *
 * Migration Guide - Gene ID Mapping:
 * ----------------------------------
 * DefaultGenes::LIFESPAN         → UniversalGenes::LIFESPAN (same)
 * DefaultGenes::SIGHT            → UniversalGenes::SIGHT_RANGE
 * DefaultGenes::HUNGER_THRESHOLD → UniversalGenes::HUNGER_THRESHOLD (same)
 * DefaultGenes::THIRST_THRESHOLD → UniversalGenes::THIRST_THRESHOLD (same)
 * DefaultGenes::FATIGUE_THRESHOLD→ UniversalGenes::FATIGUE_THRESHOLD (same)
 * DefaultGenes::MATE_THRESHOLD   → UniversalGenes::MATE_THRESHOLD (same)
 * DefaultGenes::COMFORT_INC      → UniversalGenes::COMFORT_INCREASE (same)
 * DefaultGenes::COMFORT_DEC      → UniversalGenes::COMFORT_DECREASE (same)
 * DefaultGenes::DIET             → UniversalGenes::DIET_TYPE (deprecated - use emergent diet)
 * DefaultGenes::FLOCKS           → (no direct equivalent - use behavior genes)
 * DefaultGenes::FLEE_DISTANCE    → UniversalGenes::FLEE_THRESHOLD
 * DefaultGenes::PURSUE_DISTANCE  → UniversalGenes::PURSUE_THRESHOLD
 *
 * Recommended Replacement:
 * - Instead of DefaultGenes::registerDefaults(), use UniversalGenes::registerDefaults()
 * - Instead of DefaultGenes::createRandomGenome(), use UniversalGenes::createRandomGenome()
 * - Instead of DefaultGenes::createDefaultGenome(), use UniversalGenes::createCreatureGenome()
 */
class [[deprecated("Use UniversalGenes instead - see header for migration guide")]] DefaultGenes {
public:
    /**
     * @brief Populate a registry with default gene definitions
     * @param registry The registry to populate with gene definitions
     * @deprecated Forwards to UniversalGenes::registerDefaults() - use that directly
     */
    [[deprecated("Use UniversalGenes::registerDefaults() instead")]]
    static void registerDefaults(GeneRegistry& registry);
    
    /**
     * @brief Create a new Genome with default genes initialized to random values
     * @param registry The registry containing gene definitions
     * @return A new Genome with randomized gene values within limits
     * @deprecated Forwards to UniversalGenes::createRandomGenome() - use that directly
     */
    [[deprecated("Use UniversalGenes::createRandomGenome() instead")]]
    static Genome createRandomGenome(const GeneRegistry& registry);
    
    /**
     * @brief Create a Genome with specific default values (for testing)
     * @param registry The registry containing gene definitions
     * @return A new Genome with default values matching legacy defaults
     * @deprecated Use UniversalGenes::createCreatureGenome() instead
     */
    [[deprecated("Use UniversalGenes::createCreatureGenome() instead")]]
    static Genome createDefaultGenome(const GeneRegistry& registry);
    
    // Gene IDs for easy reference - matching legacy genome traits
    // DEPRECATED: Use UniversalGenes constants instead
    
    /// @deprecated Use UniversalGenes::LIFESPAN instead
    static constexpr const char* LIFESPAN = "lifespan";
    
    /// @deprecated Use UniversalGenes::SIGHT_RANGE instead (note: different ID!)
    static constexpr const char* SIGHT = "sight";
    
    /// @deprecated Use UniversalGenes::HUNGER_THRESHOLD instead
    static constexpr const char* HUNGER_THRESHOLD = "hunger_threshold";
    
    /// @deprecated Use UniversalGenes::THIRST_THRESHOLD instead
    static constexpr const char* THIRST_THRESHOLD = "thirst_threshold";
    
    /// @deprecated Use UniversalGenes::FATIGUE_THRESHOLD instead
    static constexpr const char* FATIGUE_THRESHOLD = "fatigue_threshold";
    
    /// @deprecated Use UniversalGenes::MATE_THRESHOLD instead
    static constexpr const char* MATE_THRESHOLD = "mate_threshold";
    
    /// @deprecated Use UniversalGenes::COMFORT_INCREASE instead
    static constexpr const char* COMFORT_INC = "comfort_increase";
    
    /// @deprecated Use UniversalGenes::COMFORT_DECREASE instead
    static constexpr const char* COMFORT_DEC = "comfort_decrease";
    
    /// @deprecated Use emergent diet calculation from digestion genes instead
    static constexpr const char* DIET = "diet";
    
    /// @deprecated No direct replacement - flocking behavior is now emergent
    static constexpr const char* FLOCKS = "flocks";
    
    /// @deprecated Use UniversalGenes::FLEE_THRESHOLD instead (note: different ID!)
    static constexpr const char* FLEE_DISTANCE = "flee_distance";
    
    /// @deprecated Use UniversalGenes::PURSUE_THRESHOLD instead (note: different ID!)
    static constexpr const char* PURSUE_DISTANCE = "pursue_distance";

private:
    // Default values from legacy Genome constructor
    static constexpr unsigned DEFAULT_LIFESPAN = 10000;
    static constexpr unsigned DEFAULT_SIGHT = 50;
    static constexpr float DEFAULT_HUNGER = 5.0f;
    static constexpr float DEFAULT_THIRST = 5.0f;
    static constexpr float DEFAULT_FATIGUE = 5.0f;
    static constexpr float DEFAULT_MATE = 3.0f;
    static constexpr float DEFAULT_COMFORT_INC = 0.004f;
    static constexpr float DEFAULT_COMFORT_DEC = 0.002f;
    static constexpr int DEFAULT_DIET = 1;  // Diet::apple
    static constexpr bool DEFAULT_FLOCKS = true;
    static constexpr unsigned DEFAULT_FLEE = 3;
    static constexpr unsigned DEFAULT_PURSUE = 6;
};

} // namespace Genetics
} // namespace EcoSim
