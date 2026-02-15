#pragma once

#include "genetics/behaviors/IBehavior.hpp"
#include "genetics/behaviors/BehaviorContext.hpp"

namespace EcoSim {
namespace Genetics {

/**
 * @brief Behavior that drives organisms to seek and drink water.
 *
 * Mirrors the logic of the legacy thirstyProfile()/findWater() system:
 * - Checks adjacent tiles for water sources
 * - If adjacent, drinks and restores hydration to max
 * - If not adjacent, pathfinds toward nearest water via spiral search
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

    /// Check if a tile at (x,y) is a water source
    bool isWaterTile(int x, int y, const BehaviorContext& ctx) const;

    /// Search adjacent tiles for water, drink if found
    bool tryDrinkAdjacent(Organism& organism, BehaviorContext& ctx) const;

    /// Spiral search for nearest water tile within sight range
    bool findWaterTarget(const Organism& organism, const BehaviorContext& ctx,
                         int& outX, int& outY) const;

    static constexpr float DEFAULT_THIRST_THRESHOLD = 5.0f;
    static constexpr float BASE_PRIORITY = static_cast<float>(BehaviorPriority::NORMAL);
    static constexpr float MAX_PRIORITY_BOOST = 25.0f;
    static constexpr float THIRST_ENERGY_COST = 0.01f;
};

} // namespace Genetics
} // namespace EcoSim
