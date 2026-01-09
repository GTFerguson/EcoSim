/**
 * @file test_biome_variants.cpp
 * @brief Tests for biome-specific organism variants
 *
 * Tests verify:
 * - All creature archetypes have thermal genes
 * - All plant templates have thermal genes
 * - Biome variants have correct thermal adaptations
 * - Arctic variants survive in cold, struggle in heat
 * - Desert variants survive in heat, struggle in cold
 * - Thermal genes affect stress calculations
 */

#include "test_framework.hpp"
#include "genetics/organisms/BiomeVariantExamples.hpp"
#include "genetics/organisms/CreatureFactory.hpp"
#include "genetics/organisms/PlantFactory.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/defaults/PlantGenes.hpp"
#include "genetics/expression/EnvironmentalStress.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include <iostream>
#include <iomanip>
#include <memory>

using namespace EcoSim::Genetics;
using namespace EcoSim::Testing;

//=============================================================================
// Test Fixtures
//=============================================================================

static std::shared_ptr<GeneRegistry> createRegistry() {
    auto registry = std::make_shared<GeneRegistry>();
    UniversalGenes::registerDefaults(*registry);
    // Don't call PlantGenes::registerDefaults here - the factories handle it
    return registry;
}

// Helper to get a gene value from genome (uses Incomplete dominance)
float getGeneValue(const Genome& genome, const std::string& geneId) {
    return genome.getGene(geneId).getNumericValue(DominanceType::Incomplete);
}

// Helper to check if a creature has a gene with a value in range
bool hasGeneInRange(const Genome& genome, const std::string& geneId, float minVal, float maxVal) {
    if (!genome.hasGene(geneId)) {
        std::cout << "      Missing gene: " << geneId << std::endl;
        return false;
    }
    float value = getGeneValue(genome, geneId);
    if (value < minVal || value > maxVal) {
        std::cout << "      Gene " << geneId << " = " << value
                  << " (expected [" << minVal << ", " << maxVal << "])" << std::endl;
        return false;
    }
    return true;
}

//=============================================================================
// Test: Creature Archetypes Have Thermal Genes
//=============================================================================

void test_apex_predator_has_thermal_genes() {
    auto registry = createRegistry();
    CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    Creature apex = factory.createApexPredator(0, 0);
    const Genome& genome = apex.getGenome();
    
    std::cout << "    Checking Apex Predator thermal genes..." << std::endl;
    
    TEST_ASSERT(genome.hasGene(UniversalGenes::TEMP_TOLERANCE_LOW));
    TEST_ASSERT(genome.hasGene(UniversalGenes::TEMP_TOLERANCE_HIGH));
    TEST_ASSERT(genome.hasGene(UniversalGenes::FUR_DENSITY));
    
    // Verify temperate baseline ranges
    TEST_ASSERT(hasGeneInRange(genome, UniversalGenes::TEMP_TOLERANCE_LOW, -5.0f, 0.0f));
    TEST_ASSERT(hasGeneInRange(genome, UniversalGenes::TEMP_TOLERANCE_HIGH, 30.0f, 35.0f));
    TEST_ASSERT(hasGeneInRange(genome, UniversalGenes::FUR_DENSITY, 0.40f, 0.50f));
}

void test_pack_hunter_has_thermal_genes() {
    auto registry = createRegistry();
    CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    Creature hunter = factory.createPackHunter(0, 0);
    const Genome& genome = hunter.getGenome();
    
    std::cout << "    Checking Pack Hunter thermal genes..." << std::endl;
    
    TEST_ASSERT(genome.hasGene(UniversalGenes::TEMP_TOLERANCE_LOW));
    TEST_ASSERT(genome.hasGene(UniversalGenes::TEMP_TOLERANCE_HIGH));
    TEST_ASSERT(genome.hasGene(UniversalGenes::FUR_DENSITY));
}

void test_tank_herbivore_has_thermal_genes() {
    auto registry = createRegistry();
    CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    Creature tank = factory.createTankHerbivore(0, 0);
    const Genome& genome = tank.getGenome();
    
    std::cout << "    Checking Tank Herbivore thermal genes..." << std::endl;
    
    TEST_ASSERT(genome.hasGene(UniversalGenes::TEMP_TOLERANCE_LOW));
    TEST_ASSERT(genome.hasGene(UniversalGenes::TEMP_TOLERANCE_HIGH));
    TEST_ASSERT(genome.hasGene(UniversalGenes::FUR_DENSITY));
}

void test_all_archetypes_have_thermal_genes() {
    auto registry = createRegistry();
    CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    std::vector<std::string> archetypes = factory.getTemplateNames();
    
    std::cout << "    Checking " << archetypes.size() << " archetypes for thermal genes..." << std::endl;
    
    for (const auto& name : archetypes) {
        Creature c = factory.createFromTemplate(name, 0, 0);
        const Genome& genome = c.getGenome();
        
        bool hasThermal = genome.hasGene(UniversalGenes::TEMP_TOLERANCE_LOW) &&
                          genome.hasGene(UniversalGenes::TEMP_TOLERANCE_HIGH) &&
                          genome.hasGene(UniversalGenes::FUR_DENSITY);
        
        if (!hasThermal) {
            std::cout << "      MISSING thermal genes in: " << name << std::endl;
        }
        TEST_ASSERT(hasThermal);
    }
    
    std::cout << "    All " << archetypes.size() << " archetypes have thermal genes." << std::endl;
}

//=============================================================================
// Test: Plant Templates Have Thermal Genes
//=============================================================================

