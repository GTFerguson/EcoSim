/**
 * @file ReportGenerator.cpp
 * @brief Implementation of report generation for balance analysis
 */

#include "ReportGenerator.hpp"
#include "objects/creature/creature.hpp"
#include "world/Corpse.hpp"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <fstream>

namespace EcoSim {
namespace Balance {

ReportGenerator::ReportGenerator(std::vector<AnalysisModule*> modules)
    : modules_(std::move(modules))
{
}

bool ReportGenerator::analyze() {
    // Collect results from all modules into the balance report
    report_ = BalanceReport();  // Reset
    
    for (const auto* module : modules_) {
        if (module) {
            module->contributeToReport(report_);
        }
    }
    
    hasRun_ = true;
    return true;
}

std::string ReportGenerator::getResultsText() const {
    return generateTextReport();
}

void ReportGenerator::contributeToReport(BalanceReport& report) const {
    // Copy our aggregated report
    report = report_;
}

std::string ReportGenerator::generateTextReport() const {
    std::ostringstream ss;
    
    ss << generateHeader();
    
    // Collect output from each module
    for (const auto* module : modules_) {
        if (module) {
            ss << module->getResultsText();
            ss << "\n";
        }
    }
    
    // Include LP optimization section if available
    if (!optimizationSection_.empty()) {
        ss << optimizationSection_;
    }
    
    ss << generateSummary();
    
    return ss.str();
}

std::string ReportGenerator::generateHeader() const {
    std::ostringstream ss;
    
    // Get current timestamp
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    ss << "\n";
    ss << separator(100, '*') << "\n";
    ss << "*" << padCenter("ECOSIM BALANCE ANALYSIS REPORT", 98) << "*\n";
    ss << separator(100, '*') << "\n\n";
    
    ss << "Generated: " << std::ctime(&time);  // ctime adds newline
    ss << "Tool Version: 1.0.0\n\n";
    
    // Show key game constants being analyzed
    ss << separator(60, '-') << "\n";
    ss << "GAME CONSTANTS UNDER ANALYSIS:\n";
    ss << separator(60, '-') << "\n";
    ss << "  Creature::BREED_COST      = " << Creature::BREED_COST << " calories per parent\n";
    ss << "  Corpse::NUTRITION_PER_SIZE = " << world::Corpse::NUTRITION_PER_SIZE << " calories per size unit\n";
    ss << "  Total breed cost (2 parents) = " << (Creature::BREED_COST * 2.0f) << " calories\n";
    ss << separator(60, '-') << "\n\n";
    
    return ss.str();
}

std::string ReportGenerator::generateSummary() const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1);
    
    ss << separator(100, '=') << "\n";
    ss << "EXECUTIVE SUMMARY\n";
    ss << separator(100) << "\n\n";
    
    // Trophic analysis summary
    ss << "TROPHIC ANALYSIS:\n";
    ss << "  Pyramid health score: " << report_.pyramidHealthScore << "/100\n";
    ss << "  Ecologically plausible: " << (report_.isEcologicallyPlausible ? "NO" : "YES") << "\n\n";
    
    // Archetype summary
    int exploitableCount = 0;
    for (const auto& p : report_.archetypeProfiles) {
        if (p.reproductionROI > 1.0f) exploitableCount++;
    }
    ss << "ARCHETYPE ANALYSIS:\n";
    ss << "  Total archetypes analyzed: " << report_.archetypeProfiles.size() << "\n";
    ss << "  Archetypes with ROI > 1.0: " << exploitableCount << "\n\n";
    
    // Exploit summary
    int criticalCount = 0, highCount = 0, mediumCount = 0;
    for (const auto& e : report_.exploits) {
        if (e.severity >= 4) criticalCount++;
        else if (e.severity >= 3) highCount++;
        else if (e.severity >= 2) mediumCount++;
    }
    
