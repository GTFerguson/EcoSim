#pragma once

#include "genetics/behaviors/IBehavior.hpp"
#include "genetics/behaviors/BehaviorContext.hpp"

namespace EcoSim {
namespace Genetics {

/**
 * @brief Behavior that drives organisms to seek and drink water.
 *
 * Mirrors FeedingBehavior's structure: checks adjacency to a water source
 * (Tile::isSource), drinks if adjacent, otherwise spiral-searches outward
 * for the nearest water tile within sight range and steps toward it.
 *
 * Reads/writes hydration via the Heterotrophy component (organism->heterotrophy()->thirst).
 */
class ThirstBehavior : public IBehavior {
public:
    ThirstBehavior() = default;

    std::string getId() const override;
    bool isApplicable(const Organism& organism, const BehaviorContext& ctx) const override;
    float getPriority(const Organism& organism) const override;
    BehaviorResult execute(Organism& organism, BehaviorContext& ctx) override;
    float getEnergyCost(const Organism& organism) const override;

private:
    float getThirstLevel(const Organism& organism) const;
    float getThirstThreshold(const Organism& organism) const;

    bool isWaterTile(int x, int y, const BehaviorContext& ctx) const;
    bool tryDrinkAdjacent(Organism& organism, BehaviorContext& ctx) const;
    bool findWaterTarget(const Organism& organism, const BehaviorContext& ctx,
                         int& outX, int& outY) const;

    /// Default threshold for seeking water on the 0-RESOURCE_LIMIT scale.
    static constexpr float DEFAULT_THIRST_THRESHOLD = 5.0f;
    static constexpr float BASE_PRIORITY = 50.0f;
    static constexpr float MAX_PRIORITY_BOOST = 25.0f;
    static constexpr float THIRST_ENERGY_COST = 0.01f;
};

} // namespace Genetics
} // namespace EcoSim
