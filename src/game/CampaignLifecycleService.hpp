#pragma once

#include "game/ActiveLevelState.hpp"
#include "gameplay/common/Result.hpp"

#include <optional>
#include <set>

namespace soma::game
{
    class EconomyService;

    enum class LevelSessionResult
    {
        abandoned,
        defeated,
        completed,
        saveAndMainMenu,
        abandonAndMainMenu
    };

    struct LevelSessionOutcome
    {
        LevelSessionResult result{LevelSessionResult::abandoned};
        std::optional<ActiveLevelState> savedLevel;
    };

    struct CampaignTransitionResult
    {
        bool returnToMainMenu{};
        bool shopStockInvalidated{};
    };

    /// Applies level completion, save/resume, defeat, and abandon transitions to persistent campaign state.
    class CampaignLifecycleService
    {
      public:
        [[nodiscard]] static CampaignStateData baselineForLevel(const CampaignState& campaign,
                                                                const std::optional<ActiveLevelState>& activeLevel);

        [[nodiscard]] static gameplay::Result<CampaignTransitionResult> apply(
            LevelSessionOutcome outcome,
            const gameplay::LevelId& level,
            const CampaignStateData& baseline,
            CampaignState& campaign,
            EconomyService& economy,
            std::set<gameplay::LevelId>& completedLevels,
            std::optional<ActiveLevelState>& activeLevel);
    };
} // namespace soma::game
