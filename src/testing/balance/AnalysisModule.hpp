/**
 * @file AnalysisModule.hpp
 * @brief Base class for all analysis modules in the balance tool suite
 * 
 * Provides the abstract interface that all analyzers implement,
 * plus shared utility functions for common operations.
 */

#pragma once

#include "BalanceFramework.hpp"
#include "genetics/expression/Phenotype.hpp"
#include <string>
#include <memory>

namespace EcoSim {
namespace Balance {

/**
 * @brief Abstract base class for analysis modules
 * 
 * Each module (TrophicAnalyzer, ArchetypeProfiler, ExploitDetector)
 * inherits from this class and implements the required interface.
 */
class AnalysisModule {
public:
    virtual ~AnalysisModule() = default;
    
    /**
     * @brief Run the analysis
     * @return true if analysis succeeded, false otherwise
     */
    virtual bool analyze() = 0;
    
    /**
     * @brief Get the module name for reporting
     * @return Human-readable module name
     */
    virtual std::string getName() const = 0;
    
    /**
     * @brief Get analysis results as formatted text
     * @return Formatted text report section
     */
    virtual std::string getResultsText() const = 0;
    
    /**
     * @brief Contribute results to the aggregate report
     * @param report The report to add results to
     */
    virtual void contributeToReport(BalanceReport& report) const = 0;
    
    /**
     * @brief Check if the analysis has been run
     * @return true if analyze() has been called successfully
     */
    bool hasRun() const { return hasRun_; }

protected:
    bool hasRun_ = false;  ///< Track if analyze() has been called
    
    // ========================================================================
    // Shared Utility Functions
    // ========================================================================
    
    /**
     * @brief Classify a creature's trophic level based on diet type
     * @param dietType Diet type string (CARNIVORE, HERBIVORE, etc.)
     * @return Appropriate trophic level
     */
    static TrophicLevel classifyTrophicLevel(const std::string& dietType) {
        if (dietType == "CARNIVORE") {
            return TrophicLevel::SECONDARY;  // Most carnivores are secondary
        } else if (dietType == "HERBIVORE") {
            return TrophicLevel::PRIMARY;
        } else if (dietType == "FRUGIVORE") {
            return TrophicLevel::PRIMARY;    // Frugivores also primary consumers
        } else if (dietType == "OMNIVORE") {
            return TrophicLevel::SECONDARY;  // Omnivores treated as secondary
        } else if (dietType == "NECROVORE") {
            return TrophicLevel::DECOMPOSER;
        }
        return TrophicLevel::SECONDARY;  // Default
    }
    
    /**
     * @brief Classify trophic level from a Phenotype
     * @param phenotype The creature's phenotype
     * @return Appropriate trophic level
     */
    static TrophicLevel classifyTrophicLevel(const Genetics::Phenotype& phenotype) {
        Genetics::DietType diet = phenotype.calculateDietType();
        return classifyTrophicLevel(Genetics::Phenotype::dietTypeToString(diet));
    }
    
    /**
     * @brief Classify trophic level from a DietType enum
     * @param diet The creature's diet type
     * @return Appropriate trophic level
     */
    static TrophicLevel classifyTrophicLevel(Genetics::DietType diet) {
        return classifyTrophicLevel(Genetics::Phenotype::dietTypeToString(diet));
    }
    
    /**
     * @brief Determine if an archetype is an apex predator
     * @param archetypeName The archetype template name
     * @return true if this is an apex predator
     */
    static bool isApexPredator(const std::string& archetypeName) {
        return archetypeName == "apex_predator";
    }
    
    /**
     * @brief Determine if a creature can cannibalize offspring
     * @param dietType The creature's diet type
     * @return true if the creature can eat other creatures (including offspring)
     */
    static bool canCannibalize(const std::string& dietType) {
        return dietType == "CARNIVORE" || dietType == "OMNIVORE" || dietType == "NECROVORE";
    }
    
    /**
     * @brief Format a float to string with specified precision
     * @param value The value to format
     * @param precision Number of decimal places
     * @return Formatted string
     */
    static std::string formatFloat(float value, int precision = 2) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.*f", precision, value);
        return std::string(buffer);
    }
    
    /**
     * @brief Format a percentage value
     * @param value Value in range 0.0-1.0
     * @return String like "45.5%"
     */
    static std::string formatPercent(float value, int precision = 1) {
        return formatFloat(value * 100.0f, precision) + "%";
    }
    
    /**
     * @brief Create a separator line for text reports
     * @param width Width in characters
     * @param character The separator character
     * @return Separator string
     */
    static std::string separator(int width = 80, char character = '-') {
        return std::string(width, character);
    }
    
    /**
     * @brief Pad a string to specified width
     * @param text The text to pad
     * @param width Target width
     * @param padChar Character to pad with
     * @return Padded string
     */
    static std::string padRight(const std::string& text, size_t width, char padChar = ' ') {
        if (text.length() >= width) return text;
        return text + std::string(width - text.length(), padChar);
    }
    
    static std::string padLeft(const std::string& text, size_t width, char padChar = ' ') {
        if (text.length() >= width) return text;
        return std::string(width - text.length(), padChar) + text;
    }
    
    static std::string padCenter(const std::string& text, size_t width, char padChar = ' ') {
        if (text.length() >= width) return text;
        size_t totalPad = width - text.length();
        size_t leftPad = totalPad / 2;
        size_t rightPad = totalPad - leftPad;
        return std::string(leftPad, padChar) + text + std::string(rightPad, padChar);
    }
};

} // namespace Balance
} // namespace EcoSim