void test_berry_bush_has_thermal_genes() {
    auto registry = createRegistry();
    PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    Plant bush = factory.createFromTemplate("berry_bush", 0, 0);
    const Genome& genome = bush.getGenome();
    
    std::cout << "    Checking Berry Bush thermal genes..." << std::endl;
    
    TEST_ASSERT(genome.hasGene(UniversalGenes::TEMP_TOLERANCE_LOW));
    TEST_ASSERT(genome.hasGene(UniversalGenes::TEMP_TOLERANCE_HIGH));
    TEST_ASSERT(genome.hasGene(UniversalGenes::WATER_STORAGE));
    
    TEST_ASSERT(hasGeneInRange(genome, UniversalGenes::WATER_STORAGE, 0.45f, 0.55f));
}

void test_oak_tree_has_thermal_genes() {
    auto registry = createRegistry();
    PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    Plant tree = factory.createFromTemplate("oak_tree", 0, 0);
    const Genome& genome = tree.getGenome();
    
    std::cout << "    Checking Oak Tree thermal genes..." << std::endl;
    
    TEST_ASSERT(genome.hasGene(UniversalGenes::TEMP_TOLERANCE_LOW));
    TEST_ASSERT(genome.hasGene(UniversalGenes::TEMP_TOLERANCE_HIGH));
    TEST_ASSERT(genome.hasGene(UniversalGenes::WATER_STORAGE));
    
    TEST_ASSERT(hasGeneInRange(genome, UniversalGenes::WATER_STORAGE, 0.55f, 0.70f));
}

void test_grass_has_thermal_genes() {
    auto registry = createRegistry();
    PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    Plant grass = factory.createFromTemplate("grass", 0, 0);
    const Genome& genome = grass.getGenome();
    
    std::cout << "    Checking Grass thermal genes..." << std::endl;
    
    TEST_ASSERT(genome.hasGene(UniversalGenes::TEMP_TOLERANCE_LOW));
    TEST_ASSERT(genome.hasGene(UniversalGenes::TEMP_TOLERANCE_HIGH));
    TEST_ASSERT(genome.hasGene(UniversalGenes::WATER_STORAGE));
}

void test_thorn_bush_has_thermal_genes() {
    auto registry = createRegistry();
    PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    Plant thorn = factory.createFromTemplate("thorn_bush", 0, 0);
    const Genome& genome = thorn.getGenome();
    
    std::cout << "    Checking Thorn Bush thermal genes..." << std::endl;
    
    TEST_ASSERT(genome.hasGene(UniversalGenes::TEMP_TOLERANCE_LOW));
    TEST_ASSERT(genome.hasGene(UniversalGenes::TEMP_TOLERANCE_HIGH));
    TEST_ASSERT(genome.hasGene(UniversalGenes::WATER_STORAGE));
    
    TEST_ASSERT(hasGeneInRange(genome, UniversalGenes::WATER_STORAGE, 0.60f, 0.75f));
}

void test_all_plant_templates_have_thermal_genes() {
    auto registry = createRegistry();
    PlantFactory factory(registry);
    factory.registerDefaultTemplates();
    
    std::vector<std::string> templates = factory.getTemplateNames();
    
    std::cout << "    Checking " << templates.size() << " plant templates for thermal genes..." << std::endl;
    
    for (const auto& name : templates) {
        Plant p = factory.createFromTemplate(name, 0, 0);
        const Genome& genome = p.getGenome();
        
        bool hasThermal = genome.hasGene(UniversalGenes::TEMP_TOLERANCE_LOW) &&
                          genome.hasGene(UniversalGenes::TEMP_TOLERANCE_HIGH) &&
                          genome.hasGene(UniversalGenes::WATER_STORAGE);
        
        if (!hasThermal) {
            std::cout << "      MISSING thermal genes in: " << name << std::endl;
        }
        TEST_ASSERT(hasThermal);
    }
    
    std::cout << "    All " << templates.size() << " plant templates have thermal genes." << std::endl;
}

//=============================================================================
// Test: Arctic Wolf Variant
//=============================================================================

void test_arctic_wolf_thermal_adaptations() {
    auto registry = createRegistry();
    BiomeVariantFactory factory(registry);
    
    Creature wolf = factory.createArcticWolf(0, 0);
    const Genome& genome = wolf.getGenome();
    
    std::cout << "    Checking Arctic Wolf adaptations..." << std::endl;
    
    // Should have high insulation for cold
    float furDensity = getGeneValue(genome, UniversalGenes::FUR_DENSITY);
    float fatLayer = getGeneValue(genome, UniversalGenes::FAT_LAYER_THICKNESS);
    float metabolism = getGeneValue(genome, UniversalGenes::METABOLISM_RATE);
    float tempMin = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_LOW);
    float tempMax = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_HIGH);
    
    std::cout << "      Fur density: " << furDensity << " (expected ~0.90)" << std::endl;
    std::cout << "      Fat layer: " << fatLayer << " (expected ~0.85)" << std::endl;
    std::cout << "      Metabolism: " << metabolism << " (expected ~1.6)" << std::endl;
    std::cout << "      Temp range: [" << tempMin << ", " << tempMax << "]" << std::endl;
    
    TEST_ASSERT_NEAR(furDensity, 0.90f, 0.05f);
    TEST_ASSERT_NEAR(fatLayer, 0.85f, 0.05f);
    TEST_ASSERT_NEAR(metabolism, 1.6f, 0.1f);
    TEST_ASSERT_NEAR(tempMin, -35.0f, 1.0f);
    TEST_ASSERT_NEAR(tempMax, 15.0f, 1.0f);
}

