#pragma once

#include <nlohmann/json.hpp>

namespace EcoSim {
namespace Genetics {

/**
 * @brief Interface for objects that can be serialized to/from JSON
 * 
 * Provides a common interface for all genetic components that need
 * to support save/load functionality.
 */
class ISerializable {
public:
    virtual ~ISerializable() = default;
    
    /**
     * @brief Serialize object state to JSON
     * @return JSON object representing the serialized state
     */
    virtual nlohmann::json toJson() const = 0;
    
    /**
     * @brief Deserialize object state from JSON
     * @param j The JSON object to deserialize from
     */
    virtual void fromJson(const nlohmann::json& j) = 0;
};

} // namespace Genetics
} // namespace EcoSim
