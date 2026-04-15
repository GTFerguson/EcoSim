#pragma once

namespace EcoSim {
namespace Genetics {

class ArchetypeIdentity;
class BiomeAdaptation;

/**
 * Classification identity for an organism — flyweight pointers into
 * the archetype and biome-adaptation catalogues plus a sequential
 * display ID that UI uses as "#42" in creature panels.
 *
 * Separate from Organism::id_ — that's the base-class numeric ID used
 * internally (targets, spatial indices, serialization keys). The
 * sequentialId here is the user-facing display number. They'll
 * consolidate once callers stop depending on the two being distinct.
 *
 * Flyweight pointers are non-owning. Population counts are maintained
 * on the ArchetypeIdentity / BiomeAdaptation singletons themselves.
 * The owner (currently Creature) increments/decrements population
 * counts in its constructor/destructor; this component just holds
 * the pointers.
 *
 * Visual character and name live on the GameObject base class and
 * are accessed via getChar() / getName() — not duplicated here.
 */
struct IdentityComponent {
    int sequentialId = 0;
    const ArchetypeIdentity* archetype = nullptr;
    const BiomeAdaptation*   biomeAdaptation = nullptr;
};

} // namespace Genetics
} // namespace EcoSim
