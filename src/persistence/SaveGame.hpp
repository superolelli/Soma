#pragma once

#include "game/CampaignState.hpp"
#include "game/ActiveLevelState.hpp"

#include <set>
#include <optional>
#include <string>
#include <cstddef>

namespace soma::persistence
{
    inline constexpr int currentSaveVersion = 5;

    struct TutorialProgress
    {
        // Missing tutorial data in an older save intentionally means completed.
        bool initialSequenceCompleted{true};
        std::optional<std::size_t> mainRoomStep;
        bool rulesUnlocked{};
        std::set<std::string> shownMessages;
    };

    struct SaveGame
    {
        int schemaVersion{currentSaveVersion};
        std::string playerName;
        game::CampaignState campaign;
        std::set<gameplay::LevelId> completedLevels;
        std::optional<game::ActiveLevelState> activeLevel;
        TutorialProgress tutorial;
    };
} // namespace soma::persistence
