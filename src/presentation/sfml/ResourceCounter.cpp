#include "presentation/sfml/ResourceCounter.hpp"

namespace soma::presentation
{
    ResourceCounter::ResourceCounter(const content::EconomyContent& content, const game::CampaignState& state)
        : layout_(content.presentation.resourceCounter)
        , state_(state)
        , background_(content.assets.at(layout_.background))
        , dice_(content.assets.at(gameplay::AssetId{"skills.dice"}))
        , cards_(content.assets.at(gameplay::AssetId{"shop.cards"}))
        , font_(content.assets.at(gameplay::AssetId{"font.price"}))
    {
    }

    void ResourceCounter::render(sf::RenderTarget& target) const
    {
        sf::Sprite background{background_};
        background.setPosition({static_cast<float>(layout_.backgroundPosition.x), static_cast<float>(layout_.backgroundPosition.y)});
        target.draw(background);

        sf::Sprite dice{dice_};
        dice.setPosition({static_cast<float>(layout_.diceIcon.x), static_cast<float>(layout_.diceIcon.y)});
        const auto diceScale = static_cast<float>(layout_.diceScale);
        dice.setScale({diceScale, diceScale});
        target.draw(dice);
        sf::Sprite cards{cards_};
        cards.setPosition({static_cast<float>(layout_.cardsIcon.x), static_cast<float>(layout_.cardsIcon.y)});
        const auto cardsScale = static_cast<float>(layout_.cardsScale);
        cards.setScale({cardsScale, cardsScale});
        target.draw(cards);
        target.draw(number(state_.currency(gameplay::Currency::dice), layout_.diceTextCenter));
        target.draw(number(state_.currency(gameplay::Currency::cards), layout_.cardsTextCenter));
    }

    sf::Text ResourceCounter::number(int value, const content::Point& center) const
    {
        sf::Text text{font_, std::to_string(value), static_cast<unsigned int>(layout_.textSize)};
        text.setFillColor(sf::Color::White);
        text.setOutlineColor(sf::Color::Black);
        text.setOutlineThickness(1.0F);
        const auto bounds = text.getLocalBounds();
        text.setPosition({static_cast<float>(center.x) - bounds.position.x - bounds.size.x / 2.0F,
                          static_cast<float>(center.y) - bounds.position.y - bounds.size.y / 2.0F});
        return text;
    }
} // namespace soma::presentation
