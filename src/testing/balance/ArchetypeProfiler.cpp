/**
 * @file ArchetypeProfiler.cpp
 * @brief Implementation of per-archetype energy economics analysis
 */

#include "ArchetypeProfiler.hpp"
#include "genetics/organisms/CreatureFactory.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "objects/creature/creature.hpp"
#include "world/Corpse.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace EcoSim {
namespace Balance {

// List of all archetype templates from CreatureFactory
static const std::vector<std::string> ARCHETYPE_TEMPLATES = {
    "apex_predator",
    "pack_hunter", 
    "ambush_predator",
    "pursuit_hunter",
    "tank_herbivore",
    "armored_grazer",
    "fleet_runner",
    "spiky_defender",
    "canopy_forager",
    "carrion_stalker",
    "omnivore_generalist"
};

ArchetypeProfiler::ArchetypeProfiler(std::shared_ptr<Genetics::GeneRegistry> registry)
    : registry_(std::move(registry))
{
}

bool ArchetypeProfiler::analyze() {
    profiles_.clear();
    
    // Create factory and register templates
    Genetics::CreatureFactory factory(registry_);
    factory.registerDefaultTemplates();
    
    for (const auto& templateName : ARCHETYPE_TEMPLATES) {
        ArchetypeProfile profile = profileArchetype(templateName, factory);
        profiles_.push_back(profile);
    }
    
    hasRun_ = true;
    return true;
}

ArchetypeProfile ArchetypeProfiler::profileArchetype(const std::string& templateName,
                                                      const Genetics::CreatureFactory& factory) {
    ArchetypeProfile profile;
    profile.archetypeName = templateName;
    
    // Check if template exists
    if (!factory.hasTemplate(templateName)) {
        profile.reproductionROI = -1.0f;  // Indicate error
        return profile;
    }
    
    // Create a sample creature from this template at position (0,0)
    Creature creature = factory.createFromTemplate(templateName, 0, 0);
    
    // Get phenotype from the creature
    const Genetics::Phenotype* phenotype = creature.getPhenotype();
    if (!phenotype) {
        profile.reproductionROI = -1.0f;  // Indicate error
        return profile;
    }
    
    // Extract key gene values
    profile.metabolismRate = phenotype->getTrait("metabolism_rate");
    profile.typicalSize = phenotype->getTrait("max_size");
    
    // Calculate minimum offspring size (typically 10-20% of adult size)
    float minOffspringSize = profile.typicalSize * 0.15f;
    if (minOffspringSize < 0.5f) minOffspringSize = 0.5f;  // Floor
    
    // Get diet characteristics
    float meatDigestion = phenotype->getTrait("meat_digestion_efficiency");
    float plantDigestion = phenotype->getTrait("plant_digestion_efficiency");
    (void)plantDigestion;  // Suppress unused warning
    
    // Determine trophic level based on diet
    auto dietType = phenotype->calculateDietType();
    profile.trophicLevel = classifyTrophicLevel(dietType);
    
    // Check if this archetype can cannibalize offspring
    // (carnivores and omnivores can eat meat)
    bool canEatMeat = (meatDigestion > 0.3f);  // Threshold for meaningful meat digestion
    
    // Calculate reproduction ROI
    profile.reproductionROI = calculateReproductionROI(minOffspringSize, canEatMeat);
    
    // Calculate metabolism cost per tick
    profile.energyPerTick = calculateMetabolismCost(profile.metabolismRate, profile.typicalSize);
    
    // Estimate lifetime consumption based on typical lifespan
    float lifespan = phenotype->getTrait("lifespan");
    if (lifespan <= 0) lifespan = 1000.0f;  // Default lifespan
    profile.lifetimeConsumption = profile.energyPerTick * lifespan;
    
    // Calculate corpse value for this archetype at typical size
    profile.corpseValue = profile.typicalSize * world::Corpse::NUTRITION_PER_SIZE;
    
    return profile;
}

float ArchetypeProfiler::calculateReproductionROI(float minSize, bool canCannibalize) const {
    if (!canCannibalize) {
        // Herbivores can't exploit their offspring
        return 0.0f;  // No ROI from cannibalism
    }
    
    // Total breeding cost (both parents)
    float breedCostTotal = Creature::BREED_COST * 2.0f;
    
    // Offspring corpse value
    float offspringCorpseValue = minSize * world::Corpse::NUTRITION_PER_SIZE;
    
    // ROI = value gained / cost invested
    return offspringCorpseValue / breedCostTotal;
}

float ArchetypeProfiler::calculateMetabolismCost(float metabolismRate, float bodySize) const {
    // Energy cost scales with both metabolism and size
    // Larger creatures need more energy, faster metabolism burns more
    return metabolismRate * bodySize * 0.1f;  // Scaling factor
}

const ArchetypeProfile* ArchetypeProfiler::getProfile(const std::string& name) const {
    for (const auto& p : profiles_) {
        if (p.archetypeName == name) {
            return &p;
        }
    }
    return nullptr;
}

std::vector<std::string> ArchetypeProfiler::getExploitableArchetypes() const {
    std::vector<std::string> result;
    for (const auto& p : profiles_) {
        if (p.reproductionROI > 1.0f) {
            result.push_back(p.archetypeName);
        }
    }
    return result;
}

std::string ArchetypeProfiler::getResultsText() const {
    if (!hasRun_) {
        return "Analysis has not been run yet.\n";
    }
    
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2);
    