    ss << "EXPLOIT DETECTION:\n";
    ss << "  Total exploits found: " << report_.exploits.size() << "\n";
    ss << "  Critical: " << criticalCount << "\n";
    ss << "  High: " << highCount << "\n";
    ss << "  Medium: " << mediumCount << "\n\n";
    
    // Overall assessment
    ss << separator(60, '-') << "\n";
    ss << "OVERALL ASSESSMENT: ";
    if (criticalCount > 0) {
        ss << "CRITICAL - Immediate action required\n";
        ss << "The baby cannibalism exploit allows infinite energy generation.\n";
    } else if (highCount > 0) {
        ss << "POOR - Significant balance issues detected\n";
    } else if (mediumCount > 0) {
        ss << "FAIR - Some balance concerns\n";
    } else {
        ss << "GOOD - No major issues detected\n";
    }
    ss << separator(60, '-') << "\n\n";
    
    // Output structured recommendations if available
    if (!report_.recommendations.empty()) {
        ss << separator(80, '=') << "\n";
        ss << "STRUCTURED RECOMMENDATIONS\n";
        ss << separator(80, '=') << "\n\n";
        
        int recNum = 1;
        for (const auto& rec : report_.recommendations) {
            ss << "RECOMMENDATION #" << recNum++ << ": " << rec.parameterName << "\n";
            ss << separator(60, '-') << "\n";
            ss << "  Formula:   " << rec.derivationFormula << "\n";
            // Aligned columns: Value with fixed widths
            ss << "  Value:     " << std::setw(8) << std::left << rec.currentValue
               << " ->    " << rec.targetValue << "\n";
            // Aligned columns: ROI with fixed widths
            ss << "  ROI:       " << std::setw(8) << std::left
               << (std::to_string(static_cast<int>(rec.currentROI * 100.0f)) + "%")
               << " ->    " << static_cast<int>(rec.expectedROI * 100.0f) << "%\n";
            ss << "  Rationale: " << rec.rationale << "\n\n";
        }
    } else if (!report_.exploits.empty()) {
        // Fallback to basic recommendations if no structured ones available
        ss << "RECOMMENDED ACTIONS:\n";
        ss << "1. PRIORITY: Fix breeding cost vs corpse value ratio\n";
        ss << "   - Either increase BREED_COST to ~" << (0.5f * world::Corpse::NUTRITION_PER_SIZE / 2.0f) << " per parent\n";
        ss << "   - Or reduce NUTRITION_PER_SIZE to ~" << (Creature::BREED_COST * 2.0f) << "\n";
        ss << "   - Or implement age-based corpse scaling\n\n";
        ss << "2. Consider tracking actual energy consumed for corpse value\n";
        ss << "3. Add cannibalism penalties or prevention for own offspring\n\n";
    }
    
    ss << separator(100, '*') << "\n";
    ss << "END OF REPORT\n";
    ss << separator(100, '*') << "\n";
    
    return ss.str();
}

std::string ReportGenerator::generateTrophicCSV() const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4);
    
    // Header
    ss << "level,population,biomass,energy_in,energy_out,efficiency,archetypes\n";
    
    for (const auto& m : report_.trophicLevels) {
        ss << trophicLevelToString(m.level) << ","
           << m.populationCount << ","
           << m.biomassStanding << ","
           << m.totalEnergyInput << ","
           << m.totalEnergyOutput << ","
           << m.transferEfficiency << ","
           << "\"";
        
        for (size_t i = 0; i < m.archetypesPresent.size(); ++i) {
            if (i > 0) ss << ";";
            ss << m.archetypesPresent[i];
        }
        ss << "\"\n";
    }
    
    return ss.str();
}

std::string ReportGenerator::generateArchetypeCSV() const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4);
    
    // Header
    ss << "archetype,trophic_level,metabolism_rate,typical_size,energy_per_tick,"
       << "corpse_value,lifetime_consumption,reproduction_roi\n";
    
    for (const auto& p : report_.archetypeProfiles) {
        ss << p.archetypeName << ","
           << trophicLevelToString(p.trophicLevel) << ","
           << p.metabolismRate << ","
           << p.typicalSize << ","
           << p.energyPerTick << ","
           << p.corpseValue << ","
           << p.lifetimeConsumption << ","
           << p.reproductionROI << "\n";
    }
    
    return ss.str();
}

