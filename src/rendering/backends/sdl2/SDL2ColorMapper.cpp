/**
 * @file SDL2ColorMapper.cpp
 * @brief Implementation of SDL2 color mapping utilities
 * @author Gary Ferguson
 * @date December 2024
 */

#include "../../../../include/rendering/backends/sdl2/SDL2ColorMapper.hpp"
#include "../../../../include/objects/creature/creature.hpp"

//==============================================================================
// Terrain Color Mapping
//==============================================================================

SDL_Color SDL2ColorMapper::terrainToColor(TerrainType terrain) {
    switch (terrain) {
        case TerrainType::DEEP_WATER:
            return deepWaterColor();
        case TerrainType::WATER:
            return waterColor();
        case TerrainType::SHALLOW_WATER:
            return shallowWaterColor();
        case TerrainType::SHALLOW_WATER_2:
            return shallowWater2Color();
        case TerrainType::SAND:
            return sandColor();
        case TerrainType::DESERT_SAND:
            return desertSandColor();
        case TerrainType::PLAINS:
            return plainsColor();
        case TerrainType::SAVANNA:
            return savannaColor();
        case TerrainType::SHORT_GRASS:
            return grassColor();
        case TerrainType::LONG_GRASS:
            return tallGrassColor();
        case TerrainType::FOREST:
            return forestColor();
        case TerrainType::TREES:
            return denseForestColor();
        case TerrainType::MOUNTAIN:
            return mountainColor();
        case TerrainType::MOUNTAIN_2:
            return highMountainColor();
        case TerrainType::MOUNTAIN_3:
            return {60, 60, 60, 255};  // Dark gray for high mountain
        case TerrainType::SNOW:
            return snowColor();
        case TerrainType::PEAKS:
            return {240, 240, 255, 255};  // Bright snow white for peaks
        default:
            return {100, 100, 100, 255};  // Gray default
    }
}

SDL_Color SDL2ColorMapper::elevationToColor(unsigned int elevation) {
    // Map elevation (0-255) to terrain types using same thresholds as tile.cpp
    if (elevation < 40) {
        return deepWaterColor();
    } else if (elevation < 55) {
        return waterColor();
    } else if (elevation < 75) {
        return shallowWaterColor();
    } else if (elevation < 85) {
        return shallowWater2Color();
    } else if (elevation < 95) {
        return sandColor();
    } else if (elevation < 110) {
        return plainsColor();
    } else if (elevation < 130) {
        return savannaColor();
    } else if (elevation < 150) {
        return grassColor();
    } else if (elevation < 170) {
        return tallGrassColor();
    } else if (elevation < 190) {
        return forestColor();
    } else if (elevation < 210) {
        return denseForestColor();
    } else if (elevation < 230) {
        return mountainColor();
    } else if (elevation < 245) {
        return highMountainColor();
    } else {
        return snowColor();
    }
}

//==============================================================================
// Entity Color Mapping
//==============================================================================

SDL_Color SDL2ColorMapper::profileToColor(BehaviorProfile profile) {
    switch (profile) {
        case BehaviorProfile::HUNGRY:
            return hungryColor();
        case BehaviorProfile::THIRSTY:
            return thirstyColor();
        case BehaviorProfile::SLEEPING:
            return sleepyColor();
        case BehaviorProfile::BREEDING:
            return breedingColor();
        case BehaviorProfile::MIGRATING:
            return migratingColor();
        case BehaviorProfile::DEFAULT:
            return wanderingColor();
        default:
            return wanderingColor();
    }
}

SDL_Color SDL2ColorMapper::creatureProfileToColor(const Creature& creature) {
    // Map creature's Profile enum to BehaviorProfile
    Profile creatureProfile = creature.getProfile();
    return legacyProfileToColor(creatureProfile);
}

SDL_Color SDL2ColorMapper::legacyProfileToColor(Profile profile) {
    switch (profile) {
        case Profile::hungry:
            return hungryColor();
        case Profile::thirsty:
            return thirstyColor();
        case Profile::sleep:
            return sleepyColor();
        case Profile::breed:
            return breedingColor();
        case Profile::migrate:
            return migratingColor();
        default:
            return wanderingColor();
    }
}

SDL_Color SDL2ColorMapper::entityToColor(EntityType entity) {
    switch (entity) {
        case EntityType::CREATURE:
            return wanderingColor();  // Default creature color
        case EntityType::FOOD_APPLE:
            return {255, 50, 50, 255};  // Red for apple
        case EntityType::FOOD_BANANA:
            return {255, 255, 50, 255};  // Yellow for banana
        case EntityType::FOOD_CORPSE:
            return {150, 100, 100, 255};  // Brownish for corpse
        case EntityType::SPAWNER:
            return spawnerColor();
        // Phase 2.4: Plant entity colors
        case EntityType::PLANT_BERRY_BUSH:
            return {220, 50, 100, 255};  // Red/Magenta for berry bush
        case EntityType::PLANT_OAK_TREE:
            return {139, 90, 43, 255};   // Brown for oak tree
        case EntityType::PLANT_GRASS:
            return {50, 200, 50, 255};   // Green for grass
        case EntityType::PLANT_THORN_BUSH:
            return {80, 120, 40, 255};   // Dark green/yellow for thorn bush
        case EntityType::PLANT_GENERIC:
            return {120, 220, 100, 255}; // Light green for generic plant
        default:
            return {0, 0, 0, 0};  // Transparent
    }
}

//==============================================================================
// Color Utility Methods
//==============================================================================

SDL_Color SDL2ColorMapper::makeColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    return {r, g, b, a};
}

SDL_Color SDL2ColorMapper::blendColors(SDL_Color c1, SDL_Color c2, float t) {
    // Clamp t to [0, 1]
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    
    float invT = 1.0f - t;
    
    return {
        static_cast<Uint8>(c1.r * invT + c2.r * t),
        static_cast<Uint8>(c1.g * invT + c2.g * t),
        static_cast<Uint8>(c1.b * invT + c2.b * t),
        static_cast<Uint8>(c1.a * invT + c2.a * t)
    };
}

SDL_Color SDL2ColorMapper::lightenColor(SDL_Color color, float factor) {
    // Clamp factor to [0, 1]
    if (factor < 0.0f) factor = 0.0f;
    if (factor > 1.0f) factor = 1.0f;
    
    SDL_Color white = {255, 255, 255, color.a};
    return blendColors(color, white, factor);
}

SDL_Color SDL2ColorMapper::darkenColor(SDL_Color color, float factor) {
    // Clamp factor to [0, 1]
    if (factor < 0.0f) factor = 0.0f;
    if (factor > 1.0f) factor = 1.0f;
    
    SDL_Color black = {0, 0, 0, color.a};
    return blendColors(color, black, factor);
}