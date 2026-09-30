#pragma once
#include <optional>
#include "nlohmann/json_fwd.hpp"

namespace SpinoCore::Utils::JSON {
    using json = nlohmann::json;

    [[nodiscard]] std::optional<json> ParseJSONFromBytes(
        const std::vector<uint8_t>& fileData,
        const std::vector<std::string>& requiredFields = {}
    );
}
