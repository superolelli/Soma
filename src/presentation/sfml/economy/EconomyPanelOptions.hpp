#pragma once

#include <SFML/Graphics.hpp>

#include <functional>

namespace soma::presentation
{
    /// Names the font roles shared by the inventory, skill, and shop panels.
    struct EconomyPanelFonts
    {
        const sf::Font& interface;
        const sf::Font& title;
        const sf::Font& ability;
        const sf::Font& price;
    };

    /// Configures whether an economy panel is standalone or embedded in another screen.
    struct PanelHostOptions
    {
        bool closeWindowOnExit{true};
        std::function<void()> drawBackground;
        std::function<bool(sf::Vector2f)> navigate;
        std::function<bool(sf::Vector2f)> navigationHovered;
    };
} // namespace soma::presentation
