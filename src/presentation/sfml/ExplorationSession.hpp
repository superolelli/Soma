#pragma once

#include "content/BattleContent.hpp"
#include "content/EconomyContent.hpp"
#include "content/WorldContent.hpp"
#include "content/GameFlowContent.hpp"
#include "content/RuleContent.hpp"
#include "game/CampaignState.hpp"
#include "game/ActiveLevelState.hpp"
#include "game/EconomyService.hpp"
#include "simulation/exploration/LevelRun.hpp"

#include <cstdint>
#include <functional>
#include <optional>

namespace sf
{
    class RenderWindow;
}

namespace soma::presentation
{
    enum class ExplorationResult
    {
        abandoned,
        defeated,
        completed,
        saveAndMainMenu,
        abandonAndMainMenu
    };

    struct ExplorationOutcome
    {
        ExplorationResult result{ExplorationResult::abandoned};
        std::optional<game::ActiveLevelState> savedLevel;
    };

    [[nodiscard]] ExplorationOutcome runCampaignExplorationSession(sf::RenderWindow& window,
                                                                   const content::BattleContent& battle,
                                                                   const content::EconomyContent& economy,
                                                                   const content::WorldContent& world,
                                                                   const content::GameFlowContent& flow,
                                                                   const content::RuleContent& rules,
                                                                   const gameplay::LevelId& level,
                                                                   game::CampaignState& campaignState,
                                                                   game::EconomyService& economyService,
                                                                   std::uint64_t randomSeed,
                                                                   std::optional<game::ActiveLevelState> savedLevel = std::nullopt,
                                                                   std::function<void()> loadingFrame = {});

    int runExplorationSession(const content::BattleContent& battle,
                              const content::EconomyContent& economy,
                              const content::WorldContent& world,
                              const content::RuleContent& rules,
                              const gameplay::LevelId& level,
                              game::CampaignState& campaignState,
                              game::EconomyService& economyService,
                              std::uint64_t randomSeed,
                              std::optional<simulation::LevelRun> authoredLevel = std::nullopt);
} // namespace soma::presentation
