#pragma once

#include "gameplay/common/StrongId.hpp"

namespace soma::simulation
{
    /// Requests that the current actor use one ability with the given primary target.
    struct BattleActionCommand
    {
        gameplay::EntityId actor;
        gameplay::AbilityId ability;
        gameplay::EntityId primaryTarget;
    };
} // namespace soma::simulation
