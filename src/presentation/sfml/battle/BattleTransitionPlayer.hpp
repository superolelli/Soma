#pragma once

#include "presentation/sfml/battle/BattleScreenSession.hpp"

#include <SFML/Graphics.hpp>
#include <functional>

namespace soma::presentation
{
    /// Runs the full-screen transition animation before control passes to a battle session.
    class BattleTransitionPlayer
    {
      public:
        [[nodiscard]] bool play(sf::RenderWindow& window,
                                const BattleTransitionRequest& request,
                                const std::function<void()>& drawBattle,
                                IAnimationPlayer* preparedAnimation = nullptr) const;
    };
} // namespace soma::presentation
