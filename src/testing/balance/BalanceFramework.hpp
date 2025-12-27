/**
 * @file BalanceFramework.hpp
 * @brief Core types and data structures for the balance analysis tool suite
 * 
 * This framework provides the foundation for detecting energy flow exploits
 * in EcoSim, such as the "baby cannibalism" problem where breeding cost
 * is far less than offspring corpse value.
 */

#pragma once

#include <string>
#include <vector>
#include <array>

namespace EcoSim {
namespace Balance {

/**
 * @brief Trophic level classification for ecosystem energy flow analysis
 */
enum class TrophicLevel {
    PRODUCER,      ///< Plants - primary production via photosynthesis
    PRIMARY,       ///< Herbivores - eat plants
    SECONDARY,     ///< Carnivores - eat herbivores
    TERTIARY,      ///< Apex predators - eat carnivores
    DECOMPOSER     ///< Scavengers - eat corpses
};

/**
 * @brief Energy flow direction types in the ecosystem
 */
enum class EnergyFlow {
    PRODUCTION,    ///< Photosynthesis/external input
    CONSUMPTION,   ///< Eating food sources
    METABOLISM,    ///< Maintenance costs
    REPRODUCTION,  ///< Breeding costs
    DEATH_TRANSFER ///< Energy to corpse
};

/**
 * @brief Represents a single energy transaction in the ecosystem
 */
struct EnergyTransaction {
    TrophicLevel source;           ///< Where energy comes from
    TrophicLevel destination;      ///< Where energy goes to
    EnergyFlow flowType;           ///< Type of energy transfer
    float caloriesTransferred;     ///< Amount of energy transferred
    float efficiencyRatio;         ///< calories_out / calories_in
    std::string description;       ///< Human-readable description
};

/**
 * @brief Aggregate metrics for a single trophic level
 */
struct TrophicMetrics {
    TrophicLevel level;                      ///< Which trophic level
    float totalEnergyInput = 0.0f;           ///< Sum of energy entering this level
    float totalEnergyOutput = 0.0f;          ///< Sum of energy leaving this level
    float metabolicCost = 0.0f;              ///< Energy lost to metabolism
    float biomassStanding = 0.0f;            ///< Total creature mass at this level
    float transferEfficiency = 0.0f;         ///< Output / Input ratio
    int populationCount = 0;                 ///< Number of creatures at this level
    std::vector<std::string> archetypesPresent; ///< Archetypes at this level
};

/**
 * @brief Per-archetype economic profile for energy analysis
 */
struct ArchetypeProfile {
    std::string archetypeName;               ///< Template name (e.g., "pack_hunter")
    std::string displayName;                 ///< Human-readable name
    TrophicLevel trophicLevel = TrophicLevel::PRIMARY; ///< Primary trophic level
    
    // Energy economics
    float metabolismRate = 0.0f;             ///< Metabolism rate gene value
    float typicalSize = 0.0f;                ///< Max size from genes
    float energyPerTick = 0.0f;              ///< Calories/tick maintenance
    float lifetimeConsumption = 0.0f;        ///< Total energy over lifespan
    
    // Reproduction economics
    float breedingCost = 0.0f;               ///< Energy to produce offspring (both parents)
    float corpseValue = 0.0f;                ///< Corpse value at typical size
    float reproductionROI = 0.0f;            ///< Offspring value / breeding cost
    
    // Survival metrics
    float averageLifespan = 0.0f;            ///< Expected lifespan
    
