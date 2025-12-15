/**
 * @file RenderSystem.cpp
 * @brief Implementation of RenderSystem facade
 * @author Gary Ferguson
 * @date December 2024
 * 
 * This file implements the RenderSystem class which manages rendering backends
 * and provides factory methods for creating renderer and input handler instances.
 */

#include "../../include/rendering/RenderSystem.hpp"
#include <stdexcept>
#include <cstdio>

// Compile-time flags for backend availability
// These will be defined by the build system when backends are available
#ifndef ECOSIM_HAS_NCURSES
#define ECOSIM_HAS_NCURSES 1  // Always available for now (existing system)
#endif

#ifndef ECOSIM_HAS_SDL2
#define ECOSIM_HAS_SDL2 0      // Not yet implemented
#endif

//==============================================================================
// Backend-specific includes
//==============================================================================

#if ECOSIM_HAS_NCURSES
#include "../../include/rendering/backends/ncurses/NCursesRenderer.hpp"
#include "../../include/rendering/backends/ncurses/NCursesInputHandler.hpp"
#endif

#if ECOSIM_HAS_SDL2
#include "../../include/rendering/backends/sdl2/SDL2Renderer.hpp"
#include "../../include/rendering/backends/sdl2/SDL2InputHandler.hpp"
#endif

//==============================================================================
// Static member initialization
//==============================================================================

RenderSystem* RenderSystem::_instance = nullptr;

//==============================================================================
// Constructor / Destructor
//==============================================================================

RenderSystem::RenderSystem()
    : _renderer(nullptr)
    , _inputHandler(nullptr)
    , _backend(RenderBackend::BACKEND_AUTO)
    , _initialized(false) {
}

RenderSystem::~RenderSystem() {
    if (_initialized) {
        cleanup();
    }
}

//==============================================================================
// Static Singleton Interface
//==============================================================================

bool RenderSystem::initialize(RenderBackend backend) {
    RenderConfig config;
    config.backend = backend;
    return initialize(config);
}

bool RenderSystem::initialize(const RenderConfig& config) {
    if (_instance != nullptr) {
        // Already initialized - could return true or re-initialize
        // For safety, we'll require explicit shutdown first
        std::fprintf(stderr, "RenderSystem: Already initialized. Call shutdown() first.\n");
        return false;
    }
    
    _instance = new RenderSystem();
    
    if (!_instance->init(config)) {
        delete _instance;
        _instance = nullptr;
        return false;
    }
    
    return true;
}

void RenderSystem::shutdown() {
    if (_instance != nullptr) {
        _instance->cleanup();
        delete _instance;
        _instance = nullptr;
    }
}

RenderSystem& RenderSystem::getInstance() {
    if (_instance == nullptr) {
        throw std::runtime_error("RenderSystem: Not initialized. Call initialize() first.");
    }
    return *_instance;
}

bool RenderSystem::isInitialized() {
    return _instance != nullptr && _instance->_initialized;
}

//==============================================================================
// Instance Interface
//==============================================================================

bool RenderSystem::init(const RenderConfig& config) {
    if (_initialized) {
        std::fprintf(stderr, "RenderSystem: Already initialized.\n");
        return false;
    }
    
    // Determine which backend to use
    RenderBackend selectedBackend = config.backend;
    
    if (selectedBackend == RenderBackend::BACKEND_AUTO) {
        selectedBackend = getBestAvailableBackend();
    }
    
    // Check if selected backend is available
    if (!isBackendAvailable(selectedBackend)) {
        std::fprintf(stderr, "RenderSystem: Backend '%s' is not available.\n",
                     getBackendName(selectedBackend));
        return false;
    }
    
    // Create renderer
    _renderer = createRenderer(selectedBackend);
    if (_renderer == nullptr) {
        std::fprintf(stderr, "RenderSystem: Failed to create renderer.\n");
        return false;
    }
    
    // Initialize renderer
    if (!_renderer->initialize()) {
        std::fprintf(stderr, "RenderSystem: Renderer initialization failed.\n");
        destroyRenderer();
        return false;
    }
    
    // Create input handler
    _inputHandler = createInputHandler(selectedBackend);
    if (_inputHandler == nullptr) {
        std::fprintf(stderr, "RenderSystem: Failed to create input handler.\n");
        _renderer->shutdown();
        destroyRenderer();
        return false;
    }
    
    // Initialize input handler
    if (!_inputHandler->initialize()) {
        std::fprintf(stderr, "RenderSystem: Input handler initialization failed.\n");
        _renderer->shutdown();
        destroyRenderer();
        destroyInputHandler();
        return false;
    }
    
    // Apply configuration
    if (config.enableMouse && _inputHandler->supportsMouseInput()) {
        _inputHandler->enableMouseInput(true);
    }
    
    _backend = selectedBackend;
    _initialized = true;
    
    return true;
}

