#include "genetics/behaviors/PlantLifecycleTick.hpp"

#include "genetics/organisms/Plant.hpp"
#include "genetics/expression/EnvironmentState.hpp"

namespace EcoSim {
namespace Genetics {

void PlantLifecycleTick::tick(Organism& organism, const EnvironmentState& env) {
    // Delegate to the plant's existing update() implementation. The
    // dynamic_cast is safe because this tick is only attached to Plants
    // via the registration path (a future TickRegistry will enforce the
    // attachment contract). If a non-Plant ends up here it's a bug —
    // silently no-op rather than crash.
    Plant* plant = dynamic_cast<Plant*>(&organism);
    if (!plant) return;
    plant->update(env);
}

} // namespace Genetics
} // namespace EcoSim
