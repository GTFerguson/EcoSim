/**
 * @file balance_diagnostic.cpp
 * @brief Main executable for EcoSim balance analysis
 * 
 * Runs all balance analyzers and generates a comprehensive report
 * detecting energy flow exploits like the baby cannibalism problem.
 * 
 * Usage: ./balance_diagnostic [--csv output_dir] [--output report.txt]
 */

#include "BalanceFramework.hpp"
#include "AnalysisModule.hpp"
#include "TrophicAnalyzer.hpp"
#include "ArchetypeProfiler.hpp"
#include "ExploitDetector.hpp"
#include "ReportGenerator.hpp"

#include "genetics/core/GeneRegistry.hpp"
#include "genetics/defaults/UniversalGenes.hpp"

#include <iostream>
#include <string>
#include <memory>
#include <vector>

using namespace EcoSim::Balance;
using namespace EcoSim::Genetics;

void printUsage(const char* programName) {
    std::cerr << "Usage: " << programName << " [options]\n"
              << "Options:\n"
              << "  --csv <dir>      Write CSV files to specified directory\n"
              << "  --output <file>  Write text report to specified file\n"
              << "  --help           Show this help message\n"
              << "\n"
              << "Exit codes:\n"
              << "  0  No critical exploits detected\n"
              << "  1  Critical exploits detected\n"
              << "  2  Error during analysis\n";
}

int main(int argc, char* argv[]) {
    std::string csvDir;
    std::string outputFile;
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--csv" && i + 1 < argc) {
            csvDir = argv[++i];
        } else if (arg == "--output" && i + 1 < argc) {
            outputFile = argv[++i];
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            printUsage(argv[0]);
            return 2;
        }
    }
    
    std::cout << "EcoSim Balance Diagnostic Tool\n";
    std::cout << "================================\n\n";
    
    // Step 1: Initialize the gene registry with universal genes
    std::cout << "Initializing gene registry..." << std::flush;
    auto registry = std::make_shared<GeneRegistry>();
    UniversalGenes::registerDefaults(*registry);
    std::cout << " done. (" << registry->size() << " genes registered)\n";
    
    // Step 2: Create analyzers
    std::cout << "Creating analyzers...\n";
    
    TrophicAnalyzer trophicAnalyzer(registry);
    ArchetypeProfiler archetypeProfiler(registry);
    
    // Step 3: Run trophic analysis
    std::cout << "  Running trophic analysis..." << std::flush;
    if (!trophicAnalyzer.analyze()) {
        std::cerr << " FAILED\n";
        return 2;
    }
    std::cout << " done.\n";
    
    // Step 4: Run archetype profiling
    std::cout << "  Running archetype profiling..." << std::flush;
    if (!archetypeProfiler.analyze()) {
        std::cerr << " FAILED\n";
        return 2;
    }
    std::cout << " done. (" << archetypeProfiler.getProfiles().size() << " archetypes profiled)\n";
    
    // Step 5: Run exploit detection
    std::cout << "  Running exploit detection..." << std::flush;
    ExploitDetector exploitDetector(registry, &archetypeProfiler);
    if (!exploitDetector.analyze()) {
        std::cerr << " FAILED\n";
        return 2;
    }
    std::cout << " done. (" << exploitDetector.getExploits().size() << " exploits found)\n";
    
    // Step 6: Generate report
    std::cout << "  Generating report..." << std::flush;
    std::vector<AnalysisModule*> modules = {
        &trophicAnalyzer,
        &archetypeProfiler,
        &exploitDetector
    };
    ReportGenerator reportGenerator(modules);
    
    // Add LP optimization section from exploit detector
    reportGenerator.addOptimizationSection(exploitDetector.getOptimizationResult());
    
    if (!reportGenerator.analyze()) {
        std::cerr << " FAILED\n";
        return 2;
    }
    std::cout << " done.\n\n";
    
    // Step 7: Output results
    std::string report = reportGenerator.generateTextReport();
    
    // Write to stdout
    std::cout << report;
    
    // Write to file if specified
    if (!outputFile.empty()) {
        if (reportGenerator.writeReport(outputFile)) {
            std::cout << "\nReport written to: " << outputFile << "\n";
        } else {
            std::cerr << "\nFailed to write report to: " << outputFile << "\n";
        }
    }
    
    // Write CSV files if directory specified
    if (!csvDir.empty()) {
        if (reportGenerator.writeCSVFiles(csvDir)) {
            std::cout << "\nCSV files written to: " << csvDir << "/\n";
            std::cout << "  - trophic_levels.csv\n";
            std::cout << "  - archetype_profiles.csv\n";
            std::cout << "  - exploits.csv\n";
        } else {
            std::cerr << "\nFailed to write some CSV files to: " << csvDir << "\n";
        }
    }
    
    // Return exit code based on exploit severity
    int criticalCount = exploitDetector.getCriticalCount();
    if (criticalCount > 0) {
        std::cout << "\n*** " << criticalCount << " CRITICAL exploit(s) detected! ***\n";
        return 1;
    }
    
    return 0;
}
