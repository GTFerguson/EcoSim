/**
 * @file ConstraintBuilders.hpp
 * @brief Concrete constraint builder implementations for LP optimization
 * 
 * This file provides factory classes for generating domain-specific constraints:
 * - BreedingConstraintBuilder: Breeding economics (ROI bounds, survivability)
 * - TrophicConstraintBuilder: Trophic efficiency (energy transfer between levels)
 * - SurvivalConstraintBuilder: Basic survival mechanics (hunting viability)
 * 
 * ## Usage
 * 
 * @code
 *   BalanceConstraintRegistry registry;
 *   
 *   // Register variables first
 *   registry.registerVariable({...});
 *   
 *   // Create and configure builders
 *   BreedingConstraintBuilder breeding;
 *   breeding.setROIBounds(0.40, 0.80);
 *   
 *   TrophicConstraintBuilder trophic;
 *   trophic.setEfficiencyBounds(0.20, 0.50);
 *   
 *   // Register builders
 *   registry.addBuilder(&breeding);
 *   registry.addBuilder(&trophic);
 *   
 *   // Build all constraints
 *   registry.buildAllConstraints();
 * @endcode
 * 
 * @see BalanceConstraints.hpp for ConstraintBuilder interface
 * @see plans/balance-constraint-solver.md for full architecture details
 */

#pragma once

#include "BalanceConstraints.hpp"

namespace EcoSim {
namespace Balance {

/**
 * @brief Builds constraints related to breeding economics
 * 
 * Generates constraints that ensure breeding is:
 * - Worth something (ROI ≥ minROI)
 * - Not exploitable (ROI ≤ maxROI)
 * - Survivable (cost doesn't kill parent)
 * 
 * ## Constraints Generated
 * 
 * - `breeding_roi_min`: Minimum breeding ROI (offspring value vs cost)
 * - `breeding_roi_max`: Maximum breeding ROI (prevents baby cannibalism)
 * - `breed_cost_survivable`: Upper bound on breeding cost
 * 
 * ## Required Variables
 * 
 * - `NUTRITION_PER_SIZE`: Calories per unit corpse size
 * - `BREED_COST`: Energy cost to breed
 * 
 * ## Formula
 * 
 * ROI = (min_offspring_size × NUTRITION) / (2 × BREED_COST)
 * 
 * The factor of 2 represents the two parents sharing the cost.
 */
class BreedingConstraintBuilder : public ConstraintBuilder {
public:
    /**
     * @brief Get unique category identifier
     * @return "breeding"
     */
    std::string category() const override { return "breeding"; }
    
    /**
     * @brief Get human-readable description
     * @return Description of breeding economics constraints
     */
    std::string description() const override {
        return "Breeding economics: ROI bounds and survivability";
    }
    
    /**
     * @brief Build breeding constraints from registry state
     * 
     * Generates constraints based on current configuration. Returns
     * empty vector if required variables (NUTRITION_PER_SIZE, BREED_COST)
     * are not registered.
     * 
     * @param registry Registry containing registered variables
     * @return Vector of breeding constraints
     */
    std::vector<BalanceConstraint> build(
        const BalanceConstraintRegistry& registry) const override;
    
    // =====================
    // Configuration
    // =====================
    
    /**
     * @brief Set minimum and maximum ROI bounds
     * 
     * ROI = offspring_value / breeding_cost
     * 
     * @param min Minimum ROI (e.g., 0.40 for 40%)
     * @param max Maximum ROI (e.g., 0.80 for 80%)
     */
    void setROIBounds(double min, double max) { 
        roiMin_ = min; 
        roiMax_ = max; 
    }
    
    /**
     * @brief Set minimum offspring size for ROI calculation
     * 
     * Typically derived from smallest viable creature archetype.
     * 
     * @param size Minimum offspring size (default: 0.5)
     */
    void setMinOffspringSize(double size) { minOffspringSize_ = size; }
    
    /**
     * @brief Set maximum survivable breeding cost
     * 
     * Parent must not die from breeding. This is the upper bound
     * on BREED_COST derived from viability analysis.
     * 
     * @param cost Maximum survivable cost (default: 5.9)
     */
    void setMaxSurvivableCost(double cost) { maxSurvivableCost_ = cost; }
    
    // =====================
    // Getters for testing
    // =====================
    
    double getROIMin() const { return roiMin_; }
    double getROIMax() const { return roiMax_; }
    double getMinOffspringSize() const { return minOffspringSize_; }
    double getMaxSurvivableCost() const { return maxSurvivableCost_; }
    
private:
    double roiMin_ = 0.40;           ///< Minimum ROI (40%)
    double roiMax_ = 0.80;           ///< Maximum ROI (80%)
    double minOffspringSize_ = 0.5;  ///< Minimum offspring size from archetype
    double maxSurvivableCost_ = 5.9; ///< Max cost parent can survive
};

/**
 * @brief Builds constraints related to trophic efficiency
 * 
 * Generates constraints that ensure energy transfer between
 * trophic levels is:
 * - High enough for predators to survive
 * - Low enough to be ecologically realistic
 * 
 * ## Constraints Generated
 * 
 * - `trophic_efficiency_min`: Minimum trophic efficiency
 * - `trophic_efficiency_max`: Maximum trophic efficiency
 * 
 * ## Required Variables
 * 
 * - `NUTRITION_PER_SIZE`: Calories per unit corpse size
 * 
 * ## Formula
 * 
 * Efficiency = baseEfficiency × (NUTRITION / baseNutrition)
 * 
 * The efficiency scales linearly with nutrition value.
 */
class TrophicConstraintBuilder : public ConstraintBuilder {
public:
    /**
     * @brief Get unique category identifier
     * @return "trophic"
     */
    std::string category() const override { return "trophic"; }
    
