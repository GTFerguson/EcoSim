#pragma once

namespace EcoSim {
namespace Genetics {

/**
 * Current high-level motivation — what the organism most wants to do.
 * Derived from active behavior selection and surfaced to UI/rendering
 * so panels can show "Hungry", "Tired", etc.
 */
enum class Motivation {
    Hungry,
    Thirsty,
    Amorous,
    Tired,
    Content
};

/**
 * Current action — what the organism is visibly doing this tick.
 * Finer-grained than Motivation; tells the UI/renderer how to animate
 * or describe the organism ("hunting", "chasing", "mating").
 */
enum class Action {
    Idle,
    Wandering,
    Searching,
    Navigating,
    Eating,
    Grazing,
    Hunting,
    Chasing,
    Attacking,
    Fleeing,
    Drinking,
    Courting,
    Mating,
    Resting
};

} // namespace Genetics
} // namespace EcoSim
