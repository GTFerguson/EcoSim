/**
 * @file LegacyGenomeAdapter.cpp
 * @brief Implementation of the legacy genome adapter for format conversion
 * 
 * This file provides conversion between the legacy Genome class and the new
 * Genetics::Genome class, enabling backward compatibility during migration.
 */

#include "genetics/migration/LegacyGenomeAdapter.hpp"
#include "genetics/defaults/DefaultGenes.hpp"
#include "objects/creature/genome.hpp"  // Legacy Genome class

#include <sstream>

namespace EcoSim {
namespace Genetics {

LegacyGenomeAdapter::LegacyGenomeAdapter(const GeneRegistry& registry)
    : registry_(registry) {
}

void LegacyGenomeAdapter::resetConversionState() const {
    lossless_ = true;
    warnings_.clear();
}

Gene LegacyGenomeAdapter::createHomozygousGene(const std::string& id, float value) const {
    Allele allele(GeneValue(value), 1.0f);
    return Gene(id, allele, allele);
}

Gene LegacyGenomeAdapter::createHomozygousGene(const std::string& id, int value) const {
    Allele allele(GeneValue(value), 1.0f);
    return Gene(id, allele, allele);
}

Gene LegacyGenomeAdapter::createHomozygousGene(const std::string& id, bool value) const {
    Allele allele(GeneValue(value), 1.0f);
    return Gene(id, allele, allele);
}

float LegacyGenomeAdapter::getExpressedFloat(const Genetics::Genome& genome, const std::string& id) const {
    if (!genome.hasGene(id)) {
        std::ostringstream msg;
        msg << "Gene '" << id << "' not found in genome";
        warnings_.push_back(msg.str());
        lossless_ = false;
        return 0.0f;
    }
    
    const Gene& gene = genome.getGene(id);
    auto defOpt = registry_.tryGetDefinition(id);
    DominanceType dominance = defOpt ? defOpt->get().getDominance() : DominanceType::Incomplete;
    
    return gene.getNumericValue(dominance);
}

int LegacyGenomeAdapter::getExpressedInt(const Genetics::Genome& genome, const std::string& id) const {
    return static_cast<int>(getExpressedFloat(genome, id));
}

bool LegacyGenomeAdapter::getExpressedBool(const Genetics::Genome& genome, const std::string& id) const {
    if (!genome.hasGene(id)) {
        std::ostringstream msg;
        msg << "Gene '" << id << "' not found in genome";
        warnings_.push_back(msg.str());
        lossless_ = false;
        return false;
    }
    
    const Gene& gene = genome.getGene(id);
    GeneValue value = gene.getExpressedValue(DominanceType::Complete);
    
    // Try to extract boolean from the variant
    if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value);
    } else if (std::holds_alternative<float>(value)) {
        return std::get<float>(value) >= 0.5f;
    } else if (std::holds_alternative<int>(value)) {
        return std::get<int>(value) != 0;
    }
    
    return false;
}

