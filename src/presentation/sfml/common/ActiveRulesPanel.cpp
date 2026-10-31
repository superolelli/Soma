#include "presentation/sfml/common/ActiveRulesPanel.hpp"

#include "presentation/text/RuleDescriptionBuilder.hpp"

#include <algorithm>
#include <sstream>

namespace soma::presentation
{
    ActiveRulesPanel::ActiveRulesPanel(const content::RuleContent& content,
                                       const gameplay::ComposedRules& rules,
                                       const sf::Font& font)
        : layout_(content.activeRulesPresentation)
        , font_(font)
        , toggleTexture_(content.activeRulesToggle)
        , rowTexture_(content.fragmentBackground)
        , descriptions_(buildGermanActiveRuleDescriptions(rules, content))
    {
    }

    bool ActiveRulesPanel::handleClick(sf::Vector2f position)
    {
        if (toggleBounds().contains(position))
        {
            expanded_ = !expanded_;
            return true;
        }
        return expanded_ && listBounds().contains(position);
    }

    bool ActiveRulesPanel::clickableAt(sf::Vector2f position) const
    {
        return toggleBounds().contains(position);
    }

    bool ActiveRulesPanel::contains(sf::Vector2f position) const
    {
        return toggleBounds().contains(position) || (expanded_ && listBounds().contains(position));
    }

    void ActiveRulesPanel::render(sf::RenderWindow& window, sf::Vector2f mousePosition) const
    {
        if (expanded_)
        {
            const auto rows = std::max<std::size_t>(descriptions_.size(), 1U);
            for (std::size_t index = 0; index < rows; ++index)
            {
                const sf::Vector2f position{static_cast<float>(layout_.listPosition.x),
                                            static_cast<float>(layout_.listPosition.y +
                                                               static_cast<int>(index) * layout_.rowSpacing)};
                sf::Sprite background{rowTexture_};
                background.setPosition(position);
                window.draw(background);

                const auto& value = descriptions_.empty() ? layout_.emptyText : descriptions_.at(index);
                const auto wrappedValue = wrapped(value);
                sf::Text text{font_,
                              sf::String::fromUtf8(wrappedValue.begin(), wrappedValue.end()),
                              static_cast<unsigned int>(layout_.textSize)};
                text.setFillColor(sf::Color{35, 20, 10});
                text.setPosition(position + sf::Vector2f{static_cast<float>(layout_.textInset.x),
                                                         static_cast<float>(layout_.textInset.y)});
                window.draw(text);
            }
        }

        const auto frameWidth = static_cast<int>(toggleTexture_.getSize().x / 2U);
        const auto frame = expanded_ || (clickableAt(mousePosition) && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) ? 1 : 0;
        sf::Sprite toggle{toggleTexture_};
        toggle.setPosition({static_cast<float>(layout_.togglePosition.x), static_cast<float>(layout_.togglePosition.y)});
        toggle.setTextureRect({{frame * frameWidth, 0}, {frameWidth, static_cast<int>(toggleTexture_.getSize().y)}});
        window.draw(toggle);
    }

    sf::FloatRect ActiveRulesPanel::toggleBounds() const
    {
        return {{static_cast<float>(layout_.togglePosition.x), static_cast<float>(layout_.togglePosition.y)},
                {static_cast<float>(toggleTexture_.getSize().x / 2U), static_cast<float>(toggleTexture_.getSize().y)}};
    }

    sf::FloatRect ActiveRulesPanel::listBounds() const
    {
        const auto rows = std::max<std::size_t>(descriptions_.size(), 1U);
        const auto height = static_cast<float>((rows - 1U) * static_cast<std::size_t>(layout_.rowSpacing) + rowTexture_.getSize().y);
        return {{static_cast<float>(layout_.listPosition.x), static_cast<float>(layout_.listPosition.y)},
                {static_cast<float>(rowTexture_.getSize().x), height}};
    }

    std::string ActiveRulesPanel::wrapped(std::string_view value) const
    {
        std::istringstream words{std::string{value}};
        std::string result;
        std::string line;
        std::string word;
        while (words >> word)
        {
            const auto candidate = line.empty() ? word : line + " " + word;
            const sf::Text measurement{font_,
                                       sf::String::fromUtf8(candidate.begin(), candidate.end()),
                                       static_cast<unsigned int>(layout_.textSize)};
            if (!line.empty() && measurement.getLocalBounds().size.x > static_cast<float>(layout_.textWidth))
            {
                result += (result.empty() ? "" : "\n") + line;
                line = word;
            }
            else
            {
                line = candidate;
            }
        }
        if (!line.empty())
        {
            result += (result.empty() ? "" : "\n") + line;
        }
        return result;
    }
} // namespace soma::presentation
