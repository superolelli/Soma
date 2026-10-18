#pragma once

#include "content/EconomyContent.hpp"
#include "game/EconomyService.hpp"
#include "simulation/battle/BattleSimulation.hpp"

namespace soma::game
{
    /// Validates and applies consumables used on party members while exploring.
    class ExplorationConsumableUseService
    {
      public:
        explicit ExplorationConsumableUseService(EconomyService& economy);

        [[nodiscard]] gameplay::Result<CampaignChange> use(const gameplay::ItemDefinition& definition,
                                                            int& health,
                                                            int maximumHealth,
                                                            std::vector<simulation::ActiveBuff>& buffs) const;

      private:
        EconomyService& economy_;
    };
} // namespace soma::game
