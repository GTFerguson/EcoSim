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

ArchetypeIdentity::ArchetypeIdentity(std::string id, std::string label, char renderChar)
    : id_(std::move(id))
    , label_(std::move(label))
    , renderChar_(renderChar)
    , population_(0)
{
}

// ============================================================================
// Meyer's Singleton Implementations - Thread-Safe Static Initialization
// ============================================================================

const ArchetypeIdentity* ArchetypeIdentity::ApexPredator() {
    static ArchetypeIdentity instance("apex_predator", "Apex Predator", 'A');
    return &instance;
}

const ArchetypeIdentity* ArchetypeIdentity::PackHunter() {
    static ArchetypeIdentity instance("pack_hunter", "Pack Hunter", 'P');
    return &instance;
}

const ArchetypeIdentity* ArchetypeIdentity::AmbushPredator() {
    static ArchetypeIdentity instance("ambush_predator", "Ambush Predator", 'I');
    return &instance;
}

const ArchetypeIdentity* ArchetypeIdentity::PursuitHunter() {
    static ArchetypeIdentity instance("pursuit_hunter", "Pursuit Hunter", 'V');
    return &instance;
}

const ArchetypeIdentity* ArchetypeIdentity::TankHerbivore() {
    static ArchetypeIdentity instance("tank_herbivore", "Tank Herbivore", 'T');
    return &instance;
}

const ArchetypeIdentity* ArchetypeIdentity::ArmoredGrazer() {
    static ArchetypeIdentity instance("armored_grazer", "Armored Grazer", 'K');
    return &instance;
}

const ArchetypeIdentity* ArchetypeIdentity::FleetRunner() {
    static ArchetypeIdentity instance("fleet_runner", "Fleet Runner", 'R');
    return &instance;
}

const ArchetypeIdentity* ArchetypeIdentity::SpikyDefender() {
    static ArchetypeIdentity instance("spiky_defender", "Spiky Defender", 'S');
    return &instance;
}

const ArchetypeIdentity* ArchetypeIdentity::Scavenger() {
    static ArchetypeIdentity instance("carrion_stalker", "Carrion Stalker", 'N');
    return &instance;
}

const ArchetypeIdentity* ArchetypeIdentity::OmnivoreGeneralist() {
    static ArchetypeIdentity instance("omnivore_generalist", "Omnivore Generalist", 'O');
    return &instance;
}

const ArchetypeIdentity* ArchetypeIdentity::CanopyForager() {
    static ArchetypeIdentity instance("canopy_forager", "Canopy Forager", 'C');
    return &instance;
}

} // namespace Genetics
} // namespace EcoSim