Genetics::Genome LegacyGenomeAdapter::fromLegacy(const ::Genome& legacy) const {
    resetConversionState();
    
    Genetics::Genome modern;
    
    // Convert each legacy gene to a homozygous gene in the new system
    // The values are stored as-is, making both alleles identical
    
    // Lifespan chromosome
    if (registry_.hasGene(DefaultGenes::LIFESPAN)) {
        Gene lifespan = createHomozygousGene(DefaultGenes::LIFESPAN, 
            static_cast<float>(legacy.getLifespan()));
        modern.addGene(lifespan, ChromosomeType::Lifespan);
    }
    
    // Sensory chromosome
    if (registry_.hasGene(DefaultGenes::SIGHT)) {
        Gene sight = createHomozygousGene(DefaultGenes::SIGHT,
            static_cast<float>(legacy.getSight()));
        modern.addGene(sight, ChromosomeType::Sensory);
    }
    
    // Metabolism chromosome
    if (registry_.hasGene(DefaultGenes::HUNGER_THRESHOLD)) {
        Gene hunger = createHomozygousGene(DefaultGenes::HUNGER_THRESHOLD,
            legacy.getTHunger());
        modern.addGene(hunger, ChromosomeType::Metabolism);
    }
    
    if (registry_.hasGene(DefaultGenes::THIRST_THRESHOLD)) {
        Gene thirst = createHomozygousGene(DefaultGenes::THIRST_THRESHOLD,
            legacy.getTThirst());
        modern.addGene(thirst, ChromosomeType::Metabolism);
    }
    
    if (registry_.hasGene(DefaultGenes::FATIGUE_THRESHOLD)) {
        Gene fatigue = createHomozygousGene(DefaultGenes::FATIGUE_THRESHOLD,
            legacy.getTFatigue());
        modern.addGene(fatigue, ChromosomeType::Metabolism);
    }
    
    if (registry_.hasGene(DefaultGenes::COMFORT_INC)) {
        Gene comfInc = createHomozygousGene(DefaultGenes::COMFORT_INC,
            legacy.getComfInc());
        modern.addGene(comfInc, ChromosomeType::Metabolism);
    }
    
    if (registry_.hasGene(DefaultGenes::COMFORT_DEC)) {
        Gene comfDec = createHomozygousGene(DefaultGenes::COMFORT_DEC,
            legacy.getComfDec());
        modern.addGene(comfDec, ChromosomeType::Metabolism);
    }
    
    if (registry_.hasGene(DefaultGenes::DIET)) {
        // Convert Diet enum to float for storage
        Gene diet = createHomozygousGene(DefaultGenes::DIET,
            static_cast<float>(static_cast<int>(legacy.getDiet())));
        modern.addGene(diet, ChromosomeType::Metabolism);
    }
    
    // Behavior chromosome
    if (registry_.hasGene(DefaultGenes::FLOCKS)) {
        Gene flocks = createHomozygousGene(DefaultGenes::FLOCKS,
            legacy.ifFlocks());
        modern.addGene(flocks, ChromosomeType::Behavior);
    }
    
    if (registry_.hasGene(DefaultGenes::FLEE_DISTANCE)) {
        Gene flee = createHomozygousGene(DefaultGenes::FLEE_DISTANCE,
            static_cast<float>(legacy.getFlee()));
        modern.addGene(flee, ChromosomeType::Behavior);
    }
    
    if (registry_.hasGene(DefaultGenes::PURSUE_DISTANCE)) {
        Gene pursue = createHomozygousGene(DefaultGenes::PURSUE_DISTANCE,
            static_cast<float>(legacy.getPursue()));
        modern.addGene(pursue, ChromosomeType::Behavior);
    }
    
    // Reproduction chromosome
    if (registry_.hasGene(DefaultGenes::MATE_THRESHOLD)) {
        Gene mate = createHomozygousGene(DefaultGenes::MATE_THRESHOLD,
            legacy.getTMate());
        modern.addGene(mate, ChromosomeType::Reproduction);
    }
    
    return modern;
}

::Genome LegacyGenomeAdapter::toLegacy(const Genetics::Genome& modern) const {
    resetConversionState();
    
    // Extract expressed values from the modern genome
    unsigned lifespan = static_cast<unsigned>(
        getExpressedFloat(modern, DefaultGenes::LIFESPAN));
    unsigned sight = static_cast<unsigned>(
        getExpressedFloat(modern, DefaultGenes::SIGHT));
    
    float hunger = getExpressedFloat(modern, DefaultGenes::HUNGER_THRESHOLD);
    float thirst = getExpressedFloat(modern, DefaultGenes::THIRST_THRESHOLD);
    float fatigue = getExpressedFloat(modern, DefaultGenes::FATIGUE_THRESHOLD);
    float mate = getExpressedFloat(modern, DefaultGenes::MATE_THRESHOLD);
    
    float comfInc = getExpressedFloat(modern, DefaultGenes::COMFORT_INC);
    float comfDec = getExpressedFloat(modern, DefaultGenes::COMFORT_DEC);
    
    // Diet is stored as float, convert back to enum
    int dietValue = getExpressedInt(modern, DefaultGenes::DIET);
    Diet diet = static_cast<Diet>(dietValue);
    
    bool flocks = getExpressedBool(modern, DefaultGenes::FLOCKS);
    
    unsigned flee = static_cast<unsigned>(
        getExpressedFloat(modern, DefaultGenes::FLEE_DISTANCE));
    unsigned pursue = static_cast<unsigned>(
        getExpressedFloat(modern, DefaultGenes::PURSUE_DISTANCE));
    
    // Create the legacy genome with extracted values
    return ::Genome(lifespan, hunger, thirst, fatigue, mate,
                    comfInc, comfDec, sight, diet, flocks, flee, pursue);
}

} // namespace Genetics
} // namespace EcoSim
