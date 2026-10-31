#include "presentation/sfml/exploration/ExplorationConsumablePanel.hpp"

#include "presentation/sfml/common/AudioSettings.hpp"

#include "game/EconomyQueries.hpp"
#include "presentation/sfml/economy/EconomyTooltip.hpp"

namespace soma::presentation
{
    ExplorationConsumablePanel::ExplorationConsumablePanel(const content::EconomyContent& content,
                                                           const game::CampaignState& state,
                                                           const EconomyTextures& textures,
                                                           const sf::Font& font)
        : content_(content)
        , state_(state)
        , textures_(textures)
        , font_(font)
        , consumedBuffer_(content.assets.at(gameplay::AssetId{"exploration.consumable_sound"}))
        , consumedSound_(consumedBuffer_)
    {
    }

    void ExplorationConsumablePanel::press(sf::Vector2f position)
    {
        const auto items = game::EconomyQueries{state_, content_}.consumables(game::ItemSort::name);
        const auto index = itemAt(position);
        if (index && *index < items.size())
        {
            dragged_ = items.at(*index).item;
        }
    }

    std::optional<gameplay::ItemId> ExplorationConsumablePanel::release()
    {
        return std::exchange(dragged_, std::nullopt);
    }

    bool ExplorationConsumablePanel::clickableAt(sf::Vector2f position) const
    {
        return itemAt(position).has_value();
    }

    void ExplorationConsumablePanel::playConsumedSound()
    {
        consumedSound_.setVolume(AudioSettings::sound());
        consumedSound_.play();
    }

    void ExplorationConsumablePanel::render(sf::RenderWindow& window, sf::Vector2f mousePosition) const
    {
        const auto& layout = content_.presentation.explorationConsumables;
        sf::Sprite panel{textures_.at("battle.consumables_panel")};
        panel.setPosition({static_cast<float>(layout.position.x), static_cast<float>(layout.position.y)});
        window.draw(panel);
        const auto items = game::EconomyQueries{state_, content_}.consumables(game::ItemSort::name);
        std::optional<std::size_t> hovered;
        for (std::size_t index = 0; index < items.size(); ++index)
        {
            const auto position = itemPosition(index);
            sf::Sprite icon{textures_.at(items.at(index).icon)};
            icon.setPosition(position);
            window.draw(icon);
            sf::Text quantity{font_, std::to_string(items.at(index).quantity), 20U};
            quantity.setFillColor(sf::Color::White);
            quantity.setOutlineColor(sf::Color::Black);
            quantity.setOutlineThickness(2.0F);
            quantity.setPosition(position + sf::Vector2f{static_cast<float>(layout.quantityOffset.x),
                                                         static_cast<float>(layout.quantityOffset.y)});
            window.draw(quantity);
            const sf::FloatRect bounds{position, {static_cast<float>(layout.itemSize), static_cast<float>(layout.itemSize)}};
            if (bounds.contains(mousePosition))
            {
                hovered = index;
            }
        }
        if (dragged_)
        {
            sf::Sprite dragged{textures_.at(content_.items.at(*dragged_).icon)};
            dragged.setPosition(mousePosition - sf::Vector2f{50.0F, 50.0F});
            window.draw(dragged);
        }
        else if (hovered)
        {
            drawItemTooltip(window, font_, content_.items.at(items.at(*hovered).item), mousePosition);
        }
    }

    sf::Vector2f ExplorationConsumablePanel::itemPosition(std::size_t index) const
    {
        const auto& layout = content_.presentation.explorationConsumables;
        return {static_cast<float>(layout.position.x + layout.rowInset.x + layout.firstItemInset.x) +
                    static_cast<float>(index * static_cast<std::size_t>(layout.itemSpacing)),
                static_cast<float>(layout.position.y + layout.rowInset.y + layout.firstItemInset.y)};
    }

    std::optional<std::size_t> ExplorationConsumablePanel::itemAt(sf::Vector2f position) const
    {
        const auto count = game::EconomyQueries{state_, content_}.consumables(game::ItemSort::name).size();
        for (std::size_t index = 0; index < count; ++index)
        {
            const auto item = itemPosition(index);
            if (sf::FloatRect{item, {100.0F, 100.0F}}.contains(position))
            {
                return index;
            }
        }
        return std::nullopt;
    }
} // namespace soma::presentation