void RenderSystem::cleanup() {
    if (!_initialized) {
        return;
    }
    
    // Shutdown input handler
    if (_inputHandler != nullptr) {
        _inputHandler->shutdown();
        destroyInputHandler();
    }
    
    // Shutdown renderer
    if (_renderer != nullptr) {
        _renderer->shutdown();
        destroyRenderer();
    }
    
    _initialized = false;
}

bool RenderSystem::isReady() const {
    return _initialized && _renderer != nullptr && _inputHandler != nullptr;
}

IRenderer& RenderSystem::getRenderer() {
    if (!_initialized || _renderer == nullptr) {
        throw std::runtime_error("RenderSystem: Not initialized or no renderer available.");
    }
    return *_renderer;
}

const IRenderer& RenderSystem::getRenderer() const {
    if (!_initialized || _renderer == nullptr) {
        throw std::runtime_error("RenderSystem: Not initialized or no renderer available.");
    }
    return *_renderer;
}

IInputHandler& RenderSystem::getInputHandler() {
    if (!_initialized || _inputHandler == nullptr) {
        throw std::runtime_error("RenderSystem: Not initialized or no input handler available.");
    }
    return *_inputHandler;
}

const IInputHandler& RenderSystem::getInputHandler() const {
    if (!_initialized || _inputHandler == nullptr) {
        throw std::runtime_error("RenderSystem: Not initialized or no input handler available.");
    }
    return *_inputHandler;
}

RenderBackend RenderSystem::getBackend() const {
    return _backend;
}

RendererCapabilities RenderSystem::getCapabilities() const {
    if (_renderer != nullptr) {
        return _renderer->getCapabilities();
    }
    return RendererCapabilities();
}

//==============================================================================
// Backend Detection
//==============================================================================

bool RenderSystem::isBackendAvailable(RenderBackend backend) {
    switch (backend) {
        case RenderBackend::BACKEND_NCURSES:
#if ECOSIM_HAS_NCURSES
            return true;
#else
            return false;
#endif
            
        case RenderBackend::BACKEND_SDL2:
#if ECOSIM_HAS_SDL2
            return true;
#else
            return false;
#endif
            
        case RenderBackend::BACKEND_AUTO:
            // Auto is always "available" as it selects the best available
            return true;
            
        default:
            return false;
    }
}

RenderBackend RenderSystem::getBestAvailableBackend() {
    // Prefer SDL2 if available (better graphics support)
#if ECOSIM_HAS_SDL2
    return RenderBackend::BACKEND_SDL2;
#elif ECOSIM_HAS_NCURSES
    return RenderBackend::BACKEND_NCURSES;
#else
    // No backends available - this should not happen in practice
    return RenderBackend::BACKEND_NCURSES;
#endif
}

const char* RenderSystem::getBackendName(RenderBackend backend) {
    switch (backend) {
        case RenderBackend::BACKEND_NCURSES:
            return "NCurses Terminal";
        case RenderBackend::BACKEND_SDL2:
            return "SDL2 Graphics";
        case RenderBackend::BACKEND_AUTO:
            return "Auto-Detect";
        default:
            return "Unknown";
    }
}

//==============================================================================
// Backend Factory Methods
//==============================================================================

IRenderer* RenderSystem::createRenderer(RenderBackend backend) {
    switch (backend) {
        case RenderBackend::BACKEND_NCURSES:
#if ECOSIM_HAS_NCURSES
            return new NCursesRenderer();
#else
            return nullptr;
#endif
            
        case RenderBackend::BACKEND_SDL2:
#if ECOSIM_HAS_SDL2
            return new SDL2Renderer();
#else
            return nullptr;
#endif
            
        case RenderBackend::BACKEND_AUTO:
            // Should have been resolved before calling this
            return createRenderer(getBestAvailableBackend());
            
        default:
            return nullptr;
    }
}

IInputHandler* RenderSystem::createInputHandler(RenderBackend backend) {
    switch (backend) {
        case RenderBackend::BACKEND_NCURSES:
#if ECOSIM_HAS_NCURSES
            return new NCursesInputHandler();
#else
            return nullptr;
#endif
            
        case RenderBackend::BACKEND_SDL2:
#if ECOSIM_HAS_SDL2
            return new SDL2InputHandler();
#else
            return nullptr;
#endif
            
        case RenderBackend::BACKEND_AUTO:
            // Should have been resolved before calling this
            return createInputHandler(getBestAvailableBackend());
            
        default:
            return nullptr;
    }
}

void RenderSystem::destroyRenderer() {
    if (_renderer != nullptr) {
        delete _renderer;
        _renderer = nullptr;
    }
}

void RenderSystem::destroyInputHandler() {
    if (_inputHandler != nullptr) {
        delete _inputHandler;
        _inputHandler = nullptr;
    }
}