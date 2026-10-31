#pragma once

#include "game/CampaignState.hpp"
#include "game/ExplorationProgress.hpp"
#include "simulation/exploration/LevelRun.hpp"

namespace soma::game
{
    struct ActiveLevelState
    {
        simulation::LevelRun run;
        ExplorationProgress progress;
        float viewLeft{};
        CampaignStateData baseline;
    };
} // namespace soma::game
