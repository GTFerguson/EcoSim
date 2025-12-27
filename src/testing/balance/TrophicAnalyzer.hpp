/**
 * @file TrophicAnalyzer.hpp
 * @brief Header for trophic level energy flow analysis
 * 
 * Analyzes theoretical energy flow between trophic levels and validates
 * that the ecosystem maintains ecological plausibility (energy should
 * decrease at each higher trophic level).
 */

#pragma once

#include "AnalysisModule.hpp"
#include "BalanceFramework.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include <memory>
#include <map>

namespace EcoSim {
namespace Balance {

/**
 * @brief Analyzes energy flow between trophic levels
 * 
 * This module calculates theoretical energy transfer efficiencies
 * and validates that the ecosystem follows ecological principles
 * (typically 5-15% efficiency per trophic transfer).
 */
class TrophicAnalyzer : public AnalysisModule {
public:
    /**
     * @brief Construct with gene registry
     * @param registry Shared gene registry for phenotype calculations
     */
    explicit TrophicAnalyzer(std::shared_ptr<Genetics::GeneRegistry> registry);
    
    // AnalysisModule interface
    bool analyze() override;
    std::string getName() const override { return "Trophic Level Analyzer"; }
    std::string getResultsText() const override;
    void contributeToReport(BalanceReport& report) const override;
    
    // Accessors for results
    const std::vector<TrophicMetrics>& getMetrics() const { return metrics_; }
    float getPyramidHealthScore() const { return pyramidHealthScore_; }
    bool isEcologicallyPlausible() const { return isPlausible_; }
    
    /**
     * @brief Calculate theoretical transfer efficiency for a level
     * @param level The trophic level to analyze
     * @return Expected efficiency ratio (0.0-1.0)
     */
    float calculateTheoreticalEfficiency(TrophicLevel level) const;
    
    /**
     * @brief Get expected efficiency range for ecological validity
     * @return Pair of (min, max) efficiency values
     */
    static std::pair<float, float> getValidEfficiencyRange() {
        return {0.05f, 0.15f};  // 5-15% is ecologically realistic
    }

private:
    std::shared_ptr<Genetics::GeneRegistry> registry_;
    
    // Results
    std::vector<TrophicMetrics> metrics_;
    std::map<TrophicLevel, float> theoreticalEfficiencies_;
    float pyramidHealthScore_ = 0.0f;
    bool isPlausible_ = false;
    
    // Analysis helpers
    void initializeMetrics();
    void calculateEnergyFlows();
    void validatePyramid();
    float scorePyramidHealth() const;
    
    // Archetype data extraction helpers
    // These derive values from CreatureFactory templates instead of hardcoding
    float getAverageHerbivoreDigestion() const;
    float getAverageSecondaryPredatorOffspringSize() const;
    float getAverageApexPredatorOffspringSize() const;
    float getAverageDecomposerDigestion() const;
};

} // namespace Balance
} // namespace EcoSim
