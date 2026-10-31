#pragma once

#include "content/battle/BattleContent.hpp"
#include "content/economy/EconomyContent.hpp"
#include "content/world/WorldContent.hpp"

#include <filesystem>

namespace soma::content::detail
{
    void validateEncounterBudgetReachability(const LevelDefinition& level, const WorldContent& world);

    void validateWorldContent(const WorldContent& world,
                              const BattleContent& battle,
                              const EconomyContent& economy,
                              const std::filesystem::path& repositoryRoot);
} // namespace soma::content::detail
