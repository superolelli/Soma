#pragma once

#include "gameplay/common/StrongId.hpp"
#include "simulation/battle/BattleState.hpp"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace soma::game
{
    /// Collects the party state and reward progress that must survive every battle and a mid-level save/resume.
    struct ExplorationProgress
    {
        bool defeated{};
        int enemiesDefeated{};
        std::unordered_set<gameplay::AbilityId> usedPlayerAbilities;
        std::unordered_map<gameplay::ContentDefinitionId, int> playerHealth;
        std::unordered_map<gameplay::ContentDefinitionId, int> playerFatigue;
        std::unordered_map<gameplay::ContentDefinitionId, std::vector<simulation::ActiveBuff>> playerBuffs;
    };
} // namespace soma::game
