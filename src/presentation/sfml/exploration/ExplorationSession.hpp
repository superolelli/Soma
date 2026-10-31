#pragma once

#include "content/battle/BattleContent.hpp"
#include "content/economy/EconomyContent.hpp"
#include "content/world/WorldContent.hpp"
#include "content/flow/GameFlowContent.hpp"
#include "content/rules/RuleContent.hpp"
#include "game/CampaignState.hpp"
#include "game/ActiveLevelState.hpp"
#include "game/CampaignLifecycleService.hpp"
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
    [[nodiscard]] game::LevelSessionOutcome runCampaignExplorationSession(sf::RenderWindow& window,
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
