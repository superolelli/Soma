#pragma once

#include "game/CampaignState.hpp"
#include "simulation/exploration/LevelRun.hpp"

#include <unordered_map>

namespace soma::game
{
    struct ActiveLevelState
    {
        simulation::LevelRun run;
        std::unordered_map<gameplay::ContentDefinitionId, int> playerHealth;
        std::unordered_map<gameplay::ContentDefinitionId, int> playerFatigue;
        float viewLeft{};
        CampaignStateData baseline;
    };
} // namespace soma::game
