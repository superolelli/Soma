#pragma once

#include <SFML/Graphics.hpp>

namespace soma::presentation
{
    enum class EconomyButtonBehavior
    {
        up,
        motionUp
    };

    inline sf::Sprite economyButton(const sf::Texture& texture,
                                    sf::Vector2f position,
                                    EconomyButtonBehavior behavior,
                                    sf::Vector2f mousePosition,
                                    bool mousePressed,
                                    bool enabled = true)
    {
        const auto frames = behavior == EconomyButtonBehavior::up ? 2U : 3U;
        const auto width = texture.getSize().x / frames;
        const sf::FloatRect bounds{position, {static_cast<float>(width), static_cast<float>(texture.getSize().y)}};
        const auto hovered = enabled && bounds.contains(mousePosition);
        const auto frame = behavior == EconomyButtonBehavior::up ? (hovered && mousePressed ? 1U : 0U)
                                                                  : (hovered && mousePressed ? 2U : 1U);
        sf::Sprite sprite{texture};
        sprite.setTextureRect(sf::IntRect{{static_cast<int>(width * frame), 0},
                                          {static_cast<int>(width), static_cast<int>(texture.getSize().y)}});
        sprite.setPosition(position);
        if (!enabled)
        {
            sprite.setColor(sf::Color{90, 90, 90});
        }
        return sprite;
    }
} // namespace soma::presentation
