#pragma once

namespace EcoSim {
namespace Genetics {

// 8-way compass heading. Lives here rather than in objects/creature
// so MobilityComponent can own it natively — creature.hpp imports this
// via a type alias for backward compatibility.
enum class Direction { N, E, S, W, NE, NW, SE, SW, none };

/**
 * Runtime state for mobile organisms.
 *
 * Present only when locomotion_speed > 0. Sessile organisms don't
 * allocate this component so they don't pay for float position or
 * movement cooldowns they can't use.
 */
struct MobilityComponent {
    float worldX = 0.0f;
    float worldY = 0.0f;
    Direction direction = Direction::none;
    unsigned speed = 1;
    int movementCooldown = 0;
};

} // namespace Genetics
} // namespace EcoSim
