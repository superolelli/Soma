#pragma once

#include "content/tutorial/TutorialContent.hpp"

#include <SFML/Graphics.hpp>
#include <functional>

namespace soma::presentation
{
    /// Presents modal, data-driven tutorial copy over any game screen and optionally marks a relevant UI region.
    class TutorialOverlay
    {
      public:
        TutorialOverlay(sf::RenderWindow& window, const sf::Font& font);

        [[nodiscard]] bool show(const content::TutorialMessage& message, const std::function<void()>& drawBackground) const;

      private:
        [[nodiscard]] sf::String wrapped(std::string_view text, unsigned int size, float maximumWidth) const;
        void draw(const content::TutorialMessage& message, const std::function<void()>& drawBackground, sf::Vector2f mouse) const;

        sf::RenderWindow& window_;
        const sf::Font& font_;
    };
} // namespace soma::presentation