void test_arctic_wolf_survives_cold() {
    auto registry = createRegistry();
    BiomeVariantFactory factory(registry);
    
    Creature wolf = factory.createArcticWolf(0, 0);
    const Genome& genome = wolf.getGenome();
    
    float tempMin = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_LOW);
    float tempMax = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_HIGH);
    
    // Build thermal adaptations from genome
    ThermalAdaptations adapt;
    adapt.furDensity = getGeneValue(genome, UniversalGenes::FUR_DENSITY);
    adapt.fatLayerThickness = getGeneValue(genome, UniversalGenes::FAT_LAYER_THICKNESS);
    adapt.metabolismRate = getGeneValue(genome, UniversalGenes::METABOLISM_RATE);
    
    // Test at -20°C (cold but within range)
    auto stress = EnvironmentalStressCalculator::calculateTemperatureStress(
        -20.0f, tempMin, tempMax, adapt);
    
    std::cout << "    Arctic Wolf at -20°C: " 
              << EnvironmentalStressCalculator::stressLevelToString(stress.severity) << std::endl;
    
    // Should be comfortable or only mildly stressed in cold
    TEST_ASSERT_LE(static_cast<int>(stress.severity), static_cast<int>(StressLevel::Mild));
}

void test_arctic_wolf_struggles_in_heat() {
    auto registry = createRegistry();
    BiomeVariantFactory factory(registry);
    
    Creature wolf = factory.createArcticWolf(0, 0);
    const Genome& genome = wolf.getGenome();
    
    float tempMin = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_LOW);
    float tempMax = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_HIGH);
    
    ThermalAdaptations adapt;
    adapt.furDensity = getGeneValue(genome, UniversalGenes::FUR_DENSITY);
    adapt.fatLayerThickness = getGeneValue(genome, UniversalGenes::FAT_LAYER_THICKNESS);
    adapt.metabolismRate = getGeneValue(genome, UniversalGenes::METABOLISM_RATE);
    
    // Test at 35°C (hot - should be stressful)
    auto stress = EnvironmentalStressCalculator::calculateTemperatureStress(
        35.0f, tempMin, tempMax, adapt);
    
    std::cout << "    Arctic Wolf at 35°C: " 
              << EnvironmentalStressCalculator::stressLevelToString(stress.severity)
              << " (heat stress: " << (stress.isHeatStress ? "yes" : "no") << ")" << std::endl;
    
    // Should be stressed in heat
    TEST_ASSERT(stress.isStressed());
    TEST_ASSERT(stress.isHeatStress);
}

//=============================================================================
// Test: Desert Fennec Variant
//=============================================================================

void test_desert_fennec_thermal_adaptations() {
    auto registry = createRegistry();
    BiomeVariantFactory factory(registry);
    
    Creature fennec = factory.createDesertFennec(0, 0);
    const Genome& genome = fennec.getGenome();
    
    std::cout << "    Checking Desert Fennec adaptations..." << std::endl;
    
    float furDensity = getGeneValue(genome, UniversalGenes::FUR_DENSITY);
    float fatLayer = getGeneValue(genome, UniversalGenes::FAT_LAYER_THICKNESS);
    float metabolism = getGeneValue(genome, UniversalGenes::METABOLISM_RATE);
    float tempMin = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_LOW);
    float tempMax = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_HIGH);
    
    std::cout << "      Fur density: " << furDensity << " (expected ~0.10)" << std::endl;
    std::cout << "      Fat layer: " << fatLayer << " (expected ~0.15)" << std::endl;
    std::cout << "      Metabolism: " << metabolism << " (expected ~0.70)" << std::endl;
    std::cout << "      Temp range: [" << tempMin << ", " << tempMax << "]" << std::endl;
    
    TEST_ASSERT_NEAR(furDensity, 0.10f, 0.05f);
    TEST_ASSERT_NEAR(fatLayer, 0.15f, 0.05f);
    TEST_ASSERT_NEAR(metabolism, 0.70f, 0.1f);
    TEST_ASSERT_NEAR(tempMin, 5.0f, 1.0f);
    TEST_ASSERT_NEAR(tempMax, 50.0f, 1.0f);
}

void test_desert_fennec_survives_heat() {
    auto registry = createRegistry();
    BiomeVariantFactory factory(registry);
    
    Creature fennec = factory.createDesertFennec(0, 0);
    const Genome& genome = fennec.getGenome();
    
    float tempMin = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_LOW);
    float tempMax = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_HIGH);
    
    ThermalAdaptations adapt;
    adapt.furDensity = getGeneValue(genome, UniversalGenes::FUR_DENSITY);
    adapt.fatLayerThickness = getGeneValue(genome, UniversalGenes::FAT_LAYER_THICKNESS);
    adapt.metabolismRate = getGeneValue(genome, UniversalGenes::METABOLISM_RATE);
    adapt.thermoregulation = 0.7f;  // Desert creatures have good thermoregulation
    
    // Test at 40°C (hot but within range)
    auto stress = EnvironmentalStressCalculator::calculateTemperatureStress(
        40.0f, tempMin, tempMax, adapt);
    
    std::cout << "    Desert Fennec at 40°C: " 
              << EnvironmentalStressCalculator::stressLevelToString(stress.severity) << std::endl;
    
    // Should be comfortable or only mildly stressed in heat
    TEST_ASSERT_LE(static_cast<int>(stress.severity), static_cast<int>(StressLevel::Mild));
}