    // Diet information
    std::string dietType;                    ///< CARNIVORE, HERBIVORE, etc.
    float meatDigestion = 0.0f;              ///< Meat digestion efficiency
    float plantDigestion = 0.0f;             ///< Plant digestion efficiency
};

/**
 * @brief Exploit severity levels
 */
enum class ExploitSeverity {
    INFO,          ///< Informational only, no action needed
    WARNING,       ///< Potential balance issue, monitor
    CRITICAL       ///< Game-breaking exploit, must fix
};

/**
 * @brief Structured balance recommendation with derivation
 *
 * Instead of hardcoded fix strings, provides formula-based recommendations
 * showing how target values were calculated from game constants.
 */
struct BalanceRecommendation {
    std::string parameterName;               ///< e.g., "BREED_COST"
    float currentValue = 0.0f;               ///< Current game constant value
    float targetValue = 0.0f;                ///< Recommended value
    float currentROI = 0.0f;                 ///< Current ROI before fix
    float expectedROI = 0.0f;                ///< Expected ROI after fix (offspring_value / breed_cost)
    std::string derivationFormula;           ///< How target was calculated
    std::string rationale;                   ///< Why this change helps (including feasibility)
};

/**
 * @brief Detected exploit report
 */
struct ExploitReport {
    std::string exploitName;                 ///< Short identifier
    std::string description;                 ///< Full description
    int severity = 0;                        ///< Severity level (0=INFO to 4=CRITICAL)
    std::vector<std::string> affectedArchetypes; ///< Which archetypes can exploit
    std::vector<BalanceRecommendation> recommendations; ///< Structured recommendations for this exploit
};

/**
 * @brief Archetype size range data extracted from templates
 */
struct ArchetypeSizeRange {
    std::string archetypeName;
    float minSize = 0.0f;
    float maxSize = 0.0f;
    float avgSize() const { return (minSize + maxSize) / 2.0f; }
};

/**
 * @brief Complete analysis results aggregating all modules
 */
struct BalanceReport {
    std::vector<TrophicMetrics> trophicLevels;        ///< Per-level metrics
    std::vector<ArchetypeProfile> archetypeProfiles;  ///< Per-archetype economics
    std::vector<ExploitReport> exploits;              ///< Detected issues
    std::vector<EnergyTransaction> sampleTransactions; ///< Example transactions
    std::vector<BalanceRecommendation> recommendations; ///< Structured fix suggestions
    std::vector<ArchetypeSizeRange> archetypeSizes;   ///< Size ranges per archetype
    
    // Summary statistics
    float systemEnergyEfficiency = 0.0f;     ///< Total output / total input
    float pyramidHealthScore = 0.0f;         ///< 0-100 rating of trophic structure
    bool isEcologicallyPlausible = false;    ///< Energy decreases up pyramid
    
    // Metadata
    std::string generatedAt;                 ///< Timestamp of report generation
    std::string version = "1.0.0";           ///< Report format version
};

/**
 * @brief Convert TrophicLevel enum to string
 */
inline std::string trophicLevelToString(TrophicLevel level) {
    switch (level) {
        case TrophicLevel::PRODUCER:   return "PRODUCER";
        case TrophicLevel::PRIMARY:    return "PRIMARY";
        case TrophicLevel::SECONDARY:  return "SECONDARY";
        case TrophicLevel::TERTIARY:   return "TERTIARY";
        case TrophicLevel::DECOMPOSER: return "DECOMPOSER";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Convert EnergyFlow enum to string
 */
inline std::string energyFlowToString(EnergyFlow flow) {
    switch (flow) {
        case EnergyFlow::PRODUCTION:    return "PRODUCTION";
        case EnergyFlow::CONSUMPTION:   return "CONSUMPTION";
        case EnergyFlow::METABOLISM:    return "METABOLISM";
        case EnergyFlow::REPRODUCTION:  return "REPRODUCTION";
        case EnergyFlow::DEATH_TRANSFER: return "DEATH_TRANSFER";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Convert ExploitSeverity enum to string
 */
inline std::string severityToString(ExploitSeverity severity) {
    switch (severity) {
        case ExploitSeverity::INFO:     return "INFO";
        case ExploitSeverity::WARNING:  return "WARNING";
        case ExploitSeverity::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

} // namespace Balance
} // namespace EcoSim
