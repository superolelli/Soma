#pragma once

#include "gameplay/events/GameplayEvent.hpp"

#include <string>
#include <vector>

namespace soma::presentation
{
    enum class CueKind
    {
        animation,
        notification,
        sound
    };

    struct PresentationCue
    {
        CueKind kind{};
        gameplay::EntityId subject;
        std::string value;
    };

    /// Maps gameplay events to renderer-neutral presentation cues.
    class BattlePresentation
    {
      public:
        [[nodiscard]] std::vector<PresentationCue> map(
            const std::vector<gameplay::GameplayEvent>& events) const;
    };
} // namespace soma::presentation
