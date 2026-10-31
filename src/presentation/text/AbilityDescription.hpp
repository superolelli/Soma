#pragma once

#include "content/battle/BattleContent.hpp"
#include "content/economy/EconomyContent.hpp"

#include <string>
#include <vector>

namespace soma::presentation
{
    struct AbilityStatusDetail
    {
        gameplay::StatusId status;
        std::string color;
        std::string name;
        std::string explanation;
    };

    [[nodiscard]] std::string abilityDescription(const content::AbilityDefinition& ability);
    [[nodiscard]] std::vector<AbilityStatusDetail> abilityStatusDetails(const content::AbilityDefinition& ability);
    [[nodiscard]] std::vector<AbilityStatusDetail> skillStatusDetails(const content::AbilityUpgrade& upgrade);
    [[nodiscard]] std::string fatigueDescription(const content::AbilityDefinition& ability);
}