void test_desert_fennec_struggles_in_cold() {
    auto registry = createRegistry();
    BiomeVariantFactory factory(registry);
    
    Creature fennec = factory.createDesertFennec(0, 0);
    const Genome& genome = fennec.getGenome();
    
    float tempMin = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_LOW);
    float tempMax = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_HIGH);
    
    ThermalAdaptations adapt;
    adapt.furDensity = getGeneValue(genome, UniversalGenes::FUR_DENSITY);
    adapt.fatLayerThickness = getGeneValue(genome, UniversalGenes::FAT_LAYER_THICKNESS);
    adapt.metabolismRate = getGeneValue(genome, UniversalGenes::METABOLISM_RATE);
    
    // Test at -10°C (cold - should be stressful)
    auto stress = EnvironmentalStressCalculator::calculateTemperatureStress(
        -10.0f, tempMin, tempMax, adapt);
    
    std::cout << "    Desert Fennec at -10°C: " 
              << EnvironmentalStressCalculator::stressLevelToString(stress.severity)
              << " (cold stress: " << (stress.isColdStress() ? "yes" : "no") << ")" << std::endl;
    
    // Should be stressed in cold
    TEST_ASSERT(stress.isStressed());
    TEST_ASSERT(stress.isColdStress());
}

//=============================================================================
// Test: Woolly Mammoth (Tundra Herbivore)
//=============================================================================

void test_woolly_mammoth_thermal_adaptations() {
    auto registry = createRegistry();
    BiomeVariantFactory factory(registry);
    
    Creature mammoth = factory.createWoollyMammoth(0, 0);
    const Genome& genome = mammoth.getGenome();
    
    std::cout << "    Checking Woolly Mammoth adaptations (Tundra Herbivore)..." << std::endl;
    
    float furDensity = getGeneValue(genome, UniversalGenes::FUR_DENSITY);
    float fatLayer = getGeneValue(genome, UniversalGenes::FAT_LAYER_THICKNESS);
    float metabolism = getGeneValue(genome, UniversalGenes::METABOLISM_RATE);
    float tempMin = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_LOW);
    float tempMax = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_HIGH);
    
    std::cout << "      Fur density: " << furDensity << " (expected ~0.95)" << std::endl;
    std::cout << "      Fat layer: " << fatLayer << " (expected ~0.90)" << std::endl;
    std::cout << "      Metabolism: " << metabolism << " (expected ~1.4)" << std::endl;
    std::cout << "      Temp range: [" << tempMin << ", " << tempMax << "]" << std::endl;
    
    TEST_ASSERT_NEAR(furDensity, 0.95f, 0.05f);
    TEST_ASSERT_NEAR(fatLayer, 0.90f, 0.05f);
    TEST_ASSERT_NEAR(metabolism, 1.4f, 0.1f);
    TEST_ASSERT_NEAR(tempMin, -40.0f, 1.0f);
    TEST_ASSERT_NEAR(tempMax, 10.0f, 1.0f);
}

//=============================================================================
// Test: Desert Camel (Desert Herbivore)
//=============================================================================

void test_desert_camel_thermal_adaptations() {
    auto registry = createRegistry();
    BiomeVariantFactory factory(registry);
    
    Creature camel = factory.createDesertCamel(0, 0);
    const Genome& genome = camel.getGenome();
    
    std::cout << "    Checking Desert Camel adaptations (Desert Herbivore)..." << std::endl;
    
    float furDensity = getGeneValue(genome, UniversalGenes::FUR_DENSITY);
    float fatLayer = getGeneValue(genome, UniversalGenes::FAT_LAYER_THICKNESS);
    float metabolism = getGeneValue(genome, UniversalGenes::METABOLISM_RATE);
    float tempMin = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_LOW);
    float tempMax = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_HIGH);
    
    std::cout << "      Fur density: " << furDensity << " (expected ~0.20)" << std::endl;
    std::cout << "      Fat layer: " << fatLayer << " (expected ~0.80)" << std::endl;
    std::cout << "      Metabolism: " << metabolism << " (expected ~0.75)" << std::endl;
    std::cout << "      Temp range: [" << tempMin << ", " << tempMax << "]" << std::endl;
    
    TEST_ASSERT_NEAR(furDensity, 0.20f, 0.05f);
    TEST_ASSERT_NEAR(fatLayer, 0.80f, 0.05f);
    TEST_ASSERT_NEAR(metabolism, 0.75f, 0.1f);
    TEST_ASSERT_NEAR(tempMin, 0.0f, 1.0f);
    TEST_ASSERT_NEAR(tempMax, 55.0f, 1.0f);
}

//=============================================================================
// Test: Jungle Elephant (Tropical Herbivore)
//=============================================================================

void test_jungle_elephant_thermal_adaptations() {
    auto registry = createRegistry();
    BiomeVariantFactory factory(registry);
    
    Creature elephant = factory.createJungleElephant(0, 0);
    const Genome& genome = elephant.getGenome();
    
    std::cout << "    Checking Jungle Elephant adaptations (Tropical Herbivore)..." << std::endl;
    
    float furDensity = getGeneValue(genome, UniversalGenes::FUR_DENSITY);
    float fatLayer = getGeneValue(genome, UniversalGenes::FAT_LAYER_THICKNESS);
    float metabolism = getGeneValue(genome, UniversalGenes::METABOLISM_RATE);
    float tempMin = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_LOW);
    float tempMax = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_HIGH);
    
    std::cout << "      Fur density: " << furDensity << " (expected ~0.10)" << std::endl;
    std::cout << "      Fat layer: " << fatLayer << " (expected ~0.25)" << std::endl;
    std::cout << "      Metabolism: " << metabolism << " (expected ~0.95)" << std::endl;
    std::cout << "      Temp range: [" << tempMin << ", " << tempMax << "]" << std::endl;
    
    TEST_ASSERT_NEAR(furDensity, 0.10f, 0.05f);
    TEST_ASSERT_NEAR(fatLayer, 0.25f, 0.05f);
    TEST_ASSERT_NEAR(metabolism, 0.95f, 0.1f);
    TEST_ASSERT_NEAR(tempMin, 10.0f, 1.0f);
    TEST_ASSERT_NEAR(tempMax, 45.0f, 1.0f);
}

