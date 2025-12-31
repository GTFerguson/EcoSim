#pragma once

#include "genetics/behaviors/IBehavior.hpp"
#include <memory>
#include <functional>
#include <unordered_map>

namespace EcoSim {
namespace Genetics {

class PerceptionSystem;
class GeneRegistry;
class Phenotype;
class Genome;

using OffspringCallback = std::function<void(std::unique_ptr<IGeneticOrganism>)>;

/**
 * @brief Mating/breeding behavior for organisms
 *
 * Implements mate finding and reproduction:
 * 1. Maturity check - Organism must be mature enough to breed
 * 2. Resource check - Sufficient energy/hunger to afford breeding cost
 * 3. Mate detection - Find compatible mates via perception
 * 4. Fitness evaluation - Prefer similar but not too similar mates (avoid inbreeding)
 * 5. Offspring creation - Genetic crossover with mutation
 *
 * All methods use IGeneticOrganism& interface and phenotype traits.
 * NO type-specific code (no direct Creature references).
 *
 * @see docs/code-review/recommendations/creature-decomposition-plan.md
 */
class MatingBehavior : public IBehavior {
public:
    /**
     * @brief Construct mating behavior with required dependencies
     * @param perception Reference to perception system for mate detection
     * @param registry Reference to gene registry for offspring genome creation
     */
    MatingBehavior(PerceptionSystem& perception, const GeneRegistry& registry);
    ~MatingBehavior() override = default;
    
    /**
     * @brief Get behavior identifier
     * @return "mating"
     */
    std::string getId() const override;
    
    /**
     * @brief Check if mating is applicable for this organism
     *
     * Returns true when:
     * - Organism has mate value above MATE_THRESHOLD
     * - Organism has sufficient resources (hunger > MIN_HUNGER_TO_BREED)
     * - Organism is mature (age check via phenotype)
     *
     * @param organism The organism to check
     * @param ctx Current behavior context
     * @return true if the organism should consider mating
     */
    bool isApplicable(const IGeneticOrganism& organism,
                      const BehaviorContext& ctx) const override;
    
    /**
     * @brief Get mating priority based on mate value
     *
     * Base priority: NORMAL (50)
     * Increases with mate value
     *
     * @param organism The organism to evaluate
     * @return Priority value
     */
    float getPriority(const IGeneticOrganism& organism) const override;
    
    /**
     * @brief Execute mating for one tick
     *
     * Process:
     * 1. Find potential mate via perception system
     * 2. Check fitness compatibility
     * 3. If compatible and adjacent, create offspring
     * 4. Use callback to register offspring
     * 5. Deduct BREED_COST from both parents
     *
     * @param organism The mating organism (mutable for resource deduction)
     * @param ctx Behavior context with world access
     * @return Result indicating outcome
     */
    BehaviorResult execute(IGeneticOrganism& organism,
                          BehaviorContext& ctx) override;
    
    /**
     * @brief Get estimated energy cost for mating
     * @param organism The organism that would mate
     * @return Estimated energy units consumed (BREED_COST)
     */
    float getEnergyCost(const IGeneticOrganism& organism) const override;
    
    /**
     * @brief Set callback for offspring registration
     * @param callback Function called when offspring is created
     */
    void setOffspringCallback(OffspringCallback callback);

private:
    PerceptionSystem& perception_;
    const GeneRegistry& registry_;
    OffspringCallback offspringCallback_;
    
    static constexpr float MATE_THRESHOLD = 0.7f;
    static constexpr float BREED_COST = 3.0f;
    static constexpr float MIN_HUNGER_TO_BREED = 5.0f;
    static constexpr float MATURITY_AGE_RATIO = 0.1f;
    static constexpr float IDEAL_SIMILARITY = 0.8f;
    static constexpr float SIMILARITY_PENALTY_EXPONENT = 1.5f;
    static constexpr float BASE_PRIORITY = 50.0f;
    static constexpr float MAX_PRIORITY_BOOST = 25.0f;
    
    /**
     * @brief Check if organism is ready to mate based on mate value
     * @param organism The organism to check
     * @return true if mate value exceeds threshold
     */
    bool isReadyToMate(const IGeneticOrganism& organism) const;
    
    /**
     * @brief Get organism's mate value (breeding desire)
     * @param organism The organism to query
     * @return Current mate value
     */
    float getMateValue(const IGeneticOrganism& organism) const;
    
    /**
     * @brief Check fitness/compatibility between two potential mates
     *
     * Evaluates:
     * - Genetic similarity (prefer similar but not identical)
     * - Proximity weighting
     * - Inbreeding penalty for too-similar genomes
     *
     * @param seeker The organism looking for a mate
     * @param candidate The potential mate
     * @return Fitness score (0.0 to 1.5, higher is better)
     */
    float checkFitness(const IGeneticOrganism& seeker,
                       const IGeneticOrganism& candidate) const;
    
    /**
     * @brief Find a potential mate within perception range
     * @param seeker The organism looking for a mate
     * @param ctx Behavior context with world access
     * @return Pointer to suitable mate, or nullptr if none found
     */
    IGeneticOrganism* findMate(const IGeneticOrganism& seeker,
                               const BehaviorContext& ctx) const;
    
    /**
     * @brief Check if organism is mature enough to breed
     * @param organism The organism to check
     * @return true if age meets maturity threshold
     */
    bool isMature(const IGeneticOrganism& organism) const;
    
    /**
     * @brief Check if organism has sufficient resources to breed
     * @param organism The organism to check
     * @return true if hunger > MIN_HUNGER_TO_BREED
     */
    bool hasResourcesToBread(const IGeneticOrganism& organism) const;
    
    /**
     * @brief Calculate genetic similarity between two organisms
     * @param org1 First organism
     * @param org2 Second organism
     * @return Similarity score (0.0 to 1.0)
     */
    float calculateGeneticSimilarity(const IGeneticOrganism& org1,
                                     const IGeneticOrganism& org2) const;
    
    /**
     * @brief Calculate distance between two organisms
     * @param org1 First organism
     * @param org2 Second organism
     * @return Euclidean distance
     */
    float calculateDistance(const IGeneticOrganism& org1,
                            const IGeneticOrganism& org2) const;
    
    /**
     * @brief Create offspring from two parent organisms
     * @param parent1 First parent
     * @param parent2 Second parent
     * @return Unique pointer to new offspring genome
     */
    std::unique_ptr<Genome> createOffspringGenome(const IGeneticOrganism& parent1,
                                                   const IGeneticOrganism& parent2) const;
    
    /**
     * @brief Get organism's unique ID for tracking
     * @param organism The organism
     * @return Unique identifier
     */
    unsigned int getOrganismId(const IGeneticOrganism& organism) const;
};

} // namespace Genetics
} // namespace EcoSim
