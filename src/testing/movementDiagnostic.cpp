/**
 * @file movementDiagnostic.cpp
 * @brief Diagnostic test for creature positioning and rendering after float movement implementation
 * 
 * This test verifies:
 * 1. Creature spawn positions are valid (not NaN, not 0,0 unless intended)
 * 2. _worldX, _worldY are properly initialized
 * 3. tileX(), tileY() correctly derive from _worldX, _worldY
 * 4. getWorldX(), getWorldY() return valid values for rendering
 * 5. getMovementSpeed() returns valid values
 * 6. Viewport bounds checks work correctly
 */

#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <cassert>
#include <iomanip>

#include "../../include/objects/creature/creature.hpp"
#include "../../include/objects/creature/genome.hpp"
#include "../../include/genetics/organisms/CreatureFactory.hpp"
#include "../../include/genetics/core/GeneRegistry.hpp"
#include "../../include/rendering/RenderTypes.hpp"

// Test result tracking
static int testsPassed = 0;
static int testsFailed = 0;

void printTestHeader(const std::string& testName) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "TEST: " << testName << "\n";
    std::cout << std::string(60, '=') << "\n";
}

void printResult(const std::string& test, bool passed) {
    if (passed) {
        std::cout << "[PASS] " << test << "\n";
        testsPassed++;
    } else {
        std::cout << "[FAIL] " << test << "\n";
        testsFailed++;
    }
}

bool isValidFloat(float f) {
    return !std::isnan(f) && !std::isinf(f);
}

bool isValidPosition(float x, float y, int worldWidth, int worldHeight) {
    return isValidFloat(x) && isValidFloat(y) &&
           x >= 0.0f && x < static_cast<float>(worldWidth) &&
           y >= 0.0f && y < static_cast<float>(worldHeight);
}

//==============================================================================
// Test 1: Basic Creature Position Initialization
//==============================================================================
void testBasicPositionInitialization() {
    printTestHeader("Basic Position Initialization");
    
    // Test positions
    std::vector<std::pair<int, int>> testPositions = {
        {0, 0},
        {50, 50},
        {100, 200},
        {499, 499}
    };
    
    for (const auto& [x, y] : testPositions) {
        Genome g;
        Creature c(x, y, g);
        
        std::cout << "Testing position (" << x << ", " << y << "):\n";
        std::cout << "  _worldX = " << c.getWorldX() << " (expected: " << x << ".0)\n";
        std::cout << "  _worldY = " << c.getWorldY() << " (expected: " << y << ".0)\n";
        std::cout << "  tileX() = " << c.tileX() << " (expected: " << x << ")\n";
        std::cout << "  tileY() = " << c.tileY() << " (expected: " << y << ")\n";
        std::cout << "  getX()  = " << c.getX() << " (expected: " << x << ")\n";
        std::cout << "  getY()  = " << c.getY() << " (expected: " << y << ")\n";
        
        bool worldXCorrect = std::abs(c.getWorldX() - static_cast<float>(x)) < 0.001f;
        bool worldYCorrect = std::abs(c.getWorldY() - static_cast<float>(y)) < 0.001f;
        bool tileXCorrect = c.tileX() == x;
        bool tileYCorrect = c.tileY() == y;
        bool getXCorrect = c.getX() == x;
        bool getYCorrect = c.getY() == y;
        
        printResult("worldX matches input", worldXCorrect);
        printResult("worldY matches input", worldYCorrect);
        printResult("tileX() matches input", tileXCorrect);
        printResult("tileY() matches input", tileYCorrect);
        printResult("getX() matches input", getXCorrect);
        printResult("getY() matches input", getYCorrect);
    }
}