    /**
     * @brief Get human-readable description
     * @return Description of trophic efficiency constraints
     */
    std::string description() const override {
        return "Trophic efficiency: energy transfer between levels";
    }
    
    /**
     * @brief Build trophic constraints from registry state
     * 
     * Generates constraints based on current configuration. Returns
     * empty vector if required variable (NUTRITION_PER_SIZE) is not
     * registered.
     * 
     * @param registry Registry containing registered variables
     * @return Vector of trophic constraints
     */
    std::vector<BalanceConstraint> build(
        const BalanceConstraintRegistry& registry) const override;
    
    // =====================
    // Configuration
    // =====================
    
    /**
     * @brief Set minimum and maximum efficiency bounds
     * 
     * Ecological rule of thumb: 10-20% efficiency between levels.
     * Games often allow higher for playability.
     * 
     * @param min Minimum efficiency (e.g., 0.20 for 20%)
     * @param max Maximum efficiency (e.g., 0.50 for 50%)
     */
    void setEfficiencyBounds(double min, double max) {
        effMin_ = min;
        effMax_ = max;
    }
    
    /**
     * @brief Set base efficiency at base nutrition value
     * 
     * The current trophic efficiency with current parameters.
     * 
     * @param eff Base efficiency (default: 0.12 for 12%)
     */
    void setBaseEfficiency(double eff) { baseEfficiency_ = eff; }
    
    /**
     * @brief Set base nutrition value for efficiency calculation
     * 
     * The NUTRITION_PER_SIZE value that gives base efficiency.
     * 
     * @param n Base nutrition (default: 100.0)
     */
    void setBaseNutrition(double n) { baseNutrition_ = n; }
    
    // =====================
    // Getters for testing
    // =====================
    
    double getEffMin() const { return effMin_; }
    double getEffMax() const { return effMax_; }
    double getBaseEfficiency() const { return baseEfficiency_; }
    double getBaseNutrition() const { return baseNutrition_; }
    
private:
    double effMin_ = 0.20;           ///< Minimum efficiency (20%)
    double effMax_ = 0.50;           ///< Maximum efficiency (50%)
    double baseEfficiency_ = 0.12;   ///< Current efficiency (12%)
    double baseNutrition_ = 100.0;   ///< Current NUTRITION_PER_SIZE
};

/**
 * @brief Builds constraints related to creature survival
 * 
 * Generates constraints that ensure basic survival mechanics work:
 * - Hunting provides net positive energy
 * - (Future) Movement is affordable
 * - (Future) Metabolism doesn't cause instant death
 * 
 * ## Constraints Generated
 * 
 * - `hunt_worthwhile`: Hunting provides minimum net energy
 * 
 * ## Required Variables
 * 
 * - `NUTRITION_PER_SIZE`: Calories per unit corpse size
 * 
 * ## Notes
 * 
 * This builder is a placeholder for future survival mechanics.
 * Currently only generates a simple hunt value constraint.
 */
class SurvivalConstraintBuilder : public ConstraintBuilder {
public:
    /**
     * @brief Get unique category identifier
     * @return "survival"
     */
    std::string category() const override { return "survival"; }
    
    /**
     * @brief Get human-readable description
     * @return Description of survival mechanics constraints
     */
    std::string description() const override {
        return "Survival mechanics: creatures must be able to live";
    }
    
    /**
     * @brief Build survival constraints from registry state
     * 
     * Generates constraints based on current configuration. Returns
     * empty vector if required variable (NUTRITION_PER_SIZE) is not
     * registered.
     * 
     * @param registry Registry containing registered variables
     * @return Vector of survival constraints
     */
    std::vector<BalanceConstraint> build(
        const BalanceConstraintRegistry& registry) const override;
    
    // =====================
    // Configuration
    // =====================
    
    /**
     * @brief Set minimum value from a successful hunt
     * 
     * The minimum net energy gain from hunting prey. Below this,
     * hunting is not worth the effort.
     * 
     * @param value Minimum hunt value (default: 50.0)
     */
    void setMinHuntValue(double value) { minHuntValue_ = value; }
    
    // =====================
    // Getters for testing
    // =====================
    
    double getMinHuntValue() const { return minHuntValue_; }
    
private:
    double minHuntValue_ = 50.0;  ///< Minimum worthwhile hunt value
};

} // namespace Balance
} // namespace EcoSim
