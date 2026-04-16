#pragma once

#include <string>

namespace EcoSim {
namespace Genetics {

class ArchetypeIdentity;
class BiomeAdaptation;

/**
 * Classification identity for an organism.
 *
 * Holds flyweight pointers into the archetype and biome-adaptation
 * catalogues plus a sequential display ID (UI uses it as "#42" in
 * creature panels) and a cached species name.
 *
 * The archetype pointer is the sole source for immutable display
 * metadata — character, entity type, label, passability, description.
 * Organism::getName() / getChar() / etc. read through this pointer.
 *
 * speciesName is the gene-derived scientific name produced by
 * Organism::generateName(). It is per-lineage, not flyweightable, and
 * cached once at construction to avoid regenerating it on every UI
 * refresh. When empty (e.g. plants that don't use species naming),
 * callers fall back to the archetype label.
 *
 * sequentialId is separate from Organism::id_ — that's the base-class
 * numeric ID used internally (targets, spatial indices, serialization
 * keys). The sequentialId here is the user-facing display number.
 * They'll consolidate once callers stop depending on the two being
 * distinct.
 *
 * Flyweight pointers are non-owning. Population counts are maintained
 * on the ArchetypeIdentity / BiomeAdaptation singletons themselves.
 * The owner increments/decrements population counts in its
 * constructor/destructor; this component just holds the pointers.
 */
struct IdentityComponent {
    int sequentialId = 0;
    const ArchetypeIdentity* archetype = nullptr;
    const BiomeAdaptation*   biomeAdaptation = nullptr;
    std::string speciesName;
};

} // namespace Genetics
} // namespace EcoSim
