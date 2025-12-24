#pragma once

#include "genetics/core/Genome.hpp"
#include "genetics/core/GeneRegistry.hpp"

// Forward declaration of legacy type
// The old Genome class is in the global namespace at include/objects/creature/genome.hpp
class Genome;

namespace EcoSim {
namespace Genetics {

/**
 * @brief Converts between legacy and new genome formats
 * 
 * This adapter provides a compatibility layer between the legacy Genome class
 * (include/objects/creature/genome.hpp) and the new Genetics::Genome class.
 * 
 * The legacy genome has direct gene values, while the new genome uses a
 * diploid allele system with expression rules. During conversion:
 * - Legacy -> Modern: Values become homozygous genes (both alleles same)
 * - Modern -> Legacy: Expressed values (based on dominance) are extracted
 * 
 * Usage:
 * @code
 *   GeneRegistry registry;
 *   DefaultGenes::registerDefaults(registry);
 *   LegacyGenomeAdapter adapter(registry);
 *   
 *   // Convert legacy to modern
 *   ::Genome legacyGenome;
 *   Genetics::Genome modernGenome = adapter.fromLegacy(legacyGenome);
 *   
 *   // Convert modern to legacy
 *   ::Genome backToLegacy = adapter.toLegacy(modernGenome);
 * @endcode
 */
class LegacyGenomeAdapter {
public:
    /**
     * @brief Construct an adapter with a gene registry
     * @param registry The registry containing gene definitions for the new system
     */
    explicit LegacyGenomeAdapter(const GeneRegistry& registry);
    
    /**
     * @brief Convert legacy genome to new format
     * 
     * Creates a new Genetics::Genome with genes populated from the legacy genome.
     * Each legacy value becomes a homozygous gene (both alleles have the same value).
     * 
     * @param legacy The legacy Genome to convert
     * @return A new Genetics::Genome with equivalent gene values
     */
    Genetics::Genome fromLegacy(const ::Genome& legacy) const;
    
    /**
     * @brief Convert new genome to legacy format (for backward compatibility)
     * 
     * Creates a legacy Genome by extracting expressed values from the new genome.
     * Boolean and enum genes use their raw values; numeric genes use dominance
     * expression rules.
     * 
     * @param modern The Genetics::Genome to convert
     * @return A legacy ::Genome with equivalent trait values
     */
    ::Genome toLegacy(const Genetics::Genome& modern) const;
    
    /**
     * @brief Check if the last conversion was lossless
     * 
     * A conversion is lossless if:
     * - All genes from source format exist in target format
     * - No precision was lost during type conversions
     * - No genes were outside their valid limits
     * 
     * @return true if no information was lost in conversion
     */
    bool isLossless() const { return lossless_; }
    
    /**
     * @brief Get any warnings from the last conversion
     * @return Vector of warning messages
     */
    const std::vector<std::string>& getWarnings() const { return warnings_; }
    
private:
    const GeneRegistry& registry_;
    mutable bool lossless_ = true;
    mutable std::vector<std::string> warnings_;
    
    // Helper to create a homozygous gene from a single value
    Gene createHomozygousGene(const std::string& id, float value) const;
    Gene createHomozygousGene(const std::string& id, int value) const;
    Gene createHomozygousGene(const std::string& id, bool value) const;
    
    // Helper to extract expressed value from a gene
    float getExpressedFloat(const Genetics::Genome& genome, const std::string& id) const;
    int getExpressedInt(const Genetics::Genome& genome, const std::string& id) const;
    bool getExpressedBool(const Genetics::Genome& genome, const std::string& id) const;
    
    // Clear state for new conversion
    void resetConversionState() const;
};

} // namespace Genetics
} // namespace EcoSim
