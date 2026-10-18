#include "presentation/sfml/BattleTransitionPlayer.hpp"

#include "presentation/sfml/SpriterAnimationPlayer.hpp"

#include <memory>

namespace soma::presentation
{
    bool BattleTransitionPlayer::play(sf::RenderWindow& window,
                                      const BattleTransitionRequest& request,
                                      const std::function<void()>& drawBattle,
                                      IAnimationPlayer* preparedAnimation) const
    {
        std::unique_ptr<SpriterAnimationPlayer> ownedAnimation;
        if (preparedAnimation == nullptr)
        {
            ownedAnimation = std::make_unique<SpriterAnimationPlayer>(window, request.model, request.entity);
            preparedAnimation = ownedAnimation.get();
        }
        auto& transition = *preparedAnimation;
        transition.setPosition(request.position.x, request.position.y);
        transition.setScale(request.scale, request.scale);
        transition.play(request.animation, 1.0F);
        sf::Clock frameClock;
        sf::Clock delayClock;
        bool animationFinished{};
        while (window.isOpen() && (!animationFinished || delayClock.getElapsedTime().asSeconds() < request.delayAfterSeconds))
        {
            while (const auto event = window.pollEvent())
            {
                if (event->is<sf::Event::Closed>())
                {
                    window.close();
                }
            }
            const auto elapsed = frameClock.restart();
            drawBattle();
            if (!animationFinished)
            {
                transition.update(static_cast<float>(elapsed.asMilliseconds()));
                transition.render();
                if (!transition.isPlaying())
                {
                    animationFinished = true;
                    delayClock.restart();
                }
            }
            window.display();
        }
        return window.isOpen();
    }
} // namespace soma::presentation
