#include "presentation/sfml/battle/BattleHud.hpp"
#include "presentation/sfml/common/FormattedTextLayout.hpp"

#include <algorithm>

namespace soma::presentation
{
    sf::Vector2f tooltipSize(const sf::Font& font, const std::string& value, float maximumContentWidth)
    {
        const auto layout = layoutFormattedText(font, value, 16U, maximumContentWidth, 23.0F);
        return {layout.size.x + 24.0F, layout.size.y + 16.0F};
    }

    sf::FloatRect drawTooltip(sf::RenderWindow& window,
                              const sf::Font& font,
                              const std::string& value,
                              sf::Vector2f position,
                              bool alignBottom,
                              float maximumContentWidth)
    {
        const auto layout = layoutFormattedText(font, value, 16U, maximumContentWidth, 23.0F);
        const auto size = sf::Vector2f{layout.size.x + 24.0F, layout.size.y + 16.0F};
        if (alignBottom)
        {
            position.y -= size.y;
        }
        const auto view = window.getView();
        const auto viewOrigin = view.getCenter() - view.getSize() / 2.0F;
        const auto maximumX = std::max(viewOrigin.x + 5.0F, viewOrigin.x + view.getSize().x - size.x - 5.0F);
        const auto maximumY = std::max(viewOrigin.y + 5.0F, viewOrigin.y + view.getSize().y - size.y - 5.0F);
        position.x = std::clamp(position.x, viewOrigin.x + 5.0F, maximumX);
        position.y = std::clamp(position.y, viewOrigin.y + 5.0F, maximumY);
        sf::RectangleShape background{size};
        background.setPosition(position);
        background.setFillColor(sf::Color{0, 0, 0, 230});
        background.setOutlineColor(sf::Color{70, 70, 70});
        background.setOutlineThickness(2.0F);
        window.draw(background);

        drawFormattedText(window, font, layout, position + sf::Vector2f{12.0F, 8.0F});
        return {position, size};
    }

} // namespace soma::presentation
