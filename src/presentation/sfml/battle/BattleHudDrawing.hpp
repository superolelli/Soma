#pragma once

#include <SFML/Graphics.hpp>
#include <string>

namespace soma::presentation::detail
{
    inline sf::Text hudText(const sf::Font& font,
                            const std::string& value,
                            unsigned int size,
                            sf::Vector2f position,
                            sf::Color color = sf::Color::White)
    {
        sf::Text result{font, sf::String::fromUtf8(value.begin(), value.end()), size};
        result.setPosition(position);
        result.setFillColor(color);
        result.setOutlineColor(sf::Color::Black);
        result.setOutlineThickness(3.0F);
        return result;
    }

    inline sf::Vector2f textureSize(const sf::Texture& texture)
    {
        return {static_cast<float>(texture.getSize().x), static_cast<float>(texture.getSize().y)};
    }

    inline sf::Sprite hudSprite(const sf::Texture& texture, sf::Vector2f position)
    {
        sf::Sprite result{texture};
        result.setPosition(position);
        return result;
    }
} // namespace soma::presentation::detail