//=============================================================================
// Test: Tropical Jaguar (Tropical Carnivore)
//=============================================================================

void test_tropical_jaguar_thermal_adaptations() {
    auto registry = createRegistry();
    BiomeVariantFactory factory(registry);
    
    Creature jaguar = factory.createTropicalJaguar(0, 0);
    const Genome& genome = jaguar.getGenome();
    
    std::cout << "    Checking Tropical Jaguar adaptations (Tropical Carnivore)..." << std::endl;
    
    float furDensity = getGeneValue(genome, UniversalGenes::FUR_DENSITY);
    float fatLayer = getGeneValue(genome, UniversalGenes::FAT_LAYER_THICKNESS);
    float tempMin = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_LOW);
    float tempMax = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_HIGH);
    
    std::cout << "      Fur density: " << furDensity << " (expected ~0.15)" << std::endl;
    std::cout << "      Fat layer: " << fatLayer << " (expected ~0.20)" << std::endl;
    std::cout << "      Temp range: [" << tempMin << ", " << tempMax << "]" << std::endl;
    
    TEST_ASSERT_NEAR(furDensity, 0.15f, 0.05f);
    TEST_ASSERT_NEAR(fatLayer, 0.20f, 0.05f);
    TEST_ASSERT_NEAR(tempMin, 15.0f, 1.0f);
    TEST_ASSERT_NEAR(tempMax, 45.0f, 1.0f);
}

//=============================================================================
// Test: Complete Food Chain per Biome
//=============================================================================

void test_tundra_food_chain_exists() {
    auto registry = createRegistry();
    BiomeVariantFactory factory(registry);
    
    std::cout << "    Verifying Tundra food chain (Plant -> Herbivore -> Carnivore)..." << std::endl;
    
    // Primary producer
    Plant moss = factory.createTundraMoss(0, 0);
    TEST_ASSERT(moss.getGenome().hasGene(UniversalGenes::TEMP_TOLERANCE_LOW));
    std::cout << "      ✓ Tundra Moss (Plant)" << std::endl;
    
    // Primary consumer (herbivore)
    Creature mammoth = factory.createWoollyMammoth(0, 0);
    TEST_ASSERT(mammoth.getGenome().hasGene(UniversalGenes::TEMP_TOLERANCE_LOW));
    std::cout << "      ✓ Woolly Mammoth (Herbivore)" << std::endl;
    
    // Secondary consumer (carnivore)
    Creature wolf = factory.createArcticWolf(0, 0);
    TEST_ASSERT(wolf.getGenome().hasGene(UniversalGenes::TEMP_TOLERANCE_LOW));
    std::cout << "      ✓ Arctic Wolf (Carnivore)" << std::endl;
    
    // Verify all share same biome temperature tolerance
    float mossMin = getGeneValue(moss.getGenome(), UniversalGenes::TEMP_TOLERANCE_LOW);
    float mammothMin = getGeneValue(mammoth.getGenome(), UniversalGenes::TEMP_TOLERANCE_LOW);
    float wolfMin = getGeneValue(wolf.getGenome(), UniversalGenes::TEMP_TOLERANCE_LOW);
    
    // All should tolerate very cold temperatures (below -30°C)
    TEST_ASSERT_LT(mossMin, -30.0f);
    TEST_ASSERT_LT(mammothMin, -30.0f);
    TEST_ASSERT_LT(wolfMin, -30.0f);
    
    std::cout << "      ✓ All organisms share tundra temperature tolerance" << std::endl;
}

void test_desert_food_chain_exists() {
    auto registry = createRegistry();
    BiomeVariantFactory factory(registry);
    
    std::cout << "    Verifying Desert food chain (Plant -> Herbivore -> Carnivore)..." << std::endl;
    
    // Primary producer
    Plant cactus = factory.createDesertCactus(0, 0);
    TEST_ASSERT(cactus.getGenome().hasGene(UniversalGenes::TEMP_TOLERANCE_HIGH));
    std::cout << "      ✓ Desert Cactus (Plant)" << std::endl;
    
    // Primary consumer (herbivore)
    Creature camel = factory.createDesertCamel(0, 0);
    TEST_ASSERT(camel.getGenome().hasGene(UniversalGenes::TEMP_TOLERANCE_HIGH));
    std::cout << "      ✓ Desert Camel (Herbivore)" << std::endl;
    
    // Secondary consumer (carnivore)
    Creature fennec = factory.createDesertFennec(0, 0);
    TEST_ASSERT(fennec.getGenome().hasGene(UniversalGenes::TEMP_TOLERANCE_HIGH));
    std::cout << "      ✓ Desert Fennec (Carnivore)" << std::endl;
    
    // Verify all share same biome temperature tolerance
    float cactusMax = getGeneValue(cactus.getGenome(), UniversalGenes::TEMP_TOLERANCE_HIGH);
    float camelMax = getGeneValue(camel.getGenome(), UniversalGenes::TEMP_TOLERANCE_HIGH);
    float fennecMax = getGeneValue(fennec.getGenome(), UniversalGenes::TEMP_TOLERANCE_HIGH);
    
    // All should tolerate very high temperatures (above 45°C)
    TEST_ASSERT_GT(cactusMax, 45.0f);
    TEST_ASSERT_GT(camelMax, 45.0f);
    TEST_ASSERT_GT(fennecMax, 45.0f);
    
    std::cout << "      ✓ All organisms share desert temperature tolerance" << std::endl;
}

