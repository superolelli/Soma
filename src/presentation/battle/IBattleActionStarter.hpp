#pragma once

#include "simulation/battle/BattleCommand.hpp"

namespace soma::presentation
{
    /// Starts presentation for a selected battle command or a passed turn.
    class IBattleActionStarter
    {
      public:
        virtual ~IBattleActionStarter() = default;

        virtual void start(simulation::BattleActionCommand command, bool announce) = 0;
        virtual void startPass(const gameplay::EntityId& actor) = 0;
    };
} // namespace soma::presentation
