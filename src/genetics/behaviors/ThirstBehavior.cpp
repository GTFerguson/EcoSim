#include "genetics/behaviors/ThirstBehavior.hpp"
#include "genetics/organisms/Organism.hpp"
#include "genetics/components/HeterotrophyComponent.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/expression/PhenotypeUtils.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/core/OrganismConstants.hpp"
#include "world/world.hpp"
#include "world/tile.hpp"
#include <algorithm>
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
    if (!organism.heterotrophy()) return false;
    return getThirstLevel(organism) < getThirstThreshold(organism);
}

float ThirstBehavior::getPriority(const Organism& organism) const {
    float thirst = getThirstLevel(organism);
    float threshold = getThirstThreshold(organism);

    float priority = BASE_PRIORITY;
    if (thirst < threshold && threshold > 0.0f) {
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

    if (!ctx.world || !organism.heterotrophy()) {
        result.debugInfo = "No world or heterotrophy component";
        return result;
    }

    if (tryDrinkAdjacent(organism, ctx)) {
        result.completed = true;
        // Drinking is a net positive: report a negative cost so the
        // controller doesn't deduct energy from a creature that just
        // restored hydration.
        result.energyCost = 0.0f;
        result.debugInfo = "Drank water, hydration restored";
        return result;
    }

    int waterX = 0, waterY = 0;
    if (!findWaterTarget(organism, ctx, waterX, waterY)) {
        result.debugInfo = "No water found in range";
        return result;
    }

    // Step toward the water tile. Movement amount is bounded by both
    // distance-to-target and a per-tick speed derived from LOCOMOTION.
    float dx = static_cast<float>(waterX) + 0.5f - organism.getWorldX();
    float dy = static_cast<float>(waterY) + 0.5f - organism.getWorldY();
    float dist = std::sqrt(dx * dx + dy * dy);
    if (dist > 0.1f) {
        float speed = getTraitSafe(organism.getPhenotype(),
            UniversalGenes::LOCOMOTION, 0.3f);
        speed = std::max(0.1f, speed * 0.5f);
        float moveAmount = std::min(speed, dist);

        float newX = organism.getWorldX() + (dx / dist) * moveAmount;
        float newY = organism.getWorldY() + (dy / dist) * moveAmount;

        newX = std::max(0.0f, std::min(static_cast<float>(ctx.worldCols) - 0.01f, newX));
        newY = std::max(0.0f, std::min(static_cast<float>(ctx.worldRows) - 0.01f, newY));

        organism.setWorldPosition(newX, newY);
    }

    std::ostringstream ss;
    ss << "Moving toward water at (" << waterX << "," << waterY << ")";
    result.debugInfo = ss.str();
    return result;
}

float ThirstBehavior::getEnergyCost(const Organism& organism) const {
    return THIRST_ENERGY_COST;
}

float ThirstBehavior::getThirstLevel(const Organism& organism) const {
    if (!organism.heterotrophy()) return Constants::RESOURCE_LIMIT;
    return organism.heterotrophy()->thirst;
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
    int tx = static_cast<int>(organism.getWorldX());
    int ty = static_cast<int>(organism.getWorldY());

    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (isWaterTile(tx + dx, ty + dy, ctx)) {
                organism.heterotrophy()->thirst = Constants::RESOURCE_LIMIT;
                return true;
            }
        }
    }
    return false;
}

bool ThirstBehavior::findWaterTarget(const Organism& organism,
                                      const BehaviorContext& ctx,
                                      int& outX, int& outY) const {
    int orgX = static_cast<int>(organism.getWorldX());
    int orgY = static_cast<int>(organism.getWorldY());

    float sightRange = getTraitSafe(organism.getPhenotype(),
        UniversalGenes::SIGHT_RANGE, 5.0f);
    int maxRadius = std::max(2, static_cast<int>(sightRange));

    // Spiral outward ring-by-ring; the first hit is the nearest by
    // Chebyshev distance, which is good enough for a navigation hint.
    for (int radius = 2; radius <= maxRadius; ++radius) {
        for (int dx = -radius; dx <= radius; ++dx) {
            for (int dy : {-radius, radius}) {
                if (isWaterTile(orgX + dx, orgY + dy, ctx)) {
                    outX = orgX + dx;
                    outY = orgY + dy;
                    return true;
                }
            }
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