void test_tropical_food_chain_exists() {
    auto registry = createRegistry();
    BiomeVariantFactory factory(registry);
    
    std::cout << "    Verifying Tropical food chain (Plant -> Herbivore -> Carnivore)..." << std::endl;
    
    // Primary producer
    Plant vine = factory.createRainforestVine(0, 0);
    TEST_ASSERT(vine.getGenome().hasGene(UniversalGenes::TEMP_TOLERANCE_LOW));
    std::cout << "      ✓ Rainforest Vine (Plant)" << std::endl;
    
    // Primary consumer (herbivore)
    Creature elephant = factory.createJungleElephant(0, 0);
    TEST_ASSERT(elephant.getGenome().hasGene(UniversalGenes::TEMP_TOLERANCE_LOW));
    std::cout << "      ✓ Jungle Elephant (Herbivore)" << std::endl;
    
    // Secondary consumer (carnivore)
    Creature jaguar = factory.createTropicalJaguar(0, 0);
    TEST_ASSERT(jaguar.getGenome().hasGene(UniversalGenes::TEMP_TOLERANCE_LOW));
    std::cout << "      ✓ Tropical Jaguar (Carnivore)" << std::endl;
    
    // Verify all share same biome temperature tolerance
    float vineMin = getGeneValue(vine.getGenome(), UniversalGenes::TEMP_TOLERANCE_LOW);
    float elephantMin = getGeneValue(elephant.getGenome(), UniversalGenes::TEMP_TOLERANCE_LOW);
    float jaguarMin = getGeneValue(jaguar.getGenome(), UniversalGenes::TEMP_TOLERANCE_LOW);
    
    // All should have warm minimum tolerance (above 5°C)
    TEST_ASSERT_GT(vineMin, 5.0f);
    TEST_ASSERT_GT(elephantMin, 5.0f);
    TEST_ASSERT_GT(jaguarMin, 5.0f);
    
    std::cout << "      ✓ All organisms share tropical temperature tolerance" << std::endl;
}

//=============================================================================
// Test: Plant Variants
//=============================================================================

void test_tundra_moss_thermal_adaptations() {
    auto registry = createRegistry();
    BiomeVariantFactory factory(registry);
    
    Plant moss = factory.createTundraMoss(0, 0);
    const Genome& genome = moss.getGenome();
    
    std::cout << "    Checking Tundra Moss adaptations..." << std::endl;
    
    float waterStorage = getGeneValue(genome, UniversalGenes::WATER_STORAGE);
    float tempMin = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_LOW);
    float tempMax = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_HIGH);
    float hardiness = getGeneValue(genome, UniversalGenes::HARDINESS);
    
    std::cout << "      Water storage: " << waterStorage << " (expected ~0.30)" << std::endl;
    std::cout << "      Hardiness: " << hardiness << " (expected ~0.85)" << std::endl;
    std::cout << "      Temp range: [" << tempMin << ", " << tempMax << "]" << std::endl;
    
    TEST_ASSERT_NEAR(waterStorage, 0.30f, 0.05f);
    TEST_ASSERT_NEAR(hardiness, 0.85f, 0.05f);
    TEST_ASSERT_NEAR(tempMin, -40.0f, 1.0f);
    TEST_ASSERT_NEAR(tempMax, 10.0f, 1.0f);
}

void test_desert_cactus_thermal_adaptations() {
    auto registry = createRegistry();
    BiomeVariantFactory factory(registry);
    
    Plant cactus = factory.createDesertCactus(0, 0);
    const Genome& genome = cactus.getGenome();
    
    std::cout << "    Checking Desert Cactus adaptations..." << std::endl;
    
    float waterStorage = getGeneValue(genome, UniversalGenes::WATER_STORAGE);
    float thornDensity = getGeneValue(genome, UniversalGenes::THORN_DENSITY);
    float tempMin = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_LOW);
    float tempMax = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_HIGH);
    
    std::cout << "      Water storage: " << waterStorage << " (expected ~0.95)" << std::endl;
    std::cout << "      Thorn density: " << thornDensity << " (expected ~0.95)" << std::endl;
    std::cout << "      Temp range: [" << tempMin << ", " << tempMax << "]" << std::endl;
    
    TEST_ASSERT_NEAR(waterStorage, 0.95f, 0.05f);
    TEST_ASSERT_NEAR(thornDensity, 0.95f, 0.05f);
    TEST_ASSERT_NEAR(tempMin, 5.0f, 1.0f);
    TEST_ASSERT_NEAR(tempMax, 55.0f, 1.0f);
}

void test_rainforest_vine_thermal_adaptations() {
    auto registry = createRegistry();
    BiomeVariantFactory factory(registry);
    
    Plant vine = factory.createRainforestVine(0, 0);
    const Genome& genome = vine.getGenome();
    
    std::cout << "    Checking Rainforest Vine adaptations..." << std::endl;
    
    float waterStorage = getGeneValue(genome, UniversalGenes::WATER_STORAGE);
    float growthRate = getGeneValue(genome, PlantGenes::GROWTH_RATE);
    float tempMin = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_LOW);
    float tempMax = getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_HIGH);
    
    std::cout << "      Water storage: " << waterStorage << " (expected ~0.40)" << std::endl;
    std::cout << "      Growth rate: " << growthRate << " (expected ~0.75)" << std::endl;
    std::cout << "      Temp range: [" << tempMin << ", " << tempMax << "]" << std::endl;
    
    TEST_ASSERT_NEAR(waterStorage, 0.40f, 0.05f);
    TEST_ASSERT_NEAR(growthRate, 0.75f, 0.05f);
    TEST_ASSERT_NEAR(tempMin, 15.0f, 1.0f);
    TEST_ASSERT_NEAR(tempMax, 40.0f, 1.0f);
}

