#pragma once

#include "content/EconomyContent.hpp"
#include "content/RuleContent.hpp"
#include "content/WorldContent.hpp"
#include "presentation/sfml/EconomyTextures.hpp"
#include "simulation/rewards/LevelRewardCalculator.hpp"

#include <SFML/Graphics.hpp>

namespace soma::presentation
{
    /// Summarizes level rewards and newly unlocked content before returning to the main room.
    class LevelResultDialog
    {
      public:
        LevelResultDialog(const content::WorldContent& world,
                          const content::EconomyContent& economy,
                          const content::RuleContent& rules,
                          const EconomyTextures& textures,
                          std::string_view campaign,
                          bool completed,
                          simulation::LevelRewards rewards,
                          std::vector<gameplay::ItemId> unlockedItems,
                          std::vector<content::RuleFragmentType> unlockedRuleTypes);

        [[nodiscard]] bool clicked(sf::Vector2f position) const;
        void render(sf::RenderWindow& window, sf::Vector2f mousePosition) const;

      private:
        [[nodiscard]] sf::FloatRect buttonBounds() const;

        const content::WorldContent::Presentation::LevelResultDialog& layout_;
        sf::Texture panelTexture_;
        sf::Texture buttonTexture_;
        sf::Font titleFont_;
        sf::Font textFont_;
        const content::EconomyContent& economy_;
        const EconomyTextures& textures_;
        bool completed_{};
        simulation::LevelRewards rewards_;
        std::vector<gameplay::ItemId> unlockedItems_;
        std::vector<content::RuleFragmentType> unlockedRuleTypes_;
        std::unordered_map<content::RuleFragmentType, sf::Texture> ruleTextures_;
    };
} // namespace soma::presentation