    ss << separator(100, '=') << "\n";
    ss << "SECTION 2: PER-ARCHETYPE PROFILE\n";
    ss << separator(100) << "\n\n";
    
    // Header
    ss << padRight("Archetype", 20) << " | "
       << padRight("Trophic", 12) << " | "
       << padRight("Size", 6) << " | "
       << padRight("Metab", 6) << " | "
       << padRight("Cal/Tick", 8) << " | "
       << padRight("Corpse", 8) << " | "
       << padRight("ROI", 8) << " | "
       << "Status\n";
    ss << separator(100, '-') << "\n";
    
    // Sort by ROI for easier reading
    std::vector<ArchetypeProfile> sorted = profiles_;
    std::sort(sorted.begin(), sorted.end(), 
        [](const auto& a, const auto& b) { return a.reproductionROI > b.reproductionROI; });
    
    for (const auto& p : sorted) {
        std::string statusStr;
        if (p.reproductionROI < 0) {
            statusStr = "ERROR";
        } else if (p.reproductionROI > 1.0f) {
            statusStr = "EXPLOIT!";
        } else if (p.reproductionROI > 0.5f) {
            statusStr = "Warning";
        } else if (p.reproductionROI > 0) {
            statusStr = "OK";
        } else {
            statusStr = "N/A";  // Herbivore
        }
        
        std::string roiStr = (p.reproductionROI > 0) 
            ? formatFloat(p.reproductionROI, 1) + "x"
            : "N/A";
        
        ss << padRight(p.archetypeName, 20) << " | "
           << padRight(trophicLevelToString(p.trophicLevel), 12) << " | "
           << padRight(formatFloat(p.typicalSize, 1), 6) << " | "
           << padRight(formatFloat(p.metabolismRate, 2), 6) << " | "
           << padRight(formatFloat(p.energyPerTick, 2), 8) << " | "
           << padRight(formatFloat(p.corpseValue, 0), 8) << " | "
           << padRight(roiStr, 8) << " | "
           << statusStr << "\n";
    }
    
    // Summary
    auto exploitable = getExploitableArchetypes();
    ss << "\n";
    ss << "Archetypes with reproduction ROI > 1.0: " << exploitable.size() << "\n";
    if (!exploitable.empty()) {
        ss << "  Affected: ";
        for (size_t i = 0; i < exploitable.size(); ++i) {
            if (i > 0) ss << ", ";
            ss << exploitable[i];
        }
        ss << "\n";
    }
    
    // Key calculation explanation
    ss << "\n" << separator(100, '-') << "\n";
    ss << "KEY CALCULATION:\n";
    ss << "  Breeding Cost = 2 x " << Creature::BREED_COST << " = " 
       << (Creature::BREED_COST * 2.0f) << " calories (both parents)\n";
    ss << "  Offspring Corpse = size x " << world::Corpse::NUTRITION_PER_SIZE 
       << " calories\n";
    ss << "  ROI = Corpse Value / Breeding Cost\n";
    ss << "  ROI > 1.0 means infinite energy generation is possible!\n";
    
    return ss.str();
}

void ArchetypeProfiler::contributeToReport(BalanceReport& report) const {
    if (!hasRun_) return;
    
    report.archetypeProfiles = profiles_;
}

} // namespace Balance
} // namespace EcoSim
