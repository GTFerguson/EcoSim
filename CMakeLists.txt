cmake_minimum_required(VERSION 3.16)
project(EcoSim VERSION 1.0.0 LANGUAGES CXX)

# C++17 required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to Release build if not specified
# Release builds have significant performance improvements (5-10x faster)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Build options
option(ECOSIM_BUILD_TESTS "Build test suite" ON)
option(ECOSIM_USE_SDL2 "Build with SDL2/ImGui support" ON)
option(ECOSIM_USE_NCURSES "Build with NCurses support" ON)

# Include helper modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(CompilerWarnings)

# Find dependencies
find_package(PkgConfig REQUIRED)

if(ECOSIM_USE_NCURSES)
    pkg_check_modules(NCURSES REQUIRED ncurses)
endif()

if(ECOSIM_USE_SDL2)
    find_package(SDL2 REQUIRED)
endif()

# ==============================================================================
# Library: ecosim_logging (no dependencies)
# ==============================================================================
add_library(ecosim_logging STATIC
    src/logging/Logger.cpp
)
target_include_directories(ecosim_logging PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
add_compiler_warnings(ecosim_logging)

# ==============================================================================
# Library: ecosim_genetics (depends on ecosim_logging)
# ==============================================================================
file(GLOB GENETICS_CORE_SOURCES src/genetics/core/*.cpp)
file(GLOB GENETICS_EXPRESSION_SOURCES src/genetics/expression/*.cpp)
file(GLOB GENETICS_ORGANISMS_SOURCES src/genetics/organisms/*.cpp)
file(GLOB GENETICS_CLASSIFICATION_SOURCES src/genetics/classification/*.cpp)
file(GLOB GENETICS_INTERACTIONS_SOURCES src/genetics/interactions/*.cpp)
file(GLOB GENETICS_SYSTEMS_SOURCES src/genetics/systems/*.cpp)
file(GLOB GENETICS_BEHAVIORS_SOURCES src/genetics/behaviors/*.cpp)

add_library(ecosim_genetics STATIC
    ${GENETICS_CORE_SOURCES}
    src/genetics/defaults/PlantGenes.cpp
    src/genetics/defaults/UniversalGenes.cpp
    ${GENETICS_EXPRESSION_SOURCES}
    ${GENETICS_ORGANISMS_SOURCES}
    ${GENETICS_CLASSIFICATION_SOURCES}
    ${GENETICS_INTERACTIONS_SOURCES}
    ${GENETICS_SYSTEMS_SOURCES}
    ${GENETICS_BEHAVIORS_SOURCES}
)
target_include_directories(ecosim_genetics PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/external
)
target_link_libraries(ecosim_genetics PUBLIC ecosim_logging)
add_compiler_warnings(ecosim_genetics)

# ==============================================================================
# Library: ecosim_world (depends on ecosim_genetics)
# ==============================================================================
file(GLOB WORLD_SOURCES src/world/*.cpp)

add_library(ecosim_world STATIC
    ${WORLD_SOURCES}
)
target_include_directories(ecosim_world PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/external
)
target_link_libraries(ecosim_world PUBLIC ecosim_genetics)
add_compiler_warnings(ecosim_world)

# ==============================================================================
# Library: ecosim_core (depends on ecosim_world, ecosim_genetics)
# ==============================================================================
file(GLOB STATISTICS_SOURCES src/statistics/*.cpp)

add_library(ecosim_core STATIC
    src/objects/gameObject.cpp
    src/objects/creature/creature.cpp
    src/objects/creature/navigator.cpp
    src/objects/creature/CreatureCombat.cpp
    src/objects/creature/CreatureScent.cpp
    src/objects/creature/CreaturePlantInteraction.cpp
    src/objects/creature/CreatureResourceSearch.cpp
    src/objects/creature/CreatureSerialization.cpp
    src/calendar.cpp
    src/fileHandling.cpp
    ${STATISTICS_SOURCES}
)
target_include_directories(ecosim_core PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(ecosim_core PUBLIC ecosim_world ecosim_genetics)

add_compiler_warnings(ecosim_core)

# ==============================================================================
# Library: ecosim_rendering (depends on ecosim_core)
# ==============================================================================
set(RENDERING_SOURCES
    src/rendering/RenderSystem.cpp
)

# NCurses backend sources (always included for now)
file(GLOB NCURSES_BACKEND_SOURCES src/rendering/backends/ncurses/*.cpp)
list(APPEND RENDERING_SOURCES ${NCURSES_BACKEND_SOURCES})

# SDL2 backend sources (conditional)
if(ECOSIM_USE_SDL2)
    file(GLOB SDL2_BACKEND_SOURCES src/rendering/backends/sdl2/*.cpp)
    list(APPEND RENDERING_SOURCES ${SDL2_BACKEND_SOURCES})
endif()

add_library(ecosim_rendering STATIC
    ${RENDERING_SOURCES}
)
target_include_directories(ecosim_rendering PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(ecosim_rendering PUBLIC ecosim_core)

# NCurses linking
if(ECOSIM_USE_NCURSES)
    target_include_directories(ecosim_rendering PRIVATE ${NCURSES_INCLUDE_DIRS})
    target_link_libraries(ecosim_rendering PUBLIC ${NCURSES_LIBRARIES})
    target_compile_definitions(ecosim_rendering PUBLIC ECOSIM_HAS_NCURSES=1)
endif()

# SDL2 linking
if(ECOSIM_USE_SDL2)
    target_link_libraries(ecosim_rendering PUBLIC SDL2::SDL2)
    target_compile_definitions(ecosim_rendering PUBLIC ECOSIM_HAS_SDL2=1)
endif()

add_compiler_warnings(ecosim_rendering)

# ==============================================================================
# Library: imgui (if SDL2 enabled)
# ==============================================================================
if(ECOSIM_USE_SDL2)
    add_library(imgui STATIC
        external/imgui/imgui.cpp
        external/imgui/imgui_demo.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/backends/imgui_impl_sdl2.cpp
        external/imgui/backends/imgui_impl_sdlrenderer2.cpp
    )
    target_include_directories(imgui PUBLIC
        ${PROJECT_SOURCE_DIR}/external/imgui
        ${PROJECT_SOURCE_DIR}/external/imgui/backends
    )
    target_link_libraries(imgui PUBLIC SDL2::SDL2)
    
    # Add ImGui compile definition to rendering library
    target_compile_definitions(ecosim_rendering PUBLIC ECOSIM_HAS_IMGUI=1)
    target_include_directories(ecosim_rendering PUBLIC
        ${PROJECT_SOURCE_DIR}/external/imgui
        ${PROJECT_SOURCE_DIR}/external/imgui/backends
    )
endif()

# ==============================================================================
# Executable: EcoSim
# ==============================================================================
add_executable(EcoSim
    src/main.cpp
    src/window.cpp
)
target_include_directories(EcoSim PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(EcoSim PRIVATE
    ecosim_rendering
    ecosim_core
)

if(ECOSIM_USE_SDL2)
    target_link_libraries(EcoSim PRIVATE imgui)
endif()

if(ECOSIM_USE_NCURSES)
    target_link_libraries(EcoSim PRIVATE ${NCURSES_LIBRARIES})
endif()

add_compiler_warnings(EcoSim)

# ==============================================================================
# Tests (conditional)
# ==============================================================================
if(ECOSIM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(src/testing)
endif()

# ==============================================================================
# Summary
# ==============================================================================
message(STATUS "")
message(STATUS "EcoSim Build Configuration:")
message(STATUS "  Version:        ${PROJECT_VERSION}")
message(STATUS "  C++ Standard:   ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Tests:    ${ECOSIM_BUILD_TESTS}")
message(STATUS "  Use NCurses:    ${ECOSIM_USE_NCURSES}")
message(STATUS "  Use SDL2:       ${ECOSIM_USE_SDL2}")
message(STATUS "")
