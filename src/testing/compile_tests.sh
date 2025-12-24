#!/bin/bash
#
# EcoSim Test Suite Runner
# ========================
# Compiles and runs test files with proper return code handling for CI/CD integration.
#
# Exit codes:
#   0 - All tests passed
#   1 - One or more tests failed (compilation or runtime)
#

set -e  # Exit on first error for compilation

FAILED=0
TESTS_RUN=0
TESTS_PASSED=0

# Colors for output (if terminal supports them)
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# ==============================================================================
# TODO: Regression Tests Needed
# ==============================================================================
# The following regression tests should be added to prevent previously fixed bugs:
#
# CREATURE-001: Mutation return values
#   - Test that geneCreep() returns the mutated value, not 0
#   - Test that spliceGene() returns valid gene values
#
# CREATURE-002: A* pathfinding
#   - Test that Navigator::astar() returns valid paths
#   - Test path finding on various terrain types
#
# CREATURE-003: setFatigue clamping
#   - Test that setFatigue clamps values to [0,1] range
#   - Test edge cases at 0 and 1 boundaries
#
# WORLD-001: Terrain generation
#   - Test that World constructor properly initializes terrain
#   - Test terrain level bounds checking
#
# STATS-007/008/009: Statistics accumulation
#   - Test that incBirths/incDeaths/incFood properly increment counters
#   - Test that stats are properly accumulated over multiple calls
#
# ==============================================================================

echo "=== EcoSim Test Suite ==="
echo ""

# Include paths for genetics headers
INCLUDE_PATHS="-I../../include"

# Common genetics source files needed when creature/world depends on genetics
GENETICS_SOURCES=(
    "../genetics/core/Gene.cpp"
    "../genetics/core/Chromosome.cpp"
    "../genetics/core/Genome.cpp"
    "../genetics/core/GeneRegistry.cpp"
    "../genetics/core/GeneticTypes.cpp"
    "../genetics/expression/Phenotype.cpp"
    "../genetics/expression/EnergyBudget.cpp"
    "../genetics/expression/EnvironmentState.cpp"
    "../genetics/expression/OrganismState.cpp"
    "../genetics/expression/PhenotypeCache.cpp"
    "../genetics/defaults/DefaultGenes.cpp"
    "../genetics/defaults/PlantGenes.cpp"
    "../genetics/defaults/UniversalGenes.cpp"
    "../genetics/organisms/Plant.cpp"
    "../genetics/organisms/PlantFactory.cpp"
    "../genetics/organisms/CreatureFactory.cpp"
    "../genetics/interactions/FeedingInteraction.cpp"
    "../genetics/interactions/SeedDispersal.cpp"
    "../genetics/interactions/CoevolutionTracker.cpp"
    "../genetics/migration/LegacyGenomeAdapter.cpp"
    "../logging/Logger.cpp"
)

# Compile and run a test
compile_and_run() {
    local test_name="$1"
    local source_file="$2"
    shift 2
    local source_files="$@"
    
    echo "----------------------------------------"
    echo "Testing: $test_name"
    echo "----------------------------------------"
    
    TESTS_RUN=$((TESTS_RUN + 1))
    
    # Compile
    echo "  Compiling..."
    if ! g++ -std=c++17 $INCLUDE_PATHS "$source_file" $source_files -o "test_${test_name}" 2>&1; then
        echo -e "  ${RED}COMPILE FAILED: $test_name${NC}"
        FAILED=1
        return 1
    fi
    
    # Run
    echo "  Running..."
    if ./test_${test_name}; then
        echo -e "  ${GREEN}PASSED: $test_name${NC}"
        TESTS_PASSED=$((TESTS_PASSED + 1))
        rm -f "test_${test_name}"
        return 0
    else
        echo -e "  ${RED}TEST FAILED: $test_name${NC}"
        FAILED=1
        rm -f "test_${test_name}"
        return 1
    fi
}

# Change to testing directory
cd "$(dirname "$0")"

# ==============================================================================
# Run Tests
# ==============================================================================

# Calendar Test - standalone, no dependencies
compile_and_run "calendar" "testCalendar.cpp" \
    "../calendar.cpp"

# NOTE: Legacy creature and objects tests removed (2024-12-23)
# These tests used the old genome system (include/objects/creature/genome.hpp)
# For genetics tests, see src/testing/genetics/ which uses the new system

# File Test - tests save/load functionality
compile_and_run "file" "fileTest.cpp" \
    "../objects/gameObject.cpp" \
    "../objects/food.cpp" \
    "../objects/spawner.cpp" \
    "../objects/creature/creature.cpp" \
    "../objects/creature/genome.cpp" \
    "../world/tile.cpp" \
    "../world/world.cpp" \
    "../world/SimplexNoise.cpp" \
    "../statistics/statistics.cpp" \
    "../statistics/genomeStats.cpp" \
    "../fileHandling.cpp" \
    "../objects/creature/navigator.cpp" \
    "../calendar.cpp" \
    "${GENETICS_SOURCES[@]}"

# World Test - tests world generation and tile functionality
compile_and_run "world" "testWorld.cpp" \
    "../objects/gameObject.cpp" \
    "../objects/food.cpp" \
    "../objects/spawner.cpp" \
    "../objects/creature/creature.cpp" \
    "../objects/creature/genome.cpp" \
    "../world/tile.cpp" \
    "../world/world.cpp" \
    "../world/SimplexNoise.cpp" \
    "../statistics/statistics.cpp" \
    "../statistics/genomeStats.cpp" \
    "../objects/creature/navigator.cpp" \
    "../calendar.cpp" \
    "${GENETICS_SOURCES[@]}"

# Statistics Test - tests statistics tracking functionality
compile_and_run "statistics" "testStatistics.cpp" \
    "../objects/gameObject.cpp" \
    "../objects/food.cpp" \
    "../objects/spawner.cpp" \
    "../objects/creature/creature.cpp" \
    "../objects/creature/genome.cpp" \
    "../world/tile.cpp" \
    "../world/world.cpp" \
    "../world/SimplexNoise.cpp" \
    "../statistics/statistics.cpp" \
    "../statistics/genomeStats.cpp" \
    "../objects/creature/navigator.cpp" \
    "../calendar.cpp" \
    "${GENETICS_SOURCES[@]}"

# ==============================================================================
# Summary
# ==============================================================================

echo ""
echo "========================================"
echo "Test Summary"
echo "========================================"
echo "Tests run:    $TESTS_RUN"
echo "Tests passed: $TESTS_PASSED"
echo "Tests failed: $((TESTS_RUN - TESTS_PASSED))"
echo ""

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests PASSED!${NC}"
    exit 0
else
    echo -e "${RED}Some tests FAILED!${NC}"
    exit 1
fi
