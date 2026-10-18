#pragma once

#include "presentation/text/FormattedText.hpp"

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

namespace soma::presentation
{
    struct FormattedTextLayout
    {
        std::vector<FormattedTextLine> lines;
        sf::Vector2f size;
        unsigned int characterSize{};
        float lineHeight{};
    };

    [[nodiscard]] FormattedTextLayout layoutFormattedText(const sf::Font& font,
                                                          const std::string& value,
                                                          unsigned int characterSize,
                                                          float maximumWidth,
                                                          float lineHeight);
    void drawFormattedText(sf::RenderWindow& window,
                           const sf::Font& font,
                           const FormattedTextLayout& layout,
                           sf::Vector2f position);
}
