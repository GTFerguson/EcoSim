#include "genetics/behaviors/ThirstBehavior.hpp"
#include "genetics/organisms/Organism.hpp"
#include "genetics/interfaces/IPositionable.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/expression/PhenotypeUtils.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "world/world.hpp"
#include "world/tile.hpp"
#include <cmath>
#include <sstream>

namespace EcoSim {
namespace Genetics {

using PhenotypeUtils::getTraitSafe;

std::string ThirstBehavior::getId() const {
    return "thirst";
}

bool ThirstBehavior::isApplicable(const Organism& organism,
                                   const BehaviorContext& ctx) const {
    float thirst = getThirstLevel(organism);
    float threshold = getThirstThreshold(organism);
    return thirst < threshold;
}

float ThirstBehavior::getPriority(const Organism& organism) const {
    float thirst = getThirstLevel(organism);
    float threshold = getThirstThreshold(organism);

    float priority = BASE_PRIORITY;

    if (thirst < threshold) {
        float urgency = (threshold - thirst) / threshold;
        urgency = std::max(0.0f, std::min(1.0f, urgency));
        priority += urgency * MAX_PRIORITY_BOOST;
    }

    return priority;
}

BehaviorResult ThirstBehavior::execute(Organism& organism,
                                        BehaviorContext& ctx) {
    BehaviorResult result;
    result.executed = true;
    result.completed = false;
    result.energyCost = THIRST_ENERGY_COST;

    if (!ctx.world) {
        result.debugInfo = "No world access";
        return result;
    }

    // Check if we can drink from an adjacent water tile
    if (tryDrinkAdjacent(organism, ctx)) {
        result.completed = true;
        result.debugInfo = "Drank water, hydration restored";
        return result;
    }

    // Search for nearest water and signal movement toward it
    int waterX, waterY;
    if (findWaterTarget(organism, ctx, waterX, waterY)) {
        // Move organism toward water
        auto* pos = dynamic_cast<IPositionable*>(&organism);
        if (pos) {
            float dx = static_cast<float>(waterX) + 0.5f - pos->getWorldX();
            float dy = static_cast<float>(waterY) + 0.5f - pos->getWorldY();
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist > 0.1f) {
                float speed = getTraitSafe(organism.getPhenotype(),
                    UniversalGenes::LOCOMOTION, 0.3f);
                speed = std::max(0.1f, speed * 0.5f);
                float moveAmount = std::min(speed, dist);

                float newX = pos->getWorldX() + (dx / dist) * moveAmount;
                float newY = pos->getWorldY() + (dy / dist) * moveAmount;

                // Clamp to world bounds
                newX = std::max(0.0f, std::min(static_cast<float>(ctx.worldCols) - 0.01f, newX));
                newY = std::max(0.0f, std::min(static_cast<float>(ctx.worldRows) - 0.01f, newY));

                pos->setWorldPosition(newX, newY);
            }
        }

        std::ostringstream ss;
        ss << "Moving toward water at (" << waterX << "," << waterY << ")";
        result.debugInfo = ss.str();
    } else {
        result.debugInfo = "No water found in range";
    }

    return result;
}

float ThirstBehavior::getEnergyCost(const Organism& organism) const {
    return THIRST_ENERGY_COST;
}

float ThirstBehavior::getThirstLevel(const Organism& organism) const {
    return organism.getNeeds().hydration;
}

float ThirstBehavior::getThirstThreshold(const Organism& organism) const {
    const Phenotype& phenotype = organism.getPhenotype();

    if (phenotype.hasTrait(UniversalGenes::THIRST_THRESHOLD)) {
        return phenotype.getTrait(UniversalGenes::THIRST_THRESHOLD);
    }

    return DEFAULT_THIRST_THRESHOLD;
}

bool ThirstBehavior::isWaterTile(int x, int y, const BehaviorContext& ctx) const {
    if (x < 0 || x >= ctx.worldCols || y < 0 || y >= ctx.worldRows) {
        return false;
    }
    return ctx.world->getGrid()[x][y].isSource();
}

bool ThirstBehavior::tryDrinkAdjacent(Organism& organism, BehaviorContext& ctx) const {
    const auto* pos = dynamic_cast<const IPositionable*>(&organism);
    if (!pos) return false;

    int tx = pos->getX();
    int ty = pos->getY();

    // Check current and 8 adjacent tiles
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (isWaterTile(tx + dx, ty + dy, ctx)) {
                // Drink: restore hydration to max
                organism.getNeeds().hydration = organism.getNeeds().maxHydration;
                return true;
            }
        }
    }

    return false;
}

bool ThirstBehavior::findWaterTarget(const Organism& organism,
                                      const BehaviorContext& ctx,
                                      int& outX, int& outY) const {
    const auto* pos = dynamic_cast<const IPositionable*>(&organism);
    if (!pos) return false;

    int orgX = pos->getX();
    int orgY = pos->getY();

    float sightRange = getTraitSafe(organism.getPhenotype(),
        UniversalGenes::SIGHT_RANGE, 5.0f);
    int maxRadius = static_cast<int>(sightRange);

    // Spiral search outward from current position
    for (int radius = 2; radius <= maxRadius; ++radius) {
        for (int dx = -radius; dx <= radius; ++dx) {
            // Top and bottom edges of the ring
            for (int dy : {-radius, radius}) {
                if (isWaterTile(orgX + dx, orgY + dy, ctx)) {
                    outX = orgX + dx;
                    outY = orgY + dy;
                    return true;
                }
            }
            // Left and right edges (excluding corners)
            if (std::abs(dx) == radius) {
                for (int dy = -radius + 1; dy <= radius - 1; ++dy) {
                    if (isWaterTile(orgX + dx, orgY + dy, ctx)) {
                        outX = orgX + dx;
                        outY = orgY + dy;
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

} // namespace Genetics
} // namespace EcoSim
