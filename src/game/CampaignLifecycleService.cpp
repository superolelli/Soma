#include "game/CampaignLifecycleService.hpp"

#include "game/EconomyService.hpp"

namespace soma::game
{
    CampaignStateData CampaignLifecycleService::baselineForLevel(const CampaignState& campaign,
                                                                 const std::optional<ActiveLevelState>& activeLevel)
    {
        return activeLevel ? activeLevel->baseline : campaign.data();
    }

    gameplay::Result<CampaignTransitionResult> CampaignLifecycleService::apply(
        LevelSessionOutcome outcome,
        const gameplay::LevelId& level,
        const CampaignStateData& baseline,
        CampaignState& campaign,
        EconomyService& economy,
        std::set<gameplay::LevelId>& completedLevels,
        std::optional<ActiveLevelState>& activeLevel)
    {
        activeLevel = std::move(outcome.savedLevel);

        switch (outcome.result)
        {
        case LevelSessionResult::saveAndMainMenu:
            if (!activeLevel)
            {
                return gameplay::Result<CampaignTransitionResult>::failure(
                    "Saving an active level requires resumable level state");
            }
            return gameplay::Result<CampaignTransitionResult>::success({true, false});

        case LevelSessionResult::completed:
            completedLevels.insert(level);
            economy.clearShopStock();
            activeLevel.reset();
            return gameplay::Result<CampaignTransitionResult>::success({false, true});

        case LevelSessionResult::defeated:
            economy.clearShopStock();
            activeLevel.reset();
            return gameplay::Result<CampaignTransitionResult>::success({false, true});

        case LevelSessionResult::abandonAndMainMenu:
            campaign = CampaignState::fromData(baseline);
            activeLevel.reset();
            return gameplay::Result<CampaignTransitionResult>::success({false, false});

        case LevelSessionResult::abandoned:
            activeLevel.reset();
            return gameplay::Result<CampaignTransitionResult>::success({false, false});
        }

        return gameplay::Result<CampaignTransitionResult>::failure("Unknown level session result");
    }
} // namespace soma::game
