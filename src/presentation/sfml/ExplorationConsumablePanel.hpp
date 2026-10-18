#pragma once

#include "content/EconomyContent.hpp"
#include "game/CampaignState.hpp"
#include "presentation/sfml/EconomyTextures.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <optional>

namespace soma::presentation
{
    /// Presents draggable consumables during exploration and reports the selected item for use.
    class ExplorationConsumablePanel
    {
      public:
        ExplorationConsumablePanel(const content::EconomyContent& content,
                                   const game::CampaignState& state,
                                   const EconomyTextures& textures,
                                   const sf::Font& font);

        void press(sf::Vector2f position);
        [[nodiscard]] std::optional<gameplay::ItemId> release();
        void playConsumedSound();
        void render(sf::RenderWindow& window, sf::Vector2f mousePosition) const;
        [[nodiscard]] bool clickableAt(sf::Vector2f position) const;

      private:
        [[nodiscard]] sf::Vector2f itemPosition(std::size_t index) const;
        [[nodiscard]] std::optional<std::size_t> itemAt(sf::Vector2f position) const;

        const content::EconomyContent& content_;
        const game::CampaignState& state_;
        const EconomyTextures& textures_;
        const sf::Font& font_;
        std::optional<gameplay::ItemId> dragged_;
        sf::SoundBuffer consumedBuffer_;
        sf::Sound consumedSound_;
    };
} // namespace soma::presentation
