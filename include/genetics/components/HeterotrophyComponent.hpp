#pragma once

#include <tuple>
#include <vector>

namespace EcoSim {
namespace Genetics {

/**
 * Runtime state for heterotrophic organisms (those that consume other
 * organisms for energy). Present when the organism's gene expression
 * includes hunger_threshold > 0 or any digestive genes.
 *
 * Hunger/thirst/fatigue live here rather than on a general metabolism
 * component because they are specifically heterotroph-side needs.
 * Autotrophs that photosynthesise don't get hungry in the same sense.
 */
struct HeterotrophyComponent {
    float hunger = 1.0f;
    float thirst = 1.0f;
    float fatigue = 0.0f;
    float metabolism = 0.001f;

    // Gut contents for zoochory (seed dispersal via digestion).
    // Each entry: (seedType, progress, locationTile)
    std::vector<std::tuple<int, float, int>> gutSeeds;

    // Burrs physically attached (not yet digested/dispersed).
    // Each entry: (burrType, attachTick, locationX, locationY)
    std::vector<std::tuple<int, int, int, int>> attachedBurrs;
};

} // namespace Genetics
} // namespace EcoSim