//=============================================================================
// Test: Biome Temperature Ranges
//=============================================================================

void test_biome_temperature_ranges() {
    std::cout << "    Checking biome temperature ranges..." << std::endl;
    
    auto [tundraMin, tundraMax] = BiomeVariantFactory::getBiomeTemperatureRange(BiomeType::Tundra);
    auto [desertMin, desertMax] = BiomeVariantFactory::getBiomeTemperatureRange(BiomeType::Desert);
    auto [tropicalMin, tropicalMax] = BiomeVariantFactory::getBiomeTemperatureRange(BiomeType::Tropical);
    auto [temperateMin, temperateMax] = BiomeVariantFactory::getBiomeTemperatureRange(BiomeType::Temperate);
    
    std::cout << "      Tundra: [" << tundraMin << ", " << tundraMax << "]" << std::endl;
    std::cout << "      Desert: [" << desertMin << ", " << desertMax << "]" << std::endl;
    std::cout << "      Tropical: [" << tropicalMin << ", " << tropicalMax << "]" << std::endl;
    std::cout << "      Temperate: [" << temperateMin << ", " << temperateMax << "]" << std::endl;
    
    // Tundra is coldest
    TEST_ASSERT_LT(tundraMin, temperateMin);
    TEST_ASSERT_LT(tundraMax, temperateMax);
    
    // Desert is hottest
    TEST_ASSERT_GT(desertMax, temperateMax);
    TEST_ASSERT_GT(desertMin, tundraMin);
    
    // Tropical overlaps with temperate but warmer
    TEST_ASSERT_GT(tropicalMin, temperateMin);
}

//=============================================================================
// Test: Apply Biome Adaptation
//=============================================================================

void test_apply_biome_adaptation_tundra() {
    auto registry = createRegistry();
    CreatureFactory creatureFactory(registry);
    creatureFactory.registerDefaultTemplates();
    
    // Create a creature and get a mutable copy of its genome
    Creature creature = creatureFactory.createApexPredator(0, 0);
    Genome& genome = creature.getGenomeMutable();
    
    std::cout << "    Applying tundra adaptation..." << std::endl;
    
    float furBefore = getGeneValue(genome, UniversalGenes::FUR_DENSITY);
    BiomeVariantFactory::applyBiomeAdaptation(genome, BiomeType::Tundra);
    float furAfter = getGeneValue(genome, UniversalGenes::FUR_DENSITY);
    
    std::cout << "      Fur density: " << furBefore << " -> " << furAfter << std::endl;
    
    TEST_ASSERT_GT(furAfter, 0.7f);  // High fur for cold
    TEST_ASSERT_NEAR(getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_LOW), -35.0f, 1.0f);
}

void test_apply_biome_adaptation_desert() {
    auto registry = createRegistry();
    CreatureFactory creatureFactory(registry);
    creatureFactory.registerDefaultTemplates();
    
    // Create a creature and get a mutable copy of its genome
    Creature creature = creatureFactory.createApexPredator(0, 0);
    Genome& genome = creature.getGenomeMutable();
    
    std::cout << "    Applying desert adaptation..." << std::endl;
    
    BiomeVariantFactory::applyBiomeAdaptation(genome, BiomeType::Desert);
    
    float fur = getGeneValue(genome, UniversalGenes::FUR_DENSITY);
    
    std::cout << "      Fur density: " << fur << " (expected low)" << std::endl;
    
    TEST_ASSERT_LT(fur, 0.3f);  // Low fur for heat
    TEST_ASSERT_NEAR(getGeneValue(genome, UniversalGenes::TEMP_TOLERANCE_HIGH), 50.0f, 1.0f);
}

//=============================================================================
// Test: Cross-biome Stress Comparison
//=============================================================================

void test_arctic_vs_desert_in_cold() {
    auto registry = createRegistry();
    BiomeVariantFactory factory(registry);
    
    Creature arctic = factory.createArcticWolf(0, 0);
    Creature desert = factory.createDesertFennec(0, 0);
    
    // Build adaptations
    ThermalAdaptations arcticAdapt;
    arcticAdapt.furDensity = getGeneValue(arctic.getGenome(), UniversalGenes::FUR_DENSITY);
    arcticAdapt.fatLayerThickness = getGeneValue(arctic.getGenome(), UniversalGenes::FAT_LAYER_THICKNESS);
    arcticAdapt.metabolismRate = getGeneValue(arctic.getGenome(), UniversalGenes::METABOLISM_RATE);
    
    ThermalAdaptations desertAdapt;
    desertAdapt.furDensity = getGeneValue(desert.getGenome(), UniversalGenes::FUR_DENSITY);
    desertAdapt.fatLayerThickness = getGeneValue(desert.getGenome(), UniversalGenes::FAT_LAYER_THICKNESS);
    desertAdapt.metabolismRate = getGeneValue(desert.getGenome(), UniversalGenes::METABOLISM_RATE);
    
    // Test at -15°C
    float coldTemp = -15.0f;
    
    auto arcticStress = EnvironmentalStressCalculator::calculateTemperatureStress(
        coldTemp,
        getGeneValue(arctic.getGenome(), UniversalGenes::TEMP_TOLERANCE_LOW),
        getGeneValue(arctic.getGenome(), UniversalGenes::TEMP_TOLERANCE_HIGH),
        arcticAdapt);
    
    auto desertStress = EnvironmentalStressCalculator::calculateTemperatureStress(
        coldTemp,
        getGeneValue(desert.getGenome(), UniversalGenes::TEMP_TOLERANCE_LOW),
        getGeneValue(desert.getGenome(), UniversalGenes::TEMP_TOLERANCE_HIGH),
        desertAdapt);
    
    std::cout << "    At -15°C:" << std::endl;
    std::cout << "      Arctic Wolf: " << EnvironmentalStressCalculator::stressLevelToString(arcticStress.severity) << std::endl;
    std::cout << "      Desert Fennec: " << EnvironmentalStressCalculator::stressLevelToString(desertStress.severity) << std::endl;
    
    // Arctic should be less stressed than desert in cold
    TEST_ASSERT_LT(static_cast<int>(arcticStress.severity), static_cast<int>(desertStress.severity));
}

