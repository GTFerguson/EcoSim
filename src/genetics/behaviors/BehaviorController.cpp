#include "genetics/behaviors/BehaviorController.hpp"
#include <algorithm>
#include <sstream>

namespace EcoSim {
namespace Genetics {

void BehaviorController::addBehavior(std::unique_ptr<IBehavior> behavior) {
    if (behavior) {
        behaviors_.push_back(std::move(behavior));
    }
}

void BehaviorController::removeBehavior(const std::string& behaviorId) {
    behaviors_.erase(
        std::remove_if(behaviors_.begin(), behaviors_.end(),
            [&behaviorId](const std::unique_ptr<IBehavior>& b) {
                return b && b->getId() == behaviorId;
            }),
        behaviors_.end()
    );
    
    // Clear current behavior if it was removed
    if (currentBehaviorId_ == behaviorId) {
        currentBehaviorId_.clear();
    }
}

bool BehaviorController::hasBehavior(const std::string& behaviorId) const {
    return std::any_of(behaviors_.begin(), behaviors_.end(),
        [&behaviorId](const std::unique_ptr<IBehavior>& b) {
            return b && b->getId() == behaviorId;
        });
}

void BehaviorController::clearBehaviors() {
    behaviors_.clear();
    currentBehaviorId_.clear();
}

BehaviorResult BehaviorController::update(IGeneticOrganism& organism, BehaviorContext& ctx) {
    auto applicable = getApplicableBehaviors(organism, ctx);
    
    if (applicable.empty()) {
        currentBehaviorId_.clear();
        return BehaviorResult{false, false, 0.0f, "No applicable behaviors"};
    }
    
    // Sort by priority (highest first), stable to maintain insertion order for equal priorities
    std::stable_sort(applicable.begin(), applicable.end(),
        [&organism](const IBehavior* a, const IBehavior* b) {
            return a->getPriority(organism) > b->getPriority(organism);
        });
    
    // Execute highest priority behavior
    IBehavior* selected = applicable.front();
    currentBehaviorId_ = selected->getId();
    
    return selected->execute(organism, ctx);
}

const std::string& BehaviorController::getCurrentBehaviorId() const {
    return currentBehaviorId_;
}

std::size_t BehaviorController::getBehaviorCount() const {
    return behaviors_.size();
}

std::vector<std::string> BehaviorController::getBehaviorIds() const {
    std::vector<std::string> ids;
    ids.reserve(behaviors_.size());
    
    for (const auto& behavior : behaviors_) {
        if (behavior) {
            ids.push_back(behavior->getId());
        }
    }
    
    return ids;
}

std::string BehaviorController::getStatusString() const {
    std::ostringstream oss;
    oss << "BehaviorController: " << behaviors_.size() << " behaviors";
    
    if (!currentBehaviorId_.empty()) {
        oss << ", current: " << currentBehaviorId_;
    }
    
    return oss.str();
}

std::vector<IBehavior*> BehaviorController::getApplicableBehaviors(
    const IGeneticOrganism& organism,
    const BehaviorContext& ctx) const {
    
    std::vector<IBehavior*> applicable;
    
    for (const auto& behavior : behaviors_) {
        if (behavior && behavior->isApplicable(organism, ctx)) {
            applicable.push_back(behavior.get());
        }
    }
    
    return applicable;
}

} // namespace Genetics
} // namespace EcoSim
