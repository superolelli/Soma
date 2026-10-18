#pragma once
#include "gameplay/events/GameplayEvent.hpp"

namespace soma::gameplay
{
    /// Receives gameplay events from simulation without coupling it to a concrete presentation.
    class EventSink
    {
      public:
        virtual ~EventSink() = default;
        virtual void publish(const GameplayEvent& event) = 0;
    };
} // namespace soma::gameplay
