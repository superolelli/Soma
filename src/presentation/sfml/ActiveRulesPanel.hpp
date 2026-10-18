#pragma once

#include "content/RuleContent.hpp"
#include "game/RuleState.hpp"

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

namespace soma::presentation
{
    /// Provides a compact, read-only drawer for the rules that currently affect a run.
    class ActiveRulesPanel
    {
      public:
        ActiveRulesPanel(const content::RuleContent& content, const game::ComposedRules& rules, const sf::Font& font);

        [[nodiscard]] bool handleClick(sf::Vector2f position);
        [[nodiscard]] bool clickableAt(sf::Vector2f position) const;
        [[nodiscard]] bool contains(sf::Vector2f position) const;
        void render(sf::RenderWindow& window, sf::Vector2f mousePosition) const;

      private:
        [[nodiscard]] sf::FloatRect toggleBounds() const;
        [[nodiscard]] sf::FloatRect listBounds() const;
        [[nodiscard]] std::string wrapped(std::string_view value) const;

        const content::RuleContent::ActiveRulesPresentation& layout_;
        const sf::Font& font_;
        sf::Texture toggleTexture_;
        sf::Texture rowTexture_;
        std::vector<std::string> descriptions_;
        bool expanded_{};
    };
} // namespace soma::presentation
