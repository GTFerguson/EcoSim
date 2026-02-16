#pragma once

#include "genetics/behaviors/IBehavior.hpp"
#include "genetics/behaviors/BehaviorContext.hpp"
#include <string>

namespace EcoSim {
namespace Genetics {

class SeedDispersal;

/**
 * @brief Plant behavior for passive seed dispersal
 *
 * Determines whether a plant should attempt seed dispersal this tick.
 * Applicability depends on plant maturity, fruit/runner production
 * readiness, and a probabilistic dispersal chance.
 *
 * The actual offspring creation is handled by PlantManager after
 * it reads the behavior result — this behavior only makes the decision.
 */
class SeedDispersalBehavior : public IBehavior {
public:
    explicit SeedDispersalBehavior(SeedDispersal& seedDispersal);
    ~SeedDispersalBehavior() override = default;

    std::string getId() const override;
    bool isApplicable(const Organism& organism,
                      const BehaviorContext& ctx) const override;
    float getPriority(const Organism& organism) const override;
    BehaviorResult execute(Organism& organism,
                          BehaviorContext& ctx) override;
    float getEnergyCost(const Organism& organism) const override;

private:
    SeedDispersal& seedDispersal_;

    static constexpr float BASE_PRIORITY = 25.0f;
    static constexpr float DISPERSAL_ENERGY_COST = 0.01f;
};

} // namespace Genetics
} // namespace EcoSim
