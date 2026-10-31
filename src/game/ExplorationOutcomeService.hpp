#pragma once

#include "content/economy/EconomyContent.hpp"
#include "content/rules/RuleContent.hpp"
#include "content/world/WorldContent.hpp"
#include "game/CampaignState.hpp"
#include "game/ExplorationProgress.hpp"
#include "gameplay/common/Result.hpp"
#include "simulation/rewards/LevelRewardCalculator.hpp"

#include <set>
#include <unordered_map>
#include <vector>

namespace soma::game
{
    class EconomyService;

    struct ResolvedEncounterState
    {
        bool playersWon{};
        std::unordered_map<gameplay::ContentDefinitionId, int> playerHealth;
        std::unordered_map<gameplay::ContentDefinitionId, int> playerFatigue;
        std::unordered_set<gameplay::AbilityId> usedPlayerAbilities;
        std::unordered_map<gameplay::ContentDefinitionId, std::vector<simulation::ActiveBuff>> playerBuffs;
    };

    enum class EncounterFollowUp
    {
        battleRewards,
        bossRewards,
        levelDefeat
    };

    struct LevelResultSummary
    {
        simulation::LevelRewards rewards;
        std::vector<gameplay::ItemId> newlyUnlockedItems;
        std::vector<content::RuleFragmentType> newlyDiscoveredRuleTypes;
    };

    /// Applies resolved encounter state and level rewards without depending on presentation or an SFML window.
    class ExplorationOutcomeService
    {
      public:
        [[nodiscard]] static EncounterFollowUp applyEncounter(ResolvedEncounterState outcome,
                                                              int enemyCount,
                                                              bool boss,
                                                              ExplorationProgress& progress);

        [[nodiscard]] static gameplay::Result<LevelResultSummary> completeLevel(
            const content::LevelDefinition& level,
            bool completed,
            const ExplorationProgress& progress,
            const std::set<gameplay::ItemId>& initiallyUnlockedItems,
            const std::set<content::RuleFragmentType>& initiallyDiscoveredRuleTypes,
            CampaignState& campaign,
            EconomyService& economyService,
            const content::EconomyContent& economy,
            const content::RuleContent& rules);

        [[nodiscard]] static std::set<content::RuleFragmentType> discoveredRuleTypes(const CampaignStateData& campaign,
                                                                                     const content::RuleContent& rules);
    };
} // namespace soma::game
