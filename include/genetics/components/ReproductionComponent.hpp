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
    float reproductiveUrge = 0.0f;      // 0-1, accumulates toward mating readiness
    bool isPregnant = false;
    unsigned gestationTimer = 0;         // Ticks remaining until birth/laying
    int mateTargetId = -1;               // Currently pursued mate, -1 if none
    unsigned lastReproducedTick = 0;     // For cooldown enforcement
};

} // namespace Genetics
} // namespace EcoSim
