#pragma once

namespace EcoSim {
namespace Genetics {

/**
 * Runtime state for autotrophic organisms (those that generate energy
 * from light, soil nutrients, or chemosynthesis). Present when gene
 * expression includes photosynthesis_efficiency > 0 or root_depth > 0.
 *
 * Plants are the canonical autotrophs, but a future photosynthetic
 * creature would also carry this component.
 */
struct AutotrophyComponent {
    float photosynthesisReserve = 0.0f;     // Stored energy from light over recent ticks
    float chlorophyllActivity = 1.0f;       // 0-1, reflects current light/season modulation
    float rootDepthEffective = 0.0f;        // 0-1, actual absorption capacity
    float waterStored = 1.0f;               // Drought buffer
    float nutrientStored = 1.0f;            // Soil-absorbed nutrient buffer
};

} // namespace Genetics
} // namespace EcoSim
