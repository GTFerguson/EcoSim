#pragma once

namespace EcoSim {
namespace Genetics {

/**
 * Runtime state for organisms capable of reproduction. Most organisms
 * carry this — sterility or pre-maturity is represented by state values,
 * not by component absence. Component absence would mean something truly
 * can't reproduce (e.g. an inert corpse, a sterile hybrid).
 *
 * Both sexual and asexual reproduction share this component; the mode
 * is determined at gene-expression time.
 */
struct ReproductionComponent {
    // Raw mating drive accumulator (Creature historically called this _mate).
    // Scale is implementation-defined but roughly 0-RESOURCE_LIMIT (~10) with
    // negative values representing discomfort. The normalised 0-1
    // "reproductive urge" is derived from this via getReproductiveUrge().
    float mate = 0.0f;
    bool isPregnant = false;
    unsigned gestationTimer = 0;         // Ticks remaining until birth/laying
    int mateTargetId = -1;               // Currently pursued mate, -1 if none
    unsigned lastReproducedTick = 0;     // For cooldown enforcement
};

} // namespace Genetics
} // namespace EcoSim