//==============================================================================
// Test 2: CreatureFactory Position Initialization
//==============================================================================
void testCreatureFactoryPositions() {
    printTestHeader("CreatureFactory Position Initialization");
    
    auto registry = std::make_shared<EcoSim::Genetics::GeneRegistry>();
    EcoSim::Genetics::CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Test createRandom
    {
        int x = 100, y = 200;
        Creature c = factory.createRandom(x, y);
        
        std::cout << "createRandom(" << x << ", " << y << "):\n";
        std::cout << "  getWorldX() = " << c.getWorldX() << "\n";
        std::cout << "  getWorldY() = " << c.getWorldY() << "\n";
        std::cout << "  tileX() = " << c.tileX() << "\n";
        std::cout << "  tileY() = " << c.tileY() << "\n";
        
        bool worldXCorrect = std::abs(c.getWorldX() - static_cast<float>(x)) < 0.001f;
        bool worldYCorrect = std::abs(c.getWorldY() - static_cast<float>(y)) < 0.001f;
        
        printResult("createRandom worldX matches", worldXCorrect);
        printResult("createRandom worldY matches", worldYCorrect);
    }
    
    // Test createFromTemplate for each template type
    std::vector<std::string> templates = {"grazer", "browser", "hunter", "forager", "scavenger"};
    
    for (const auto& templateName : templates) {
        int x = 150, y = 250;
        Creature c = factory.createFromTemplate(templateName, x, y);
        
        std::cout << "createFromTemplate(\"" << templateName << "\", " << x << ", " << y << "):\n";
        std::cout << "  getWorldX() = " << c.getWorldX() << "\n";
        std::cout << "  getWorldY() = " << c.getWorldY() << "\n";
        
        bool worldXCorrect = std::abs(c.getWorldX() - static_cast<float>(x)) < 0.001f;
        bool worldYCorrect = std::abs(c.getWorldY() - static_cast<float>(y)) < 0.001f;
        
        printResult(templateName + " worldX matches", worldXCorrect);
        printResult(templateName + " worldY matches", worldYCorrect);
    }
}

//==============================================================================
// Test 3: setWorldPosition Function
//==============================================================================
void testSetWorldPosition() {
    printTestHeader("setWorldPosition Function");
    
    Genome g;
    Creature c(10, 20, g);
    
    std::cout << "Initial position:\n";
    std::cout << "  worldX = " << c.getWorldX() << ", worldY = " << c.getWorldY() << "\n";
    
    // Test setWorldPosition with various values
    std::vector<std::pair<float, float>> testPositions = {
        {50.5f, 100.5f},
        {25.25f, 75.75f},
        {0.0f, 0.0f},
        {499.9f, 499.9f}
    };
    
    for (const auto& [newX, newY] : testPositions) {
        c.setWorldPosition(newX, newY);
        
        std::cout << "After setWorldPosition(" << newX << ", " << newY << "):\n";
        std::cout << "  getWorldX() = " << c.getWorldX() << "\n";
        std::cout << "  getWorldY() = " << c.getWorldY() << "\n";
        std::cout << "  tileX() = " << c.tileX() << " (expected: " << static_cast<int>(newX) << ")\n";
        std::cout << "  tileY() = " << c.tileY() << " (expected: " << static_cast<int>(newY) << ")\n";
        
        bool worldXCorrect = std::abs(c.getWorldX() - newX) < 0.001f;
        bool worldYCorrect = std::abs(c.getWorldY() - newY) < 0.001f;
        bool tileXCorrect = c.tileX() == static_cast<int>(newX);
        bool tileYCorrect = c.tileY() == static_cast<int>(newY);
        
        printResult("setWorldPosition worldX", worldXCorrect);
        printResult("setWorldPosition worldY", worldYCorrect);
        printResult("setWorldPosition tileX", tileXCorrect);
        printResult("setWorldPosition tileY", tileYCorrect);
    }
}

//==============================================================================
// Test 4: Movement Speed Calculation
//==============================================================================
void testMovementSpeed() {
    printTestHeader("Movement Speed Calculation");
    
    auto registry = std::make_shared<EcoSim::Genetics::GeneRegistry>();
    EcoSim::Genetics::CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    std::vector<std::string> templates = {"grazer", "browser", "hunter", "forager", "scavenger"};
    
    for (const auto& templateName : templates) {
        Creature c = factory.createFromTemplate(templateName, 100, 100);
        float speed = c.getMovementSpeed();
        
        std::cout << templateName << " movement speed: " << speed << "\n";
        
        bool validSpeed = isValidFloat(speed) && speed > 0.0f && speed < 10.0f;
        printResult(templateName + " has valid movement speed", validSpeed);
    }
}

