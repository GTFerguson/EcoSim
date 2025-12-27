/**
 * @file ConstraintBuilders.cpp
 * @brief Implementation of constraint builder factories
 * 
 * @see ConstraintBuilders.hpp for class documentation
 */

#include "ConstraintBuilders.hpp"
#include <sstream>
#include <iomanip>

namespace EcoSim {
namespace Balance {

// ============================================================================
// BreedingConstraintBuilder Implementation
// ============================================================================

std::vector<BalanceConstraint> BreedingConstraintBuilder::build(
    const BalanceConstraintRegistry& registry) const
{
    std::vector<BalanceConstraint> constraints;
    
    // Check for required variables
    const auto* n = registry.getVariable("NUTRITION_PER_SIZE");
    const auto* b = registry.getVariable("BREED_COST");
    
    if (!n || !b) {
        // Required variables not registered - return empty list gracefully
        return constraints;
    }
    
    // Helper for formatting percentages
    auto pct = [](double v) { return static_cast<int>(v * 100); };
    
    // Helper for formatting doubles with precision
    auto fmt = [](double v, int precision = 2) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(precision) << v;
        return ss.str();
    };
    
    // -------------------------------------------------------------------------
    // Breeding ROI formula: (min_offspring_size × NUTRITION) / (2 × BREED_COST)
    // 
    // For ROI ≥ roiMin_: 
    //   min_size * n / (2 * b) >= roiMin_
    //   min_size * n >= roiMin_ * 2 * b
    //   min_size * n - 2 * roiMin_ * b >= 0
    //
    // For ROI ≤ roiMax_:
    //   min_size * n / (2 * b) <= roiMax_
    //   min_size * n <= roiMax_ * 2 * b
    //   min_size * n - 2 * roiMax_ * b <= 0
    // -------------------------------------------------------------------------
    
    // Constraint: breeding_roi_min
    // Ensures offspring provides minimum return on breeding investment
    {
        double coefN = minOffspringSize_;
        double coefB = -2.0 * roiMin_;
        
        std::string formula = fmt(minOffspringSize_) + "n - " + 
                              fmt(2.0 * roiMin_) + "b ≥ 0";
        
        constraints.push_back({
            "breeding_roi_min",
            "Breeding ROI ≥ " + std::to_string(pct(roiMin_)) + "%",
            "breeding",
            {{"NUTRITION_PER_SIZE", coefN}, {"BREED_COST", coefB}},
            BalanceConstraint::Relation::GEQ,
            0.0,
            formula,
            "Offspring must provide some return on breeding investment",
            {"breeding_roi_max"}  // Paired constraint
        });
    }
    
    // Constraint: breeding_roi_max
    // Prevents baby cannibalism exploit where breeding is too profitable
    {
        double coefN = minOffspringSize_;
        double coefB = -2.0 * roiMax_;
        
        std::string formula = fmt(minOffspringSize_) + "n - " + 
                              fmt(2.0 * roiMax_) + "b ≤ 0";
        
        constraints.push_back({
            "breeding_roi_max",
            "Breeding ROI ≤ " + std::to_string(pct(roiMax_)) + "%",
            "breeding",
            {{"NUTRITION_PER_SIZE", coefN}, {"BREED_COST", coefB}},
            BalanceConstraint::Relation::LEQ,
            0.0,
            formula,
            "Prevents baby cannibalism exploit",
            {"trophic_efficiency_min", "breeding_roi_min"}  // Known conflict
        });
    }
    
    // Constraint: breed_cost_survivable
    // Simple upper bound on BREED_COST - parent must survive breeding
    {
        std::string formula = "b ≤ " + fmt(maxSurvivableCost_);
        
        constraints.push_back({
            "breed_cost_survivable",
            "BREED_COST ≤ " + fmt(maxSurvivableCost_),
            "breeding",
            {{"BREED_COST", 1.0}},
            BalanceConstraint::Relation::LEQ,
            maxSurvivableCost_,
            formula,
            "Parent must be able to afford breeding without dying",
            {}  // No direct conflicts
        });
    }
    
