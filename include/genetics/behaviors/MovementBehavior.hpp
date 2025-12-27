#pragma once

#include "genetics/behaviors/IBehavior.hpp"
#include <utility>

namespace EcoSim {
namespace Genetics {

class MovementBehavior : public IBehavior {
public:
    MovementBehavior() = default;
    ~MovementBehavior() override = default;
    
    std::string getId() const override;
    bool isApplicable(const IGeneticOrganism& organism,
                      const BehaviorContext& ctx) const override;
    float getPriority(const IGeneticOrganism& organism) const override;
    BehaviorResult execute(IGeneticOrganism& organism,
                          BehaviorContext& ctx) override;
    float getEnergyCost(const IGeneticOrganism& organism) const override;
    
    void setTarget(int targetX, int targetY);
    void clearTarget();
    bool hasTarget() const;
    std::pair<int, int> getTarget() const;

private:
    int targetX_ = -1;
    int targetY_ = -1;
    bool hasTarget_ = false;
    
    static constexpr float LOCOMOTION_THRESHOLD = 0.3f;
    static constexpr float BASE_MOVEMENT_COST = 0.01f;
    static constexpr float DIAGONAL_COST_MULTIPLIER = 1.414f;
    
    float getMovementSpeed(const IGeneticOrganism& organism) const;
    float calculateMovementCost(const IGeneticOrganism& organism, float distance) const;
    bool canMove(const IGeneticOrganism& organism) const;
    void updatePosition(IGeneticOrganism& organism, float newX, float newY);
};

} // namespace Genetics
} // namespace EcoSim
