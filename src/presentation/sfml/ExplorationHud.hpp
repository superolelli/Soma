#pragma once

#include "content/WorldContent.hpp"
#include "presentation/sfml/ResourceCounter.hpp"

#include <SFML/Graphics.hpp>

namespace soma::presentation
{
    /// Draws exploration resource counters and the collapsible consumable interface.
    class ExplorationHud
    {
      public:
        ExplorationHud(const content::EconomyContent& economy,
                       const game::CampaignState& state);
        void render(sf::RenderTarget& target, int cards, int dice) const;

      private:
        ResourceCounter resources_;
    };
} // namespace soma::presentation
