#pragma once

#include "content/WorldContent.hpp"
#include "simulation/exploration/LevelRun.hpp"

#include <filesystem>

namespace soma::game
{
    [[nodiscard]] simulation::LevelRun loadAuthoredLevel(const std::filesystem::path& path,
                                                         const gameplay::LevelId& level,
                                                         const content::WorldContent& world);
}
