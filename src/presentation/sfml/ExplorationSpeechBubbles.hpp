#pragma once

#include "content/WorldContent.hpp"
#include "gameplay/random/IRandom.hpp"
#include "presentation/animation/IAnimationPlayer.hpp"
#include "presentation/sfml/NinePatch.hpp"

#include <SFML/Graphics.hpp>
#include <array>
#include <vector>

namespace soma::presentation
{
    /// Schedules ambient party dialogue and sizes nine-slice bubbles to wrapped text.
    class ExplorationSpeechBubbles
    {
      public:
        ExplorationSpeechBubbles(const content::WorldContent::Presentation::SpeechBubbles& layout,
                                 const sf::Texture& texture,
                                 const sf::Font& font,
                                 gameplay::IRandom& random);

        void update(sf::Time elapsed, const std::array<AnimationBounds, 4>& speakers, const std::array<bool, 4>& available, bool active);
        void render(sf::RenderTarget& target) const;

      private:
        struct Bubble
        {
            std::size_t speaker{};
            sf::Text text;
            NinePatch background;
            sf::Time remaining;
        };

        void createBubble(const std::array<bool, 4>& available);
        void positionBubble(Bubble& bubble, const AnimationBounds& speaker) const;
        [[nodiscard]] sf::Text wrappedText(const std::string& value) const;

        const content::WorldContent::Presentation::SpeechBubbles& layout_;
        const sf::Texture& texture_;
        const sf::Font& font_;
        gameplay::IRandom& random_;
        sf::Time untilCheck_;
        std::vector<Bubble> bubbles_;
        bool active_{};
    };
} // namespace soma::presentation
