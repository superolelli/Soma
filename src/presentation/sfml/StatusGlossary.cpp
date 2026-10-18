#include "presentation/sfml/StatusGlossary.hpp"

#include "presentation/sfml/FormattedTextLayout.hpp"

#include <algorithm>

namespace soma::presentation
{
    namespace
    {
        constexpr float outerPadding = 14.0F;
        constexpr float iconColumnWidth = 38.0F;
        constexpr float rowGap = 10.0F;
        constexpr float iconSize = 28.0F;

        std::string formattedDetail(const AbilityStatusDetail& detail)
        {
            return "*#" + detail.color + ' ' + detail.name + "*\n#dddddd " + detail.explanation;
        }

        sf::Sprite scaledIcon(const sf::Texture& texture, sf::Vector2f position)
        {
            sf::Sprite icon{texture};
            const auto textureSize = sf::Vector2f{texture.getSize()};
            const auto maximumDimension = std::max(textureSize.x, textureSize.y);
            const auto scale = maximumDimension > 0.0F ? iconSize / maximumDimension : 1.0F;
            icon.setScale({scale, scale});
            icon.setPosition(position + sf::Vector2f{(iconSize - textureSize.x * scale) / 2.0F,
                                                     (iconSize - textureSize.y * scale) / 2.0F});
            return icon;
        }

        bool intersects(const sf::FloatRect& left, const sf::FloatRect& right)
        {
            return left.position.x < right.position.x + right.size.x && left.position.x + left.size.x > right.position.x &&
                   left.position.y < right.position.y + right.size.y && left.position.y + left.size.y > right.position.y;
        }

        void drawIconBacking(sf::RenderWindow& window, sf::Vector2f position)
        {
            sf::RectangleShape backing{{iconSize + 6.0F, iconSize + 6.0F}};
            backing.setPosition(position - sf::Vector2f{3.0F, 3.0F});
            backing.setFillColor(sf::Color{225, 205, 150, 235});
            backing.setOutlineColor(sf::Color{105, 80, 38, 230});
            backing.setOutlineThickness(1.0F);
            window.draw(backing);
        }
    }

    sf::FloatRect drawStatusGlossary(sf::RenderWindow& window,
                                     const sf::Font& font,
                                     const std::vector<AbilityStatusDetail>& details,
                                     const StatusIconLookup& iconForStatus,
                                     sf::Vector2f topRight,
                                     std::optional<sf::FloatRect> avoid,
                                     float width)
    {
        constexpr float titleHeight = 28.0F;
        const auto textWidth = width - outerPadding * 2.0F - iconColumnWidth;
        std::vector<FormattedTextLayout> layouts;
        layouts.reserve(details.size());
        auto height = outerPadding + titleHeight;
        for (const auto& detail : details)
        {
            auto layout = layoutFormattedText(font, formattedDetail(detail), 16U, textWidth, 22.0F);
            height += std::max(iconSize, layout.size.y) + rowGap;
            layouts.push_back(std::move(layout));
        }
        height += outerPadding - rowGap;

        const auto view = window.getView();
        const auto viewOrigin = view.getCenter() - view.getSize() / 2.0F;
        const auto maximumX = std::max(viewOrigin.x + 5.0F, viewOrigin.x + view.getSize().x - width - 5.0F);
        const auto maximumY = std::max(viewOrigin.y + 5.0F, viewOrigin.y + view.getSize().y - height - 5.0F);
        auto position = sf::Vector2f{std::clamp(topRight.x - width, viewOrigin.x + 5.0F, maximumX),
                                     std::clamp(topRight.y, viewOrigin.y + 5.0F, maximumY)};
        if (avoid.has_value() && intersects({position, {width, height}}, *avoid))
        {
            const auto leftPosition = sf::Vector2f{viewOrigin.x + 20.0F, position.y};
            if (!intersects({leftPosition, {width, height}}, *avoid))
            {
                position = leftPosition;
            }
            else
            {
                const auto below = avoid->position.y + avoid->size.y + 10.0F;
                const auto above = avoid->position.y - height - 10.0F;
                position.y = below <= maximumY ? below : std::max(viewOrigin.y + 5.0F, above);
            }
        }

        sf::RectangleShape background{{width, height}};
        background.setPosition(position);
        background.setFillColor(sf::Color{15, 15, 15, 238});
        background.setOutlineColor(sf::Color{205, 175, 95});
        background.setOutlineThickness(2.0F);
        window.draw(background);

        auto title = layoutFormattedText(font, "*#d6b45c Statusdetails*", 17U, textWidth, 23.0F);
        drawFormattedText(window, font, title, position + sf::Vector2f{outerPadding, outerPadding});

        auto y = position.y + outerPadding + titleHeight;
        for (std::size_t index = 0; index < details.size(); ++index)
        {
            const auto rowHeight = std::max(iconSize, layouts.at(index).size.y);
            if (const auto* texture = iconForStatus(details.at(index).status))
            {
                const auto iconPosition = sf::Vector2f{position.x + outerPadding, y + (rowHeight - iconSize) / 2.0F};
                drawIconBacking(window, iconPosition);
                window.draw(scaledIcon(*texture, iconPosition));
            }
            drawFormattedText(window,
                              font,
                              layouts.at(index),
                              {position.x + outerPadding + iconColumnWidth, y + (rowHeight - layouts.at(index).size.y) / 2.0F});
            y += rowHeight + rowGap;
        }
        return {position, {width, height}};
    }
}