//==============================================================================
// Test 5: Rendering Bounds Check Simulation
//==============================================================================
void testRenderingBoundsCheck() {
    printTestHeader("Rendering Bounds Check Simulation");
    
    // Simulate viewport settings (matching SDL2Renderer logic)
    const int screenWidth = 1920;
    const int screenHeight = 1080;
    const int tileSize = 16;
    
    Viewport viewport;
    viewport.originX = 0;
    viewport.originY = 0;
    viewport.width = screenWidth / tileSize;   // 120 tiles
    viewport.height = screenHeight / tileSize; // 67 tiles
    viewport.screenX = 0;
    viewport.screenY = 0;
    
    std::cout << "Viewport settings:\n";
    std::cout << "  originX: " << viewport.originX << ", originY: " << viewport.originY << "\n";
    std::cout << "  width: " << viewport.width << ", height: " << viewport.height << "\n";
    std::cout << "  screenX: " << viewport.screenX << ", screenY: " << viewport.screenY << "\n";
    
    // Test creatures at various positions
    std::vector<std::pair<int, int>> testPositions = {
        {0, 0},         // Top-left corner
        {50, 50},       // Middle-ish
        {100, 60},      // Within viewport
        {120, 67},      // Just outside viewport
        {200, 200},     // Far outside
        {-1, -1}        // Invalid negative
    };
    
    for (const auto& [x, y] : testPositions) {
        Genome g;
        Creature c(std::max(0, x), std::max(0, y), g);  // Clamp to valid range for construction
        
        if (x < 0 || y < 0) {
            std::cout << "Skipping negative position test (invalid construction)\n";
            continue;
        }
        
        float worldX = c.getWorldX();
        float worldY = c.getWorldY();
        int tileX = static_cast<int>(worldX);
        int tileY = static_cast<int>(worldY);
        
        // Calculate bounds (matching SDL2Renderer::renderCreatures)
        int xRange = viewport.originX + viewport.width;
        int yRange = viewport.originY + viewport.height;
        
        // Bounds check (matching SDL2Renderer::renderCreatures)
        bool inBounds = (tileX >= viewport.originX && tileX < xRange &&
                         tileY >= viewport.originY && tileY < yRange);
        
        // Calculate screen position (matching SDL2Renderer::renderCreatures)
        int baseScreenX = static_cast<int>(viewport.screenX) * tileSize;
        int baseScreenY = static_cast<int>(viewport.screenY) * tileSize;
        float screenX = baseScreenX + (worldX - viewport.originX) * tileSize;
        float screenY = baseScreenY + (worldY - viewport.originY) * tileSize;
        int pixelX = static_cast<int>(screenX);
        int pixelY = static_cast<int>(screenY);
        
        std::cout << "\nPosition (" << x << ", " << y << "):\n";
        std::cout << "  worldX/Y: (" << worldX << ", " << worldY << ")\n";
        std::cout << "  tileX/Y: (" << tileX << ", " << tileY << ")\n";
        std::cout << "  inBounds: " << (inBounds ? "YES" : "NO") << "\n";
        std::cout << "  screenX/Y: (" << screenX << ", " << screenY << ")\n";
        std::cout << "  pixelX/Y: (" << pixelX << ", " << pixelY << ")\n";
        
        // Verify screen position is valid for in-bounds creatures
        if (inBounds) {
            bool validScreenPos = pixelX >= 0 && pixelX < screenWidth &&
                                  pixelY >= 0 && pixelY < screenHeight;
            printResult("In-bounds creature has valid screen position", validScreenPos);
        }
    }
}

//==============================================================================
// Test 6: NaN/Infinity Check
//==============================================================================
void testNaNInfinityCheck() {
    printTestHeader("NaN/Infinity Check");
    
    auto registry = std::make_shared<EcoSim::Genetics::GeneRegistry>();
    EcoSim::Genetics::CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    // Create many creatures and check for NaN/Infinity
    std::mt19937 rng(12345);
    std::uniform_int_distribution<int> posDist(0, 499);
    
    int nanCount = 0;
    int infCount = 0;
    int zeroCount = 0;
    const int numCreatures = 100;
    
    for (int i = 0; i < numCreatures; i++) {
        int x = posDist(rng);
        int y = posDist(rng);
        
        Creature c = factory.createRandom(x, y);
        
        float worldX = c.getWorldX();
        float worldY = c.getWorldY();
        
        if (std::isnan(worldX) || std::isnan(worldY)) {
            std::cout << "NaN detected at creature " << i << ": (" << worldX << ", " << worldY << ")\n";
            nanCount++;
        }
        if (std::isinf(worldX) || std::isinf(worldY)) {
            std::cout << "Infinity detected at creature " << i << ": (" << worldX << ", " << worldY << ")\n";
            infCount++;
        }
        if (worldX == 0.0f && worldY == 0.0f && x != 0 && y != 0) {
            std::cout << "Unexpected (0,0) at creature " << i << " (expected: " << x << ", " << y << ")\n";
            zeroCount++;
        }
    }
    
    std::cout << "\nResults for " << numCreatures << " creatures:\n";
    std::cout << "  NaN count: " << nanCount << "\n";
    std::cout << "  Infinity count: " << infCount << "\n";
    std::cout << "  Unexpected (0,0) count: " << zeroCount << "\n";
    
    printResult("No NaN values", nanCount == 0);
    printResult("No Infinity values", infCount == 0);
    printResult("No unexpected (0,0) values", zeroCount == 0);
}

