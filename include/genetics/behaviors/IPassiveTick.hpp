#pragma once

namespace EcoSim {
namespace Genetics {

class Organism;
struct EnvironmentState;

/**
 * Interface for passive per-tick processes that run unconditionally —
 * no priority gating, no "is applicable" check, no decision-making.
 *
 * Distinct from IBehavior: IBehavior is for active decisions the
 * organism makes (hunt, mate, flee). IPassiveTick is for physiology —
 * photosynthesis, growth, aging, fatigue decay, metabolic burn.
 *
 * Passive ticks run in the order they were registered and each one
 * always runs (barring the organism being dead). BehaviorController
 * holds them alongside IBehavior instances.
 *
 * A passive tick may mutate organism state directly (it's not
 * subject to BehaviorResult indirection). It should not make
 * world mutations — confine those to IBehavior::execute().
 */
class IPassiveTick {
public:
    virtual ~IPassiveTick() = default;

    /**
     * @brief Run one tick of this passive process.
     * @param organism The organism whose state to update (mutable).
     * @param env      Current local environment state.
     */
    virtual void tick(Organism& organism, const EnvironmentState& env) = 0;
};

} // namespace Genetics
} // namespace EcoSim
