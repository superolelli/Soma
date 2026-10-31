#pragma once

#include "gameplay/events/GameplayEvent.hpp"

#include <vector>

namespace soma::presentation
{
    /// Receives the two sound phases emitted while a resolved battle action is presented.
    class IBattleSoundPlayer
    {
      public:
        virtual ~IBattleSoundPlayer() = default;

        virtual void playActionStart(const std::vector<gameplay::GameplayEvent>& events) = 0;
        virtual void playResults(const std::vector<gameplay::GameplayEvent>& events) = 0;
    };
} // namespace soma::presentation
