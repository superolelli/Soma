#pragma once

#include "content/BattleContent.hpp"
#include "content/EconomyContent.hpp"
#include "game/CampaignState.hpp"
#include "game/EconomyQueries.hpp"
#include "game/EconomyService.hpp"
#include "presentation/sfml/EconomyTextures.hpp"
#include "presentation/sfml/EconomySoundPlayer.hpp"

#include <SFML/Graphics.hpp>
#include <array>
#include <functional>
#include <string>
#include <string_view>

namespace soma::presentation
{
    /// Displays ability variants and skill trees, purchases skills, and selects active versions.
    class SkillPanel
    {
      public:
        SkillPanel(sf::RenderWindow& window,
                   const content::BattleContent& battleContent,
                   const content::EconomyContent& content,
                   game::CampaignState& state,
                   game::EconomyService& economy,
                   const EconomyTextures& textures,
                   const sf::Font& interfaceFont,
                   const sf::Font& titleFont,
                   const sf::Font& abilityFont,
                   const sf::Font& priceFont,
                   bool closeWindowOnExit = true,
                   std::function<void()> drawBackground = {},
                   std::function<bool(sf::Vector2f)> navigate = {},
                   std::function<bool(sf::Vector2f)> navigationHovered = {});

        void processEvents();
        void render();
        [[nodiscard]] bool closeRequested() const noexcept;
        void showPlayer(std::size_t index);

      private:
        [[nodiscard]] gameplay::ContentDefinitionId currentPlayerId() const;
        [[nodiscard]] const content::AbilitySetDefinition& currentAbilitySet() const;
        [[nodiscard]] gameplay::AbilityId currentAbilityId(std::size_t version) const;
        [[nodiscard]] std::vector<game::SkillEntry> currentSkills(std::size_t version) const;
        [[nodiscard]] content::AbilityDefinition abilityForVersion(std::size_t version) const;
        [[nodiscard]] sf::Vector2f absolute(content::Point relative) const;
        [[nodiscard]] sf::FloatRect absoluteBounds(content::Rectangle relative) const;
        [[nodiscard]] sf::FloatRect boundsAt(std::string_view asset, content::Point relative) const;
        [[nodiscard]] sf::FloatRect versionSelectorBounds(std::size_t version) const;
        [[nodiscard]] bool canBuy(const game::SkillEntry& skill) const;
        [[nodiscard]] bool clickableAt(sf::Vector2f position) const;
        void handleClick(sf::Vector2f position);
        void requestClose();
        void choosePreviousPlayer();
        void chooseNextPlayer();
        void buySelectedSkill();
        void selectVersion(std::size_t version);
        void drawBackground() const;
        void drawAbilityBridge() const;
        void drawPlayerAndAbilities() const;
        void drawSkillGraph() const;
        void drawSkills() const;
        void drawSelectionDetails() const;
        void drawHoveredSkillTooltip() const;
        void drawSelectedAbilityTooltip() const;
        void drawVersionSelectorTooltip() const;
        [[nodiscard]] sf::FloatRect drawTooltip(const std::string& description, sf::Vector2f preferredPosition) const;

        sf::RenderWindow& window_;
        const content::BattleContent& battleContent_;
        const content::EconomyContent& content_;
        game::CampaignState& state_;
        game::EconomyService& economy_;
        const EconomyTextures& textures_;
        const sf::Font& interfaceFont_;
        const sf::Font& titleFont_;
        const sf::Font& abilityFont_;
        const sf::Font& priceFont_;
        std::size_t playerIndex_{};
        std::size_t abilityIndex_{};
        std::size_t selectedVersion_{};
        std::size_t skillIndex_{};
        sf::Vector2f mousePosition_{};
        bool mousePressed_{};
        EconomySoundPlayer sounds_;
        bool closeWindowOnExit_{};
        bool closeRequested_{};
        std::function<void()> drawBackground_;
        std::function<bool(sf::Vector2f)> navigate_;
        std::function<bool(sf::Vector2f)> navigationHovered_;
        const std::array<std::string_view, 4> playerKeys_{"simon", "ole", "anna", "markus"};
        const std::array<std::string_view, 4> playerNames_{"Simon", "Ole", "Anna", "Markus"};
    };
} // namespace soma::presentation
