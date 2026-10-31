#include "presentation/sfml/exploration/ExplorationHud.hpp"

namespace soma::presentation
{
    ExplorationHud::ExplorationHud(const content::EconomyContent& economy,
                                   const game::CampaignState& state)
        : resources_(economy, state)
    {
    }

    void ExplorationHud::render(sf::RenderTarget& target, int cards, int dice) const
    {
        static_cast<void>(cards);
        static_cast<void>(dice);
        resources_.render(target);
    }
} // namespace soma::presentation
