#pragma once

#include "content/EconomyContent.hpp"
#include "game/CampaignState.hpp"

#include <SFML/Graphics.hpp>

namespace soma::presentation
{
    /// Draws compact card and dice counters shared by hub, exploration, and battle screens.
    class ResourceCounter
    {
      public:
        ResourceCounter(const content::EconomyContent& content, const game::CampaignState& state);
        void render(sf::RenderTarget& target) const;

      private:
        [[nodiscard]] sf::Text number(int value, const content::Point& center) const;

        const content::EconomyPresentation::ResourceCounterLayout& layout_;
        const game::CampaignState& state_;
        sf::Texture dice_;
        sf::Texture cards_;
        sf::Font font_;
    };
} // namespace soma::presentation
