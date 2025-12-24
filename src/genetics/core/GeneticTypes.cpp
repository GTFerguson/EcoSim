#include "genetics/core/GeneticTypes.hpp"

namespace EcoSim {
namespace Genetics {

const char* chromosomeTypeToString(ChromosomeType type) {
    switch (type) {
        case ChromosomeType::Morphology:    return "Morphology";
        case ChromosomeType::Sensory:       return "Sensory";
        case ChromosomeType::Metabolism:    return "Metabolism";
        case ChromosomeType::Locomotion:    return "Locomotion";
        case ChromosomeType::Behavior:      return "Behavior";
        case ChromosomeType::Reproduction:  return "Reproduction";
        case ChromosomeType::Environmental: return "Environmental";
        case ChromosomeType::Lifespan:      return "Lifespan";
        default:                            return "Unknown";
    }
}

std::optional<ChromosomeType> stringToChromosomeType(const std::string& str) {
    if (str == "Morphology")    return ChromosomeType::Morphology;
    if (str == "Sensory")       return ChromosomeType::Sensory;
    if (str == "Metabolism")    return ChromosomeType::Metabolism;
    if (str == "Locomotion")    return ChromosomeType::Locomotion;
    if (str == "Behavior")      return ChromosomeType::Behavior;
    if (str == "Reproduction")  return ChromosomeType::Reproduction;
    if (str == "Environmental") return ChromosomeType::Environmental;
    if (str == "Lifespan")      return ChromosomeType::Lifespan;
    return std::nullopt;
}

} // namespace Genetics
} // namespace EcoSim
