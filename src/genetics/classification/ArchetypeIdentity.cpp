/**
 * @file ArchetypeIdentity.cpp
 * @brief Implementation of the ArchetypeIdentity flyweight class
 * 
 * Uses Meyer's Singleton pattern for thread-safe static initialization.
 * Each archetype is a lazily-initialized static local variable,
 * guaranteed thread-safe in C++11 and later.
 */

#include "genetics/classification/ArchetypeIdentity.hpp"

namespace EcoSim {
namespace Genetics {

// ============================================================================
// Constructor
// ============================================================================

ArchetypeIdentity::ArchetypeIdentity(std::string id,
                                     std::string label,
                                     char renderChar,
                                     EntityType entityType,
                                     std::string desc,
                                     bool passable)
    : id_(std::move(id))
    , label_(std::move(label))
    , renderChar_(renderChar)
    , entityType_(entityType)
    , desc_(std::move(desc))
    , passable_(passable)
    , population_(0)
{
}

// ============================================================================
// Meyer's Singleton Implementations - Thread-Safe Static Initialization
// ============================================================================

const ArchetypeIdentity* ArchetypeIdentity::ApexPredator() {
    static ArchetypeIdentity instance("apex_predator", "Apex Predator", 'A',
        EntityType::CREATURE, "Large, powerful, territorial dominant predator.", true);
    return &instance;
}

const ArchetypeIdentity* ArchetypeIdentity::PackHunter() {
    static ArchetypeIdentity instance("pack_hunter", "Pack Hunter", 'P',
        EntityType::CREATURE, "Small, fast, coordinated group hunter.", true);
    return &instance;
}

const ArchetypeIdentity* ArchetypeIdentity::AmbushPredator() {
    static ArchetypeIdentity instance("ambush_predator", "Ambush Predator", 'I',
        EntityType::CREATURE, "Patient, stealthy, high burst damage.", true);
    return &instance;
}

const ArchetypeIdentity* ArchetypeIdentity::PursuitHunter() {
    static ArchetypeIdentity instance("pursuit_hunter", "Pursuit Hunter", 'V',
        EntityType::CREATURE, "Fastest predator, speed-based chaser.", true);
    return &instance;
}

const ArchetypeIdentity* ArchetypeIdentity::TankHerbivore() {
    static ArchetypeIdentity instance("tank_herbivore", "Tank Herbivore", 'T',
        EntityType::CREATURE, "Large armored herbivore with horn defense.", true);
    return &instance;
}

const ArchetypeIdentity* ArchetypeIdentity::ArmoredGrazer() {
    static ArchetypeIdentity instance("armored_grazer", "Armored Grazer", 'K',
        EntityType::CREATURE, "Scaled herbivore with tail-club defense.", true);
    return &instance;
}

const ArchetypeIdentity* ArchetypeIdentity::FleetRunner() {
    static ArchetypeIdentity instance("fleet_runner", "Fleet Runner", 'R',
        EntityType::CREATURE, "Speed-based escape specialist, minimal combat.", true);
    return &instance;
}

const ArchetypeIdentity* ArchetypeIdentity::SpikyDefender() {
    static ArchetypeIdentity instance("spiky_defender", "Spiky Defender", 'S',
        EntityType::CREATURE, "Counter-attack specialist with body spines.", true);
    return &instance;
}

const ArchetypeIdentity* ArchetypeIdentity::Scavenger() {
    static ArchetypeIdentity instance("carrion_stalker", "Carrion Stalker", 'N',
        EntityType::CREATURE, "Corpse-feeding specialist with high toxin tolerance.", true);
    return &instance;
}

const ArchetypeIdentity* ArchetypeIdentity::OmnivoreGeneralist() {
    static ArchetypeIdentity instance("omnivore_generalist", "Omnivore Generalist", 'O',
        EntityType::CREATURE, "Adaptable mixed-diet generalist.", true);
    return &instance;
}

const ArchetypeIdentity* ArchetypeIdentity::CanopyForager() {
    static ArchetypeIdentity instance("canopy_forager", "Canopy Forager", 'C',
        EntityType::CREATURE, "Fruit-eating arboreal forager, good seed disperser.", true);
    return &instance;
}

// ----- Plant archetypes -----

const ArchetypeIdentity* ArchetypeIdentity::BerryBush() {
    static ArchetypeIdentity instance("berry_bush", "Berry Bush", 'B',
        EntityType::PLANT_BERRY_BUSH, "Fast-growing shrub that bears edible berries.", true);
    return &instance;
}

const ArchetypeIdentity* ArchetypeIdentity::OakTree() {
    static ArchetypeIdentity instance("oak_tree", "Oak Tree", 'T',
        EntityType::PLANT_OAK_TREE, "Long-lived tree with slow growth.", true);
    return &instance;
}

const ArchetypeIdentity* ArchetypeIdentity::Grass() {
    static ArchetypeIdentity instance("grass", "Grass", '"',
        EntityType::PLANT_GRASS, "Low ground cover, grazing-adapted.", true);
    return &instance;
}

const ArchetypeIdentity* ArchetypeIdentity::ThornBush() {
    static ArchetypeIdentity instance("thorn_bush", "Thorn Bush", '*',
        EntityType::PLANT_THORN_BUSH, "Defensive shrub covered in spines.", true);
    return &instance;
}

const ArchetypeIdentity* ArchetypeIdentity::GenericPlant() {
    static ArchetypeIdentity instance("plant_generic", "Plant", '.',
        EntityType::PLANT_GENERIC, "Unclassified plant.", true);
    return &instance;
}

} // namespace Genetics
} // namespace EcoSim
