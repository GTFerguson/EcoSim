#include "genetics/behaviors/MovementBehavior.hpp"
#include "genetics/behaviors/BehaviorContext.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/interfaces/IGeneticOrganism.hpp"
#include "genetics/interfaces/IPositionable.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/core/RandomEngine.hpp"
#include <cmath>
#include <sstream>

namespace EcoSim {
namespace Genetics {

namespace {
    float getTraitSafe(const Phenotype& phenotype, const std::string& traitName, float defaultValue) {
        if (phenotype.hasTrait(traitName)) {
            return phenotype.getTrait(traitName);
        }
        return defaultValue;
    }
}

std::string MovementBehavior::getId() const {
    return "movement";
}

bool MovementBehavior::isApplicable(const IGeneticOrganism& organism,
                                     const BehaviorContext& ctx) const {
    if (!canMove(organism)) {
        return false;
    }
    
    return hasTarget_ || true;
}

float MovementBehavior::getPriority(const IGeneticOrganism& organism) const {
    return static_cast<float>(BehaviorPriority::LOW);
}

BehaviorResult MovementBehavior::execute(IGeneticOrganism& organism,
                                          BehaviorContext& ctx) {
    BehaviorResult result;
    result.executed = true;
    result.completed = false;
    
    constexpr float ARRIVAL_THRESHOLD = 0.1f;
    
    float currentX = 0.0f;
    float currentY = 0.0f;
    
    IPositionable* positionable = dynamic_cast<IPositionable*>(&organism);
    if (positionable) {
        currentX = positionable->getWorldX();
        currentY = positionable->getWorldY();
    }
    
    float newX = currentX;
    float newY = currentY;
    float distance = 0.0f;
    
    if (hasTarget_) {
        float targetXf = static_cast<float>(targetX_) + 0.5f;
        float targetYf = static_cast<float>(targetY_) + 0.5f;
        
        float dx = targetXf - currentX;
        float dy = targetYf - currentY;
        
        float distToTarget = std::sqrt(dx * dx + dy * dy);
        
        if (distToTarget <= ARRIVAL_THRESHOLD) {
            result.completed = true;
            result.debugInfo = "Reached target";
            clearTarget();
            return result;
        }
        
        float speed = getMovementSpeed(organism);
        float moveAmount = std::min(speed, distToTarget);
        
        float normX = dx / distToTarget;
        float normY = dy / distToTarget;
        
        newX = currentX + normX * moveAmount;
        newY = currentY + normY * moveAmount;
        
        distance = moveAmount;
        
        if (std::abs(dx) > 0.1f && std::abs(dy) > 0.1f) {
            distance *= DIAGONAL_COST_MULTIPLIER;
        }
        
        std::ostringstream ss;
        ss << "Moving toward target (" << targetX_ << "," << targetY_ << ")";
        result.debugInfo = ss.str();
        
    } else {
        float wanderX = RandomEngine::randomFloat(-1.0f, 1.0f);
        float wanderY = RandomEngine::randomFloat(-1.0f, 1.0f);
        
        float wanderMag = std::sqrt(wanderX * wanderX + wanderY * wanderY);
        if (wanderMag < 0.1f) {
            result.debugInfo = "Staying in place";
            result.energyCost = 0.0f;
            return result;
        }
        
        wanderX /= wanderMag;
        wanderY /= wanderMag;
        
        float speed = getMovementSpeed(organism);
        newX = currentX + wanderX * speed;
        newY = currentY + wanderY * speed;
        
        distance = speed;
        if (std::abs(wanderX) > 0.1f && std::abs(wanderY) > 0.1f) {
            distance *= DIAGONAL_COST_MULTIPLIER;
        }
        
        result.debugInfo = "Wandering randomly";
    }
    
    if (ctx.worldRows > 0 && ctx.worldCols > 0) {
        newX = std::max(0.0f, std::min(static_cast<float>(ctx.worldCols) - 0.01f, newX));
        newY = std::max(0.0f, std::min(static_cast<float>(ctx.worldRows) - 0.01f, newY));
    }
    
    if (positionable && (newX != currentX || newY != currentY)) {
        positionable->setWorldPosition(newX, newY);
    }
    
    result.energyCost = calculateMovementCost(organism, distance);
    
    return result;
}

float MovementBehavior::getEnergyCost(const IGeneticOrganism& organism) const {
    return calculateMovementCost(organism, 1.0f);
}

void MovementBehavior::setTarget(int targetX, int targetY) {
    targetX_ = targetX;
    targetY_ = targetY;
    hasTarget_ = true;
}

void MovementBehavior::clearTarget() {
    targetX_ = -1;
    targetY_ = -1;
    hasTarget_ = false;
}

bool MovementBehavior::hasTarget() const {
    return hasTarget_;
}

std::pair<int, int> MovementBehavior::getTarget() const {
    return {targetX_, targetY_};
}

float MovementBehavior::getMovementSpeed(const IGeneticOrganism& organism) const {
    const Phenotype& phenotype = organism.getPhenotype();
    
    float locomotion = getTraitSafe(phenotype, UniversalGenes::LOCOMOTION, 0.5f);
    float bodyMass = getTraitSafe(phenotype, UniversalGenes::MAX_SIZE, 1.0f);
    
    bodyMass = 0.5f + (bodyMass / 20.0f) * 1.5f;
    float legLength = 0.3f + locomotion * 0.7f;
    
    constexpr float BASE_SPEED = 0.5f;
    constexpr float MIN_SPEED = 0.1f;
    
    float speed = (BASE_SPEED * locomotion * legLength) / std::sqrt(bodyMass);
    
    return std::max(MIN_SPEED, speed);
}

float MovementBehavior::calculateMovementCost(const IGeneticOrganism& organism, float distance) const {
    const Phenotype& phenotype = organism.getPhenotype();
    
    float metabolism = getTraitSafe(phenotype, UniversalGenes::METABOLISM_RATE, 0.5f);
    
    return BASE_MOVEMENT_COST * distance * metabolism;
}

bool MovementBehavior::canMove(const IGeneticOrganism& organism) const {
    const Phenotype& phenotype = organism.getPhenotype();
    float locomotion = getTraitSafe(phenotype, UniversalGenes::LOCOMOTION, 0.0f);
    
    return locomotion > LOCOMOTION_THRESHOLD;
}

void MovementBehavior::updatePosition(IGeneticOrganism& organism, float newX, float newY) {
    IPositionable* positionable = dynamic_cast<IPositionable*>(&organism);
    if (positionable) {
        positionable->setWorldPosition(newX, newY);
    }
}

} // namespace Genetics
} // namespace EcoSim
