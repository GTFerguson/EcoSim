/**
 * @file ReportGenerator.hpp
 * @brief Report formatting and output for balance analysis
 * 
 * Generates text and CSV formatted reports from balance analysis results.
 */

#pragma once

#include "BalanceFramework.hpp"
#include "AnalysisModule.hpp"
#include "BalanceOptimizer.hpp"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

namespace EcoSim {
namespace Balance {

/**
 * @brief Output format options
 */
enum class OutputFormat {
    TEXT,   ///< Human-readable text report
    CSV,    ///< CSV files for data analysis
    BOTH    ///< Generate both formats
};

/**
 * @brief Report generator for balance analysis results
 * 
 * Collects results from all analysis modules and generates
 * formatted output in text and/or CSV format.
 */
class ReportGenerator : public AnalysisModule {
public:
    /**
     * @brief Construct report generator
     * @param modules Vector of analysis modules to collect results from
     */
    explicit ReportGenerator(std::vector<AnalysisModule*> modules);
    
    /**
     * @brief Run report generation
     * @return true if report generated successfully
     */
    bool analyze() override;
    
    /**
     * @brief Get module name
     */
    std::string getName() const override { return "ReportGenerator"; }
    
    /**
     * @brief Get the complete text report
     */
    std::string getResultsText() const override;
    
    /**
     * @brief Contribute to balance report (collects from all modules)
     */
    void contributeToReport(BalanceReport& report) const override;
    
    // --- Report Generation ---
    
    /**
     * @brief Generate complete text report
     * @return Formatted text report string
     */
    std::string generateTextReport() const;
    
    /**
     * @brief Write CSV files to specified directory
     * @param outputDir Directory path for CSV files
     * @return true if files written successfully
     */
    bool writeCSVFiles(const std::string& outputDir) const;
    
    /**
     * @brief Write single report file
     * @param filepath Output file path
     * @param format Output format
     * @return true if file written successfully
     */
    bool writeReport(const std::string& filepath, OutputFormat format = OutputFormat::TEXT) const;
    
    /**
     * @brief Add LP optimization section to the report
     * @param result Optimization result from BalanceOptimizer
     */
    void addOptimizationSection(const OptimizationResult& result);
    
    /**
     * @brief Get the aggregated balance report
     */
    const BalanceReport& getBalanceReport() const { return report_; }

private:
    std::vector<AnalysisModule*> modules_;
    BalanceReport report_;
    bool hasRun_ = false;
    std::string optimizationSection_;  ///< LP optimization report section
    
    /**
     * @brief Generate header section of text report
     */
    std::string generateHeader() const;
    
    /**
     * @brief Generate summary section
     */
    std::string generateSummary() const;
    
    /**
     * @brief Generate CSV for trophic levels
     */
    std::string generateTrophicCSV() const;
    
    /**
     * @brief Generate CSV for archetype profiles
     */
    std::string generateArchetypeCSV() const;
    
    /**
     * @brief Generate CSV for exploits
     */
    std::string generateExploitCSV() const;
};

} // namespace Balance
} // namespace EcoSim