    return constraints;
}

// ============================================================================
// TrophicConstraintBuilder Implementation
// ============================================================================

std::vector<BalanceConstraint> TrophicConstraintBuilder::build(
    const BalanceConstraintRegistry& registry) const
{
    std::vector<BalanceConstraint> constraints;
    
    // Check for required variables
    const auto* n = registry.getVariable("NUTRITION_PER_SIZE");
    
    if (!n) {
        // Required variable not registered - return empty list gracefully
        return constraints;
    }
    
    // Helper for formatting percentages
    auto pct = [](double v) { return static_cast<int>(v * 100); };
    
    // Helper for formatting doubles with precision
    auto fmt = [](double v, int precision = 2) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(precision) << v;
        return ss.str();
    };
    
    // -------------------------------------------------------------------------
    // Efficiency formula: baseEfficiency * (n / baseNutrition)
    // 
    // For efficiency ≥ effMin_:
    //   baseEff * (n / baseN) >= effMin_
    //   n >= (effMin_ / baseEff) * baseN
    //   n >= nMin
    //
    // For efficiency ≤ effMax_:
    //   baseEff * (n / baseN) <= effMax_
    //   n <= (effMax_ / baseEff) * baseN
    //   n <= nMax
    // -------------------------------------------------------------------------
    
    // Calculate bounds
    double nMin = (effMin_ / baseEfficiency_) * baseNutrition_;
    double nMax = (effMax_ / baseEfficiency_) * baseNutrition_;
    
    // Constraint: trophic_efficiency_min
    // Ensures predators can survive by hunting
    {
        std::string formula = "n ≥ " + fmt(nMin);
        
        constraints.push_back({
            "trophic_efficiency_min",
            "Trophic Efficiency ≥ " + std::to_string(pct(effMin_)) + "%",
            "trophic",
            {{"NUTRITION_PER_SIZE", 1.0}},
            BalanceConstraint::Relation::GEQ,
            nMin,
            formula,
            "Predators must survive on hunting",
            {"breeding_roi_max"}  // Known conflict with breeding
        });
    }
    
    // Constraint: trophic_efficiency_max
    // Ensures energy transfer is ecologically realistic
    {
        std::string formula = "n ≤ " + fmt(nMax);
        
        constraints.push_back({
            "trophic_efficiency_max",
            "Trophic Efficiency ≤ " + std::to_string(pct(effMax_)) + "%",
            "trophic",
            {{"NUTRITION_PER_SIZE", 1.0}},
            BalanceConstraint::Relation::LEQ,
            nMax,
            formula,
            "Energy transfer must be ecologically realistic",
            {}  // No direct conflicts
        });
    }
    
    return constraints;
}

// ============================================================================
// SurvivalConstraintBuilder Implementation
// ============================================================================

std::vector<BalanceConstraint> SurvivalConstraintBuilder::build(
    const BalanceConstraintRegistry& registry) const
{
    std::vector<BalanceConstraint> constraints;
    
    // Check for required variables
    const auto* n = registry.getVariable("NUTRITION_PER_SIZE");
    
    if (!n) {
        // Required variable not registered - return empty list gracefully
        return constraints;
    }
    
    // Helper for formatting doubles with precision
    auto fmt = [](double v, int precision = 2) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(precision) << v;
        return ss.str();
    };
    
    // -------------------------------------------------------------------------
    // Hunting must provide positive net energy
    // 
    // This is a placeholder constraint. In a more complete model, this would
    // factor in:
    // - Hunt success probability
    // - Energy cost to chase/attack
    // - Average prey size
    // 
    // For now, we just require NUTRITION_PER_SIZE >= minHuntValue_
    // -------------------------------------------------------------------------
    
    // Constraint: hunt_worthwhile
    // Simple lower bound ensuring hunting provides meaningful energy
    {
        std::string formula = "n ≥ " + fmt(minHuntValue_);
        
        constraints.push_back({
            "hunt_worthwhile",
            "Hunting provides net energy",
            "survival",
            {{"NUTRITION_PER_SIZE", 1.0}},
            BalanceConstraint::Relation::GEQ,
            minHuntValue_,
            formula,
            "Hunting must provide enough energy to be worthwhile (placeholder)",
            {"trophic_efficiency_max"}  // Conflicts if hunt value > trophic max
        });
    }
    
    // Future constraints to add:
    // - metabolism_positive: creatures can gain energy
    // - movement_affordable: can move to find food
    // - starvation_avoidable: metabolism doesn't cause instant death
    
    return constraints;
}

} // namespace Balance
} // namespace EcoSim
