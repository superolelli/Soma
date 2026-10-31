#pragma once

#include "gameplay/events/GameplayEvent.hpp"

#include <vector>

namespace soma::presentation
{
    /// Groups simulation events into coherent presentation beats so related outcomes can play together.
    class BattleEventBatcher
    {
      public:
        [[nodiscard]] static bool isRiposteTrigger(const gameplay::GameplayEvent& event);

        [[nodiscard]] static std::vector<gameplay::GameplayEvent> takeNext(
            std::vector<gameplay::GameplayEvent>& deferredEvents);
    };
} // namespace soma::presentation
