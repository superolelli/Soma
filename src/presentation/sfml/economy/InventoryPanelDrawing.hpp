#pragma once

#include <SFML/Graphics.hpp>
#include <string>

namespace soma::presentation::detail
{
    inline sf::Text inventoryText(const sf::Font& font,
                                  const std::string& value,
                                  unsigned int size,
                                  sf::Vector2f position,
                                  sf::Color color = sf::Color::White)
    {
        sf::Text text{font, sf::String::fromUtf8(value.begin(), value.end()), size};
        text.setPosition(position);
        text.setFillColor(color);
        return text;
    }

    inline sf::Sprite inventoryButtonSprite(const sf::Texture& texture, sf::Vector2f position, unsigned int frames)
    {
        sf::Sprite sprite{texture};
        const auto width = texture.getSize().x / frames;
        sprite.setTextureRect(sf::IntRect{{0, 0}, {static_cast<int>(width), static_cast<int>(texture.getSize().y)}});
        sprite.setPosition(position);
        return sprite;
    }
} // namespace soma::presentation::detail
