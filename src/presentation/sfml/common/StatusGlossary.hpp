#pragma once

#include "presentation/text/AbilityDescription.hpp"

#include <SFML/Graphics.hpp>
#include <functional>
#include <optional>
#include <vector>

namespace soma::presentation
{
    using StatusIconLookup = std::function<const sf::Texture*(const gameplay::StatusId&)>;

    [[nodiscard]] sf::FloatRect drawStatusGlossary(sf::RenderWindow& window,
                                                   const sf::Font& font,
                                                   const std::vector<AbilityStatusDetail>& details,
                                                   const StatusIconLookup& iconForStatus,
                                                   sf::Vector2f topRight,
                                                   std::optional<sf::FloatRect> avoid = std::nullopt,
                                                   float width = 420.0F);
}
