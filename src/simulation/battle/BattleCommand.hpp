#pragma once

#include "gameplay/common/StrongId.hpp"

#include <vector>

namespace soma::simulation
{
    struct UseAbilityCommand
    {
        gameplay::EntityId actor;
        gameplay::AbilityId ability;
        std::vector<gameplay::EntityId> targets;
    };

    struct UseConsumableCommand
    {
        gameplay::EntityId actor;
        gameplay::ItemId consumable;
        std::vector<gameplay::EntityId> targets;
    };
} // namespace soma::simulation
