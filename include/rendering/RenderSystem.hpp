/**
 * @file RenderSystem.hpp
 * @brief Facade class managing renderer and input handler
 * @author Gary Ferguson
 * @date December 2024
 * 
 * This file defines the RenderSystem class which provides a unified interface
 * for initializing and managing rendering backends. It uses the factory pattern
 * to create appropriate renderer and input handler implementations based on
 * the selected backend.
 */

#ifndef ECOSIM_RENDERSYSTEM_HPP
#define ECOSIM_RENDERSYSTEM_HPP

#include "IRenderer.hpp"
#include "IInputHandler.hpp"
#include "RenderTypes.hpp"

/**
 * @brief Rendering backend enumeration
 * 
 * Specifies which rendering backend to use.
 */
enum class RenderBackend {
    BACKEND_NCURSES = 0,    ///< NCurses terminal-based ASCII rendering
    BACKEND_SDL2,           ///< SDL2 graphics with optional ImGui (future)
    BACKEND_AUTO            ///< Auto-detect best available backend
};

/**
 * @brief Configuration for render system initialization
 */
struct RenderConfig {
    RenderBackend backend;          ///< Which backend to use
    bool enableMouse;               ///< Enable mouse input if supported
    unsigned int inputDelayMs;      ///< Input polling delay (ms)
    unsigned int targetFPS;         ///< Target frame rate (for SDL2)
    
    /** @brief Default constructor with sensible defaults */
    RenderConfig()
        : backend(RenderBackend::BACKEND_AUTO)
        , enableMouse(false)
        , inputDelayMs(100)
        , targetFPS(60) {}
};

/**
 * @brief Facade class managing the rendering subsystem
 * 
 * RenderSystem provides a unified interface for initializing, configuring,
 * and accessing rendering and input handling functionality. It uses the
 * factory pattern to create the appropriate backend implementations.
 * 
 * This class follows the singleton pattern for global access, but also
 * supports explicit instantiation for testing or multiple instances.
 * 
 * Usage:
 * @code
 * // Using singleton
 * RenderSystem::initialize(RenderBackend::BACKEND_NCURSES);
 * IRenderer& renderer = RenderSystem::getInstance().getRenderer();
 * IInputHandler& input = RenderSystem::getInstance().getInputHandler();
 * 
 * // Main loop
 * while (running) {
 *     renderer.beginFrame();
 *     renderer.renderWorld(world, viewport);
 *     renderer.endFrame();
 *     
 *     InputEvent event = input.pollInput();
 *     // handle event...
 * }
 * 
 * RenderSystem::shutdown();
 * @endcode
 * 
 * Or without singleton:
 * @code
 * RenderSystem renderSystem;
 * RenderConfig config;
 * config.backend = RenderBackend::BACKEND_NCURSES;
 * 
 * if (renderSystem.init(config)) {
 *     // use renderSystem.getRenderer() etc.
 *     renderSystem.cleanup();
 * }
 * @endcode
 */
class RenderSystem {
public:
    /**
     * @brief Default constructor
     * 
     * Creates an uninitialized RenderSystem. Call init() to initialize.
     */
    RenderSystem();
    
    /**
     * @brief Destructor
     * 
     * Automatically calls cleanup() if still initialized.
     */
    ~RenderSystem();
    
    // Prevent copying
    RenderSystem(const RenderSystem&) = delete;
    RenderSystem& operator=(const RenderSystem&) = delete;

    //==========================================================================
    // Static Singleton Interface
    //==========================================================================
    
    /**
     * @brief Initialize the global render system singleton
     * 
     * @param backend The rendering backend to use
     * @return true if initialization successful
     */
    static bool initialize(RenderBackend backend = RenderBackend::BACKEND_AUTO);
    
    /**
     * @brief Initialize the global render system singleton with config
     * 
     * @param config Configuration options
     * @return true if initialization successful
     */
    static bool initialize(const RenderConfig& config);
    
    /**
     * @brief Shutdown the global render system singleton
     */
    static void shutdown();
    
    /**
     * @brief Get the global render system instance
     * 
     * @return Reference to the singleton instance
     * @throws std::runtime_error if not initialized
     */
    static RenderSystem& getInstance();
    
    /**
     * @brief Check if the singleton is initialized
     * 
     * @return true if initialized
     */
    static bool isInitialized();

    //==========================================================================
    // Instance Interface
    //==========================================================================
    
    /**
     * @brief Initialize this render system instance
     * 
     * @param config Configuration options
     * @return true if initialization successful
     */
    bool init(const RenderConfig& config);
    
    /**
     * @brief Cleanup and shutdown this instance
     */
    void cleanup();
    
    /**
     * @brief Check if this instance is initialized
     * 
     * @return true if initialized and ready
     */
    bool isReady() const;
    
    /**
     * @brief Get the renderer interface
     * 
     * @return Reference to the renderer
     * @throws std::runtime_error if not initialized
     */
    IRenderer& getRenderer();
    
    /**
     * @brief Get the renderer interface (const version)
     * 
     * @return Const reference to the renderer
     */
    const IRenderer& getRenderer() const;
    
    /**
     * @brief Get the input handler interface
     * 
     * @return Reference to the input handler
     * @throws std::runtime_error if not initialized
     */
    IInputHandler& getInputHandler();
    
    /**
     * @brief Get the input handler interface (const version)
     * 
     * @return Const reference to the input handler
     */
    const IInputHandler& getInputHandler() const;
    
    /**
     * @brief Get the current backend type
     * 
     * @return The active rendering backend
     */
    RenderBackend getBackend() const;
    
    /**
     * @brief Get renderer capabilities
     * 
     * @return Capabilities of the current renderer
     */
    RendererCapabilities getCapabilities() const;

    //==========================================================================
    // Backend Detection
    //==========================================================================
    
    /**
     * @brief Check if a specific backend is available
     * 
     * @param backend The backend to check
     * @return true if the backend can be used
     */
    static bool isBackendAvailable(RenderBackend backend);
    
    /**
     * @brief Get the best available backend
     * 
     * @return The recommended backend for this system
     */
    static RenderBackend getBestAvailableBackend();
    
    /**
     * @brief Get a human-readable name for a backend
     * 
     * @param backend The backend
     * @return Name string (e.g., "NCurses Terminal")
     */
    static const char* getBackendName(RenderBackend backend);

private:
    IRenderer* _renderer;           ///< Renderer implementation
    IInputHandler* _inputHandler;   ///< Input handler implementation
    RenderBackend _backend;         ///< Active backend type
    bool _initialized;              ///< Initialization state
    
    // Singleton instance
    static RenderSystem* _instance;
    
    /**
     * @brief Create renderer for specified backend
     * 
     * @param backend The backend type
     * @return Pointer to new renderer, or nullptr on failure
     */
    IRenderer* createRenderer(RenderBackend backend);
    
    /**
     * @brief Create input handler for specified backend
     * 
     * @param backend The backend type
     * @return Pointer to new input handler, or nullptr on failure
     */
    IInputHandler* createInputHandler(RenderBackend backend);
    
    /**
     * @brief Destroy renderer and release resources
     */
    void destroyRenderer();
    
    /**
     * @brief Destroy input handler and release resources
     */
    void destroyInputHandler();
};

#endif // ECOSIM_RENDERSYSTEM_HPP