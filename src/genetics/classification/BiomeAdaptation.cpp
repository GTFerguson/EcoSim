/**
 * @file BiomeAdaptation.cpp
 * @brief Implementation of the BiomeAdaptation flyweight class
 * 
 * Uses Meyer's Singleton pattern for thread-safe static initialization.
 * Each biome adaptation is a lazily-initialized static local variable,
 * guaranteed thread-safe in C++11 and later.
 */

#include "genetics/classification/BiomeAdaptation.hpp"
#include "genetics/classification/ArchetypeIdentity.hpp"

#include <unordered_map>

namespace EcoSim {
namespace Genetics {

// ============================================================================
// Constructor
// ============================================================================

BiomeAdaptation::BiomeAdaptation(std::string id, std::string prefix, 
                                  std::string altPrefix, std::string latinModifier,
                                  BiomeAdaptationType biomeType)
    : id_(std::move(id))
    , prefix_(std::move(prefix))
    , altPrefix_(std::move(altPrefix))
    , latinModifier_(std::move(latinModifier))
    , biomeType_(biomeType)
    , population_(0)
{
}

// ============================================================================
// Meyer's Singleton Implementations - Thread-Safe Static Initialization
// ============================================================================

const BiomeAdaptation* BiomeAdaptation::Temperate() {
    static BiomeAdaptation instance("temperate", "", "", "", BiomeAdaptationType::Temperate);
    return &instance;
}

const BiomeAdaptation* BiomeAdaptation::Tundra() {
    static BiomeAdaptation instance("tundra", "Arctic", "Frost", "borealis", BiomeAdaptationType::Tundra);
    return &instance;
}

const BiomeAdaptation* BiomeAdaptation::Taiga() {
    static BiomeAdaptation instance("taiga", "Boreal", "Snow", "borealis", BiomeAdaptationType::Taiga);
    return &instance;
}

const BiomeAdaptation* BiomeAdaptation::Tropical() {
    static BiomeAdaptation instance("tropical", "Jungle", "", "tropicus", BiomeAdaptationType::Tropical);
    return &instance;
}

const BiomeAdaptation* BiomeAdaptation::Desert() {
    static BiomeAdaptation instance("desert", "Dune", "Sand", "deserti", BiomeAdaptationType::Desert);
    return &instance;
}

// ============================================================================
// Archetype Suffix Mapping
// ============================================================================

std::string BiomeAdaptation::getArchetypeSuffix(const ArchetypeIdentity* archetype) {
    if (!archetype) {
        return "Unknown";
    }
    
    // Systematic suffix mapping for scalable naming
    // Format: {Biome Prefix} + {Archetype Suffix} = Full Name
    // Example: "Arctic" + "Pack" = "Arctic Pack"
    static const std::unordered_map<std::string, std::string> suffixes = {
        {"apex_predator",       "Tyrant"},   // Dominance, apex position
        {"pack_hunter",         "Pack"},     // Social coordination
        {"ambush_predator",     "Lurker"},   // Stealth, patience
        {"pursuit_hunter",      "Chaser"},   // Speed pursuit
        {"tank_herbivore",      "Titan"},    // Massive size
        {"armored_grazer",      "Shell"},    // Defensive armor
        {"fleet_runner",        "Runner"},   // Speed escape
        {"spiky_defender",      "Spike"},    // Spine defense
        {"carrion_stalker",     "Scav"},     // Carrion feeding
        {"omnivore_generalist", "Forager"},  // Opportunistic
        {"canopy_forager",      "Canopy"}    // Arboreal specialist
    };
    
    auto it = suffixes.find(archetype->getId());
    if (it != suffixes.end()) {
        return it->second;
    }
    
    // Fallback: return the archetype label if no suffix defined
    return archetype->getLabel();
}

std::string BiomeAdaptation::getFullLabel(const ArchetypeIdentity* archetype) const {
    if (!archetype) {
        return "Unknown";
    }
    
    // For Temperate (baseline), return the original archetype label
    if (biomeType_ == BiomeAdaptationType::Temperate) {
        return archetype->getLabel();
    }
    
    // For other biomes, combine prefix + suffix
    // Example: "Arctic" + " " + "Pack" = "Arctic Pack"
    std::string suffix = getArchetypeSuffix(archetype);
    return prefix_ + " " + suffix;
}

} // namespace Genetics
} // namespace EcoSim
