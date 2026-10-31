#pragma once

#include "content/battle/BattleContent.hpp"

#include <filesystem>
#include <json.hpp>

namespace soma::content::detail
{
    using Json = nlohmann::json;

    /// Reads and parses one JSON document, reporting the source path when opening or parsing fails.
    [[nodiscard]] Json readJsonFile(const std::filesystem::path& path);
    [[nodiscard]] AttributeDelta readAttributeDelta(const Json& value);
} // namespace soma::content::detail
