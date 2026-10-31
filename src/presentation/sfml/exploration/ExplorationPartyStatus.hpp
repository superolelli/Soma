#pragma once

#include "content/world/WorldContent.hpp"
#include "gameplay/combat/Fatigue.hpp"
#include "presentation/animation/IAnimationPlayer.hpp"

#include <SFML/Graphics.hpp>
#include <array>

namespace soma::presentation
{
    /// Draws party health, fatigue, and status transitions beneath exploration characters.
    class ExplorationPartyStatus
    {
      public:
        ExplorationPartyStatus(const content::WorldContent& world, const sf::Font& font);

        void update(sf::Time elapsed,
                    const std::array<int, 4>& health,
                    const std::array<int, 4>& fatigue,
                    const std::array<bool, 4>& buffed);

        void render(sf::RenderTarget& target,
                    const std::array<AnimationBounds, 4>& bounds,
                    const std::array<int, 4>& health,
                    const std::array<int, 4>& maximumHealth,
                    sf::Vector2f mouse) const;

      private:
        void renderMember(sf::RenderTarget& target,
                          const AnimationBounds& bounds,
                          int maximumHealth,
                          std::size_t index,
                          sf::Vector2f mouse) const;
        void renderTooltip(sf::RenderTarget& target, std::string_view value, sf::Vector2f position) const;
        struct StatusTransition
        {
            bool desired{};
            bool visible{};
            bool removing{};
            float elapsed{0.5F};
        };
        static void updateTransition(StatusTransition& transition, bool desired, float elapsed);
        [[nodiscard]] static float transitionScale(const StatusTransition& transition);

        const content::WorldContent::Presentation::PartyStatus& layout_;
        const sf::Font& font_;
        sf::Texture healthBar_;
        sf::Texture healthBarFrame_;
        sf::Texture fatigueBar_;
        sf::Texture fatigueBarFrame_;
        sf::Texture fatigueTired_;
        sf::Texture fatigueStupid_;
        sf::Texture buff_;
        std::array<float, 4> displayedHealth_{-1.0F, -1.0F, -1.0F, -1.0F};
        std::array<float, 4> healthStart_{};
        std::array<int, 4> healthTarget_{};
        std::array<float, 4> healthElapsed_{0.7F, 0.7F, 0.7F, 0.7F};
        std::array<float, 4> displayedFatigue_{-1.0F, -1.0F, -1.0F, -1.0F};
        std::array<float, 4> fatigueStart_{};
        std::array<int, 4> fatigueTarget_{};
        std::array<float, 4> fatigueElapsed_{0.7F, 0.7F, 0.7F, 0.7F};
        std::array<StatusTransition, 4> buffTransitions_;
        std::array<StatusTransition, 4> tiredTransitions_;
        std::array<StatusTransition, 4> stupidTransitions_;
    };
} // namespace soma::presentation
