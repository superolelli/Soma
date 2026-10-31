#pragma once

#include "content/battle/BattleContent.hpp"
#include "content/economy/EconomyContent.hpp"
#include "game/CampaignState.hpp"

namespace soma::game
{
    /// Resolves skills, equipment, and selected ability variants into battle-ready content.
    class CampaignBattleContentBuilder
    {
      public:
        [[nodiscard]] content::BattleContent build(const content::BattleContent& battleContent,
                                                   const content::EconomyContent& economyContent,
                                                   const CampaignState& state) const;
    };
} // namespace soma::game
