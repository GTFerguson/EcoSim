#pragma once

#include "genetics/core/GeneRegistry.hpp"
#include "genetics/core/Genome.hpp"
#include <cmath>

namespace EcoSim {
namespace Genetics {

// Forward declaration to avoid circular dependency
class Phenotype;

/**
 * @brief Energy state tracking for an organism
 *
 * Tracks the current energy reserves and expenditure breakdown
 * for an organism using the Universal Energy Budget system.
 */
struct EnergyState {
    float currentEnergy = 100.0f;    ///< Current energy reserves
    float maxEnergy = 100.0f;        ///< Maximum storage capacity
    float baseMetabolism = 1.0f;     ///< Base energy burn per tick
    float maintenanceCost = 0.0f;    ///< Gene maintenance per tick
    float activityCost = 0.0f;       ///< Movement, sensing, etc.
    float totalExpenditure = 0.0f;   ///< Sum of all costs
    
    /**
     * @brief Get energy as a ratio (0.0 to 1.0)
     */
    float getEnergyRatio() const {
        return (maxEnergy > 0.0f) ? (currentEnergy / maxEnergy) : 0.0f;
    }
};

/**
 * @brief Energy budget calculation and management
 *
 * The EnergyBudget class implements the Universal Resource Allocation Framework
 * as described in resource-allocation.md. It calculates:
 *
 * - Total maintenance cost from genome (sum of all gene maintenance costs)
 * - Specialist efficiency bonus (dietary specialization rewards)
 * - Metabolic overhead (multi-system organisms pay extra)
 * - Energy state updates each tick
 *
 * This creates natural selection pressure for specialization without
 * arbitrary caps - organisms with too many active genes simply starve.
 *
 * Usage:
 * @code
 *   EnergyBudget budget;
 *
 *   // Calculate costs
 *   float maintenance = budget.calculateMaintenanceCost(genome, registry);
 *   float specialistBonus = budget.calculateSpecialistBonus(phenotype);
 *   float overhead = budget.calculateMetabolicOverhead(phenotype);
 *
 *   // Update energy state each tick
 *   EnergyState newState = budget.updateEnergy(currentState, income, activity);
 *
 *   // Check organism status
 *   if (budget.isStarving(newState)) { ... health declines ... }
 * @endcode
 */
class EnergyBudget {
public:
    /**
     * @brief Default constructor
     */
    EnergyBudget() = default;
    
    /**
     * @brief Calculate total maintenance cost from genome
     * @param genome The organism's genome
     * @param registry The gene registry with definitions
     * @return Total energy cost per tick for maintaining all expressed genes
     * 
     * Iterates through all genes in the genome and sums their individual
     * maintenance costs based on expression level and cost scaling.
     */
    float calculateMaintenanceCost(const Genome& genome, const GeneRegistry& registry) const;
    
    /**
     * @brief Calculate specialist efficiency bonus
     * @param phenotype The organism's phenotype
     * @return Multiplier for food extraction (1.0 = no bonus, up to 1.3 = 30% bonus)
     * 
     * Specialists who focus on one food type (high plant OR high meat digestion)
     * get up to 30% more energy from their preferred food source.
     * 
     * Formula: bonus = 1.0 + (|plantDigestion - meatDigestion| * 0.3)
     */
    float calculateSpecialistBonus(const Phenotype& phenotype) const;
    
    /**
     * @brief Calculate metabolic overhead for multi-system organisms
     * @param phenotype The organism's phenotype
     * @return Overhead multiplier (1.0 = no overhead, increases with active systems)
     * 
     * Organisms maintaining multiple digestive enzyme systems pay an overhead cost.
     * Each active system (>0.3 expression) adds 8% compounding overhead.
     * 
     * Systems counted:
     * - plant_digestion_efficiency
     * - meat_digestion_efficiency  
     * - cellulose_breakdown
     * - toxin_metabolism
     * 
     * Formula: overhead = pow(1.08, systemCount)
     */
    float calculateMetabolicOverhead(const Phenotype& phenotype) const;
    
    /**
     * @brief Update energy state for one tick
     * @param current Current energy state
     * @param income Energy gained this tick (from eating, photosynthesis, etc.)
     * @param activity Additional activity cost this tick (movement, hunting, etc.)
     * @return Updated energy state
     * 
     * Energy flow per tick:
     * Energy(t+1) = Energy(t) + Income - (BaseMetabolism + Maintenance + Activity)
     * 
     * If total expenditure exceeds available energy, currentEnergy goes to 0.
     */
    EnergyState updateEnergy(const EnergyState& current, float income, float activity) const;
    
    /**
     * @brief Check if organism is starving
     * @param state Current energy state
     * @return true if energy is critically low (<= 10% of max)
     * 
     * Starving organisms should have their health decline.
     */
    bool isStarving(const EnergyState& state) const;
    
    /**
     * @brief Check if organism can reproduce
     * @param state Current energy state
     * @param reproductionCost Energy cost to reproduce
     * @return true if organism has enough energy for reproduction
     * 
     * Organisms need at least reproductionCost + 2x maintenance as reserve.
     */
    bool canReproduce(const EnergyState& state, float reproductionCost) const;
    
    /**
     * @brief Get starvation threshold ratio
     * @return Energy ratio below which organism is considered starving
     */
    float getStarvationThreshold() const { return starvationThreshold_; }
    
    /**
     * @brief Set starvation threshold ratio
     * @param threshold Energy ratio (0.0-1.0) below which organism starves
     */
    void setStarvationThreshold(float threshold) { starvationThreshold_ = threshold; }

private:
    float starvationThreshold_ = 0.1f;  // 10% of max energy = starving
};

} // namespace Genetics
} // namespace EcoSim
