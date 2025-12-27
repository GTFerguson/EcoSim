/**
 * @file ArchetypeProfiler.hpp
 * @brief Per-archetype energy economics analysis
 * 
 * Profiles each creature archetype template to analyze their
 * energy economics including metabolism costs, reproduction ROI,
 * and diet-based energy acquisition.
 */

#pragma once

#include "BalanceFramework.hpp"
#include "AnalysisModule.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/organisms/CreatureFactory.hpp"
#include <memory>
#include <vector>
#include <string>
#include <map>

namespace EcoSim {
namespace Balance {

/**
 * @brief Analysis module for profiling archetype energy economics
 * 
 * Uses CreatureFactory to instantiate sample creatures from each template
 * and extracts phenotype values to calculate reproduction ROI and other
 * energy metrics.
 */
class ArchetypeProfiler : public AnalysisModule {
public:
    /**
     * @brief Construct with gene registry
     * @param registry Shared gene registry for creature creation
     */
    explicit ArchetypeProfiler(std::shared_ptr<Genetics::GeneRegistry> registry);
    
    /**
     * @brief Run the archetype analysis
     * @return true if analysis completed successfully
     */
    bool analyze() override;
    
    /**
     * @brief Get module name
     */
    std::string getName() const override { return "ArchetypeProfiler"; }
    
    /**
     * @brief Get formatted results text
     */
    std::string getResultsText() const override;
    
    /**
     * @brief Contribute results to the balance report
     */
    void contributeToReport(BalanceReport& report) const override;
    
    // --- Accessors ---
    
    /**
     * @brief Get all archetype profiles
     */
    const std::vector<ArchetypeProfile>& getProfiles() const { return profiles_; }
    
    /**
     * @brief Get profile for a specific archetype
     * @param name Archetype template name
     * @return Pointer to profile or nullptr if not found
     */
    const ArchetypeProfile* getProfile(const std::string& name) const;
    
    /**
     * @brief Get archetypes flagged as exploitable
     * @return Vector of archetype names with ROI > 1.0
     */
    std::vector<std::string> getExploitableArchetypes() const;

private:
    std::shared_ptr<Genetics::GeneRegistry> registry_;
    std::vector<ArchetypeProfile> profiles_;
    bool hasRun_ = false;
    
    /**
     * @brief Profile a single archetype template
     * @param templateName Name of the archetype template
     * @param factory CreatureFactory to create sample creatures
     * @return Filled ArchetypeProfile struct
     */
    ArchetypeProfile profileArchetype(const std::string& templateName,
                                       const Genetics::CreatureFactory& factory);
    
    /**
     * @brief Calculate reproduction ROI for an archetype
     * @param minSize Minimum offspring size
     * @param canCannibalize Whether the archetype can eat its young
     * @return ROI value (>1.0 indicates exploit potential)
     */
    float calculateReproductionROI(float minSize, bool canCannibalize) const;
    
    /**
     * @brief Calculate theoretical metabolism cost per tick
     * @param metabolismRate Metabolism rate gene value
     * @param bodySize Body size gene value
     * @return Calories consumed per tick
     */
    float calculateMetabolismCost(float metabolismRate, float bodySize) const;
};

} // namespace Balance
} // namespace EcoSim
