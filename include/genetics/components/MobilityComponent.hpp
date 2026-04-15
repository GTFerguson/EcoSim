#pragma once

namespace EcoSim {
namespace Genetics {

/**
 * Runtime state for mobile organisms.
 *
 * Present only when locomotion_speed > 0. Sessile organisms don't
 * allocate this component so they don't pay for float position or
 * movement cooldowns they can't use.
 *
 * Direction is stored as an integer enum value at this layer to keep
 * the component header free of creature.hpp dependencies; the Direction
 * enum itself lives in objects/creature for now and will migrate into
 * genetics core when Creature is deleted.
 */
struct MobilityComponent {
    float worldX = 0.0f;
    float worldY = 0.0f;
    int direction = 8;          // matches Direction::none (8) as default
    unsigned speed = 1;
    int movementCooldown = 0;
};

} // namespace Genetics
} // namespace EcoSim
