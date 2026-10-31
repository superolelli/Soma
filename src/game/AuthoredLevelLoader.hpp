#pragma once

#include "content/world/WorldContent.hpp"
#include "simulation/exploration/LevelRun.hpp"

#include <filesystem>

namespace soma::game
{
    /// Loads an explicitly authored room sequence and validates every referenced world definition.
    ///
    /// @throws std::runtime_error if the file cannot be opened, a referenced asset or definition is unknown, or no rooms are provided.
    /// @throws nlohmann::json::exception if the document is malformed or does not match the authored-level schema.
    /// @throws std::invalid_argument if the resulting rooms do not satisfy LevelRun's regular-room-then-boss invariant.
    [[nodiscard]] simulation::LevelRun loadAuthoredLevel(const std::filesystem::path& path,
                                                         const gameplay::LevelId& level,
                                                         const content::WorldContent& world);
}