std::string ReportGenerator::generateExploitCSV() const {
    std::ostringstream ss;
    
    // Header
    ss << "exploit_name,severity,affected_archetypes,description\n";
    
    for (const auto& e : report_.exploits) {
        ss << "\"" << e.exploitName << "\","
           << e.severity << ","
           << "\"";
        
        for (size_t i = 0; i < e.affectedArchetypes.size(); ++i) {
            if (i > 0) ss << ";";
            ss << e.affectedArchetypes[i];
        }
        
        // Escape description for CSV
        std::string desc = e.description;
        size_t pos = 0;
        while ((pos = desc.find('"', pos)) != std::string::npos) {
            desc.replace(pos, 1, "\"\"");
            pos += 2;
        }
        while ((pos = desc.find('\n')) != std::string::npos) {
            desc.replace(pos, 1, " ");
        }
        
        ss << "\",\"" << desc << "\"\n";
    }
    
    return ss.str();
}

bool ReportGenerator::writeCSVFiles(const std::string& outputDir) const {
    bool success = true;
    
    // Write trophic levels CSV
    {
        std::ofstream file(outputDir + "/trophic_levels.csv");
        if (file) {
            file << generateTrophicCSV();
        } else {
            success = false;
        }
    }
    
    // Write archetype profiles CSV
    {
        std::ofstream file(outputDir + "/archetype_profiles.csv");
        if (file) {
            file << generateArchetypeCSV();
        } else {
            success = false;
        }
    }
    
    // Write exploits CSV
    {
        std::ofstream file(outputDir + "/exploits.csv");
        if (file) {
            file << generateExploitCSV();
        } else {
            success = false;
        }
    }
    
    return success;
}

bool ReportGenerator::writeReport(const std::string& filepath, OutputFormat format) const {
    if (format == OutputFormat::TEXT || format == OutputFormat::BOTH) {
        std::ofstream file(filepath);
        if (!file) return false;
        file << generateTextReport();
    }
    
    return true;
}

void ReportGenerator::addOptimizationSection(const OptimizationResult& result) {
    std::ostringstream ss;
    
    ss << "\n";
    ss << separator(100, '=') << "\n";
    ss << "LP OPTIMIZATION ANALYSIS\n";
    ss << separator(100, '=') << "\n\n";
    
    // Include the optimizer's generated report
    ss << result.report;
    
    // If infeasible, add constraint conflict visualization
    if (!result.feasible && !result.conflicts.empty()) {
        ss << "\nCONSTRAINT CONFLICT VISUALIZATION:\n";
        ss << separator(60, '-') << "\n";
        
        // ASCII diagram showing the gap between breeding and trophic constraints
        ss << R"(
  NUTRITION_PER_SIZE
    500 ┼
        │
    417 ┼ ─ ─ ─ ─ ─ ─ ─ ─  Trophic max (50%)
        │ ███████████████  Trophic viable range
    167 ┼ ═══════════════  Trophic min (20%)
        │       GAP
    100 ┼ · · · · · · · ·  Current value
        │       GAP
     10 ┼ ═══════════════  Breeding max (80% ROI)
        │ ███████████████  Breeding viable range
      0 ┼────────────────
)";
        
        ss << "\n";
        ss << "The breeding constraints require NUTRITION_PER_SIZE < 10 to prevent exploits,\n";
        ss << "while trophic constraints require NUTRITION_PER_SIZE > 167 for ecosystem viability.\n";
        ss << "These requirements are fundamentally incompatible with current game parameters.\n";
    }
    
    optimizationSection_ = ss.str();
}

} // namespace Balance
} // namespace EcoSim
