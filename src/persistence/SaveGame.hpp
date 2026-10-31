#pragma once

#include "game/CampaignState.hpp"
#include "game/ActiveLevelState.hpp"

#include <set>
#include <optional>
#include <string>

namespace soma::persistence
{
    inline constexpr int currentSaveVersion = 5;

    struct SaveGame
    {
        int schemaVersion{currentSaveVersion};
        std::string playerName;
        game::CampaignState campaign;
        std::set<gameplay::LevelId> completedLevels;
        std::optional<game::ActiveLevelState> activeLevel;
    };
} // namespace soma::persistence