//==============================================================================
// Test 7: Position after enableNewGenetics()
//==============================================================================
void testPositionAfterEnableNewGenetics() {
    printTestHeader("Position After enableNewGenetics()");
    
    // This tests that enableNewGenetics() doesn't corrupt position
    Genome g;
    int x = 123, y = 456;
    Creature c(x, y, g);
    
    std::cout << "Before enableNewGenetics():\n";
    std::cout << "  worldX = " << c.getWorldX() << ", worldY = " << c.getWorldY() << "\n";
    
    float beforeX = c.getWorldX();
    float beforeY = c.getWorldY();
    
    c.enableNewGenetics(true);
    
    std::cout << "After enableNewGenetics(true):\n";
    std::cout << "  worldX = " << c.getWorldX() << ", worldY = " << c.getWorldY() << "\n";
    
    float afterX = c.getWorldX();
    float afterY = c.getWorldY();
    
    bool positionPreserved = std::abs(beforeX - afterX) < 0.001f &&
                             std::abs(beforeY - afterY) < 0.001f;
    
    printResult("Position preserved after enableNewGenetics()", positionPreserved);
}

//==============================================================================
// Test 8: Creature Vector Simulation (like main.cpp populateWorld)
//==============================================================================
void testCreatureVectorPopulation() {
    printTestHeader("Creature Vector Population (simulating populateWorld)");
    
    auto registry = std::make_shared<EcoSim::Genetics::GeneRegistry>();
    EcoSim::Genetics::CreatureFactory factory(registry);
    factory.registerDefaultTemplates();
    
    std::vector<Creature> creatures;
    
    // Simulate populateWorld from main.cpp
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> colDis(0, 499);
    std::uniform_int_distribution<int> rowDis(0, 499);
    
    const int numCreatures = 20;
    
    for (int i = 0; i < numCreatures; i++) {
        int x = colDis(rng);
        int y = rowDis(rng);
        
        Creature newC = factory.createRandom(x, y);
        creatures.push_back(std::move(newC));
    }
    
    std::cout << "Created " << creatures.size() << " creatures\n\n";
    
    // Check all creatures have valid positions
    int validCount = 0;
    int invalidCount = 0;
    
    for (size_t i = 0; i < creatures.size(); i++) {
        const Creature& c = creatures[i];
        float worldX = c.getWorldX();
        float worldY = c.getWorldY();
        
        std::cout << "Creature " << i << ": worldX=" << std::setw(8) << worldX 
                  << ", worldY=" << std::setw(8) << worldY
                  << ", tileX=" << std::setw(4) << c.tileX()
                  << ", tileY=" << std::setw(4) << c.tileY() << "\n";
        
        if (isValidPosition(worldX, worldY, 500, 500)) {
            validCount++;
        } else {
            invalidCount++;
            std::cout << "  ^^^ INVALID POSITION!\n";
        }
    }
    
    std::cout << "\nValid positions: " << validCount << "/" << creatures.size() << "\n";
    std::cout << "Invalid positions: " << invalidCount << "/" << creatures.size() << "\n";
    
    printResult("All creatures have valid positions", invalidCount == 0);
}

//==============================================================================
// Main
//==============================================================================
int main() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║     MOVEMENT DIAGNOSTIC TEST - Float Position System       ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";
    
    // Initialize creature gene registry (required for genetics-enabled creatures)
    Creature::initializeGeneRegistry();
    
    // Run all tests
    testBasicPositionInitialization();
    testCreatureFactoryPositions();
    testSetWorldPosition();
    testMovementSpeed();
    testRenderingBoundsCheck();
    testNaNInfinityCheck();
    testPositionAfterEnableNewGenetics();
    testCreatureVectorPopulation();
    
    // Summary
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                      TEST SUMMARY                          ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";
    std::cout << "Tests Passed: " << testsPassed << "\n";
    std::cout << "Tests Failed: " << testsFailed << "\n";
    std::cout << "Total Tests:  " << (testsPassed + testsFailed) << "\n";
    
    if (testsFailed == 0) {
        std::cout << "\n✓ ALL TESTS PASSED - Position system appears correct\n";
        std::cout << "  If creatures are still invisible, check:\n";
        std::cout << "  1. Viewport origin vs creature positions (are creatures in view?)\n";
        std::cout << "  2. Tile size and screen calculations\n";
        std::cout << "  3. Creature color (might be rendering but invisible)\n";
        std::cout << "  4. Z-ordering (creatures rendering behind terrain?)\n";
    } else {
        std::cout << "\n✗ SOME TESTS FAILED - Position system has issues\n";
    }
    
    return testsFailed > 0 ? 1 : 0;
}
