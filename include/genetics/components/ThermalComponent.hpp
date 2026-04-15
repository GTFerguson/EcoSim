#pragma once

#include "genetics/expression/EnvironmentalStress.hpp"

namespace EcoSim {
namespace Genetics {

/**
 * Runtime cache for temperature tolerance and environmental-stress
 * calculations.
 *
 * Thermal adaptations and tolerance ranges are derived from gene values
 * at phenotype evaluation time. Caching avoids recomputing them every
 * tick — when the phenotype invalidates (mutation, expression shift),
 * the cache is marked dirty and recomputed on next access.
 *
 * Every organism that cares about temperature carries this component.
 * Organisms that are temperature-indifferent (future cold-immune fungi,
 * hypothetical deep-sea organisms) can skip it.
 */
struct ThermalComponent {
    // Derived thermal adaptations (bundled from gene values)
    ThermalAdaptations adaptations = ThermalAdaptations::defaults();

    // Effective tolerance range after adaptation + phenotype modulation
    EffectiveToleranceRange toleranceRange{};

    // Raw base tolerance values from gene expression
    float baseTempLow  = -999.0f;
    float baseTempHigh = -999.0f;

    // Cache invalidation: dirty flag set on phenotype update, cleared
    // when the cache is recomputed.
    bool cacheDirty = true;

    // Last processed environment temperature — allows us to skip the
    // stress recomputation when the current temperature hasn't moved
    // far enough to matter.
    float lastProcessedTemp = -999.0f;

    // Current environmental stress state (for UI/debug + per-tick use)
    TemperatureStress currentStress{};
};

} // namespace Genetics
} // namespace EcoSim