void test_arctic_vs_desert_in_heat() {
    auto registry = createRegistry();
    BiomeVariantFactory factory(registry);
    
    Creature arctic = factory.createArcticWolf(0, 0);
    Creature desert = factory.createDesertFennec(0, 0);
    
    ThermalAdaptations arcticAdapt;
    arcticAdapt.furDensity = getGeneValue(arctic.getGenome(), UniversalGenes::FUR_DENSITY);
    arcticAdapt.fatLayerThickness = getGeneValue(arctic.getGenome(), UniversalGenes::FAT_LAYER_THICKNESS);
    arcticAdapt.metabolismRate = getGeneValue(arctic.getGenome(), UniversalGenes::METABOLISM_RATE);
    
    ThermalAdaptations desertAdapt;
    desertAdapt.furDensity = getGeneValue(desert.getGenome(), UniversalGenes::FUR_DENSITY);
    desertAdapt.fatLayerThickness = getGeneValue(desert.getGenome(), UniversalGenes::FAT_LAYER_THICKNESS);
    desertAdapt.metabolismRate = getGeneValue(desert.getGenome(), UniversalGenes::METABOLISM_RATE);
    desertAdapt.thermoregulation = 0.7f;
    
    // Test at 35°C
    float hotTemp = 35.0f;
    
    auto arcticStress = EnvironmentalStressCalculator::calculateTemperatureStress(
        hotTemp,
        getGeneValue(arctic.getGenome(), UniversalGenes::TEMP_TOLERANCE_LOW),
        getGeneValue(arctic.getGenome(), UniversalGenes::TEMP_TOLERANCE_HIGH),
        arcticAdapt);
    
    auto desertStress = EnvironmentalStressCalculator::calculateTemperatureStress(
        hotTemp,
        getGeneValue(desert.getGenome(), UniversalGenes::TEMP_TOLERANCE_LOW),
        getGeneValue(desert.getGenome(), UniversalGenes::TEMP_TOLERANCE_HIGH),
        desertAdapt);
    
    std::cout << "    At 35°C:" << std::endl;
    std::cout << "      Arctic Wolf: " << EnvironmentalStressCalculator::stressLevelToString(arcticStress.severity) << std::endl;
    std::cout << "      Desert Fennec: " << EnvironmentalStressCalculator::stressLevelToString(desertStress.severity) << std::endl;
    
    // Desert should be less stressed than arctic in heat
    TEST_ASSERT_LT(static_cast<int>(desertStress.severity), static_cast<int>(arcticStress.severity));
}

//=============================================================================
// Test Runner
//=============================================================================

void runBiomeVariantTests() {
    BEGIN_TEST_GROUP("Biome Variant System");
    
    // Creature archetype thermal gene tests
    RUN_TEST(test_apex_predator_has_thermal_genes);
    RUN_TEST(test_pack_hunter_has_thermal_genes);
    RUN_TEST(test_tank_herbivore_has_thermal_genes);
    RUN_TEST(test_all_archetypes_have_thermal_genes);
    
    // Plant template thermal gene tests
    RUN_TEST(test_berry_bush_has_thermal_genes);
    RUN_TEST(test_oak_tree_has_thermal_genes);
    RUN_TEST(test_grass_has_thermal_genes);
    RUN_TEST(test_thorn_bush_has_thermal_genes);
    RUN_TEST(test_all_plant_templates_have_thermal_genes);
    
    // Tundra biome variants
    RUN_TEST(test_arctic_wolf_thermal_adaptations);
    RUN_TEST(test_arctic_wolf_survives_cold);
    RUN_TEST(test_arctic_wolf_struggles_in_heat);
    RUN_TEST(test_woolly_mammoth_thermal_adaptations);
    RUN_TEST(test_tundra_moss_thermal_adaptations);
    
    // Desert biome variants
    RUN_TEST(test_desert_fennec_thermal_adaptations);
    RUN_TEST(test_desert_fennec_survives_heat);
    RUN_TEST(test_desert_fennec_struggles_in_cold);
    RUN_TEST(test_desert_camel_thermal_adaptations);
    RUN_TEST(test_desert_cactus_thermal_adaptations);
    
    // Tropical biome variants
    RUN_TEST(test_tropical_jaguar_thermal_adaptations);
    RUN_TEST(test_jungle_elephant_thermal_adaptations);
    RUN_TEST(test_rainforest_vine_thermal_adaptations);
    
    // Food chain verification - each biome has plant + herbivore + carnivore
    RUN_TEST(test_tundra_food_chain_exists);
    RUN_TEST(test_desert_food_chain_exists);
    RUN_TEST(test_tropical_food_chain_exists);
    
    // Utility tests
    RUN_TEST(test_biome_temperature_ranges);
    RUN_TEST(test_apply_biome_adaptation_tundra);
    RUN_TEST(test_apply_biome_adaptation_desert);
    
    // Cross-biome comparison tests
    RUN_TEST(test_arctic_vs_desert_in_cold);
    RUN_TEST(test_arctic_vs_desert_in_heat);
    
    END_TEST_GROUP();
}
