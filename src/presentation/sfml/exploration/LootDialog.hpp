#pragma once

#include "content/economy/EconomyContent.hpp"
#include "content/world/WorldContent.hpp"
#include "game/EconomyService.hpp"
#include "game/RuleBuilderService.hpp"
#include "presentation/sfml/economy/EconomySoundPlayer.hpp"
#include "presentation/sfml/economy/EconomyTextures.hpp"
#include "simulation/rewards/LootGenerator.hpp"

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace soma::presentation
{
    /// Presents battle or exploration loot and transfers selected rewards into campaign state.
    class LootDialog
    {
      public:
        LootDialog(const content::WorldContent& world,
                   const content::EconomyContent& economy,
                   const content::RuleContent& rules,
                   const EconomyTextures& textures,
                   EconomySoundPlayer& sounds,
                   game::EconomyService& economyService,
                   game::RuleBuilderService& ruleService,
                   std::string title,
                   std::optional<std::filesystem::path> openingSound,
                   simulation::LootBundle loot);

        void handleClick(sf::Vector2f position);
        void render(sf::RenderWindow& window, sf::Vector2f mousePosition) const;

        [[nodiscard]] bool closed() const noexcept;
        [[nodiscard]] bool clickableAt(sf::Vector2f position) const;

      private:
        [[nodiscard]] sf::FloatRect itemBounds(std::size_t index) const;
        bool take(std::size_t index, bool playFeedback = true);
        void takeAll();

        const content::WorldContent& world_;
        const content::EconomyContent& economy_;
        const content::RuleContent& rules_;
        const EconomyTextures& textures_;
        EconomySoundPlayer& sounds_;
        game::EconomyService& economyService_;
        game::RuleBuilderService& ruleService_;
        const content::WorldContent::Presentation::LootDialog& layout_;
        sf::Texture panelTexture_;
        sf::Texture closeTexture_;
        sf::Texture takeAllTexture_;
        sf::Font font_;
        std::optional<sf::SoundBuffer> openingSoundBuffer_;
        sf::SoundBuffer itemSoundBuffer_;
        std::unique_ptr<sf::Sound> openingSound_;
        std::unique_ptr<sf::Sound> feedbackSound_;
        std::string title_;
        std::vector<std::optional<simulation::ItemGrant>> items_;
        std::optional<gameplay::RuleFragmentId> ruleFragment_;
        std::unordered_map<content::RuleFragmentType, sf::Texture> ruleTextures_;
        std::string error_;
        bool closed_{};
    };
} // namespace soma::presentation
