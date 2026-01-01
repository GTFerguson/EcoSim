#ifndef CREATURE_PLANT_INTERACTION_HPP
#define CREATURE_PLANT_INTERACTION_HPP

/**
 * @file CreaturePlantInteraction.hpp
 * @brief Plant interaction functionality extracted from Creature class
 *
 * This module handles plant feeding, burr attachment/detachment, and zoochory
 * (seed dispersal via gut passage). Part of Phase 1c creature decomposition.
 */

#include <vector>

// Forward declarations
class Creature;

namespace EcoSim {
namespace Genetics {
class Plant;
class Phenotype;
class FeedingInteraction;
class SeedDispersal;
struct FeedingResult;
struct DispersalEvent;
enum class DispersalStrategy;
}
}

namespace CreaturePlantInteraction {

//============================================================================
//  Constants
//============================================================================

/**
 * @brief Viability for burr-dispersed seeds.
 *        Burrs protect seeds well during external transport.
 */
constexpr float BURR_SEED_VIABILITY = 0.85f;

/**
 * @brief Viability bonus for optimal gut transit time (4-12 hours).
 *        Scarification from stomach acid improves germination.
 */
constexpr float GUT_SEED_SCARIFICATION_BONUS = 1.15f;

/**
 * @brief Viability penalty for prolonged gut transit (>12 hours).
 *        Extended acid exposure damages seed coat.
 */
constexpr float GUT_SEED_ACID_DAMAGE = 0.9f;

/**
 * @brief Default gut transit time in hours for creatures without the gene.
 */
constexpr float DEFAULT_GUT_TRANSIT_HOURS = 6.0f;

/**
 * @brief Conversion factor: simulation ticks per hour.
 */
constexpr float TICKS_PER_HOUR = 10.0f;

/**
 * @brief Comfort boost from successful feeding.
 */
constexpr float FEEDING_MATE_BOOST = 2.0f;

/**
 * @brief Hunger cost per point of damage received from plant defenses.
 */
constexpr float DAMAGE_HUNGER_COST = 0.5f;

/**
 * @brief Comfort reduction multiplier while seeking food.
 */
constexpr float SEEKING_FOOD_MATE_PENALTY = 0.5f;

/**
 * @brief Detection range bonus from color vision (30%).
 */
constexpr float COLOR_VISION_RANGE_BONUS = 0.3f;

/**
 * @brief Detection range bonus from scent detection (50%).
 */
constexpr float SCENT_DETECTION_RANGE_BONUS = 0.5f;

//============================================================================
//  Plant Feeding
//============================================================================

/**
 * @brief Attempt to eat a plant using genetics-based feeding calculations.
 *
 * Calculates nutrition gained based on creature's digestion genes and
 * plant's nutritional content. Handles plant defense damage and seed
 * consumption for zoochory.
 *
 * @param creature The creature attempting to eat
 * @param plant The plant to eat (will be damaged)
 * @param feedingInteraction The shared feeding interaction calculator
 * @return FeedingResult with nutrition gained, damage taken, and seed info
 */
EcoSim::Genetics::FeedingResult eatPlant(
    Creature& creature,
    EcoSim::Genetics::Plant& plant,
    EcoSim::Genetics::FeedingInteraction& feedingInteraction);

/**
 * @brief Check if creature can eat the given plant.
 *
 * Evaluates detection (can creature find the plant?) and access
 * (can creature overcome plant defenses?).
 *
 * @param creature The creature attempting to eat
 * @param plant The plant to check
 * @param feedingInteraction The shared feeding interaction calculator
 * @return True if creature can detect and access the plant
 */
bool canEatPlant(
    const Creature& creature,
    const EcoSim::Genetics::Plant& plant,
    EcoSim::Genetics::FeedingInteraction& feedingInteraction);

/**
 * @brief Get the maximum range at which a creature can detect plants.
 *
 * Base range from sight, enhanced by color vision and scent detection genes.
 * Formula: baseRange * (1.0 + colorVision*0.3 + scentDetection*0.5)
 *
 * @param creature The creature to calculate range for
 * @return Detection range in world units
 */
float getPlantDetectionRange(const Creature& creature);

//============================================================================
//  Burr Management (External Seed Dispersal)
//============================================================================

/**
 * @brief Attach a burr from a plant to a creature's fur/hide.
 *
 * Burrs attach based on plant's hook strength vs creature's fur density.
 * Attached burrs track origin position and time for dispersal calculations.
 *
 * @param creature The creature receiving the burr
 * @param plant The plant providing the burr seeds
 * @param seedDispersal The shared seed dispersal calculator
 * @param attachedBurrs The creature's burr storage vector
 */
void attachBurr(
    Creature& creature,
    const EcoSim::Genetics::Plant& plant,
    EcoSim::Genetics::SeedDispersal& seedDispersal,
    std::vector<std::tuple<int, int, int, int>>& attachedBurrs);

/**
 * @brief Process burr detachment based on grooming and time.
 *
 * Burrs may detach due to creature movement, grooming behavior, or
 * natural wear. Detached burrs create dispersal events at creature's
 * current location.
 *
 * @param creature The creature with attached burrs
 * @param seedDispersal The shared seed dispersal calculator
 * @param attachedBurrs The creature's burr storage vector (modified)
 * @return Vector of dispersal events for detached burrs
 */
std::vector<EcoSim::Genetics::DispersalEvent> detachBurrs(
    const Creature& creature,
    EcoSim::Genetics::SeedDispersal& seedDispersal,
    std::vector<std::tuple<int, int, int, int>>& attachedBurrs);

/**
 * @brief Check if creature has any burrs attached.
 *
 * @param attachedBurrs The creature's burr storage vector
 * @return True if burrs are present
 */
bool hasBurrs(const std::vector<std::tuple<int, int, int, int>>& attachedBurrs);

/**
 * @brief Get pending dispersal events from attached burrs.
 *
 * Creates a snapshot of potential dispersal locations without
 * actually detaching the burrs. Useful for visualization.
 *
 * @param creature The creature with attached burrs
 * @param attachedBurrs The creature's burr storage vector
 * @return Vector of potential dispersal events
 */
std::vector<EcoSim::Genetics::DispersalEvent> getPendingBurrDispersal(
    const Creature& creature,
    const std::vector<std::tuple<int, int, int, int>>& attachedBurrs);

//============================================================================
//  Zoochory (Gut Seed Dispersal)
//============================================================================

/**
 * @brief Add seeds to creature's gut for digestion and potential dispersal.
 *
 * Seeds pass through the digestive system over time. Transit time is
 * determined by creature's GUT_TRANSIT_TIME gene. Optimal transit
 * improves viability through scarification.
 *
 * @param creature The creature consuming seeds
 * @param plant Source plant for the seeds
 * @param count Number of seeds consumed
 * @param viability Initial seed viability (0-1)
 * @param gutSeeds The creature's gut seed storage vector
 */
void consumeSeeds(
    const Creature& creature,
    const EcoSim::Genetics::Plant& plant,
    int count,
    float viability,
    std::vector<std::tuple<int, float, int>>& gutSeeds);

/**
 * @brief Process gut seed passage over time.
 *
 * Seeds that complete gut transit create dispersal events at the
 * creature's current location. Viability is modified by transit time:
 * - Optimal (4-12 hours): 15% bonus from scarification
 * - Too long (>12 hours): 10% penalty from acid damage
 *
 * @param creature The creature processing seeds
 * @param ticksElapsed Number of ticks since last processing
 * @param gutSeeds The creature's gut seed storage vector (modified)
 * @return Vector of dispersal events for passed seeds
 */
std::vector<EcoSim::Genetics::DispersalEvent> processGutSeeds(
    const Creature& creature,
    int ticksElapsed,
    std::vector<std::tuple<int, float, int>>& gutSeeds);

/**
 * @brief Check if creature has any seeds in gut.
 *
 * @param gutSeeds The creature's gut seed storage vector
 * @return True if gut seeds are present
 */
bool hasGutSeeds(const std::vector<std::tuple<int, float, int>>& gutSeeds);

} // namespace CreaturePlantInteraction

#endif // CREATURE_PLANT_INTERACTION_HPP
