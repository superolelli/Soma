#include "presentation/sfml/NinePatch.hpp"

#include <algorithm>
#include <array>

namespace soma::presentation
{
    namespace
    {
        std::pair<float, float> markerRange(const sf::Image& image, bool horizontal)
        {
            const auto limit = horizontal ? image.getSize().x : image.getSize().y;
            auto first = 0U;
            auto last = limit - 2U;
            auto found = false;
            for (auto offset = 1U; offset < limit; ++offset)
            {
                const auto pixel = horizontal ? image.getPixel({offset, 0U}) : image.getPixel({0U, offset});
                if (pixel == sf::Color::Black)
                {
                    first = found ? first : offset - 1U;
                    last = offset - 1U;
                    found = true;
                }
                else if (found)
                {
                    break;
                }
            }
            return {static_cast<float>(first), static_cast<float>(last)};
        }

        void quad(sf::VertexArray& vertices,
                  std::size_t offset,
                  sf::Vector2f topLeft,
                  sf::Vector2f bottomRight,
                  sf::Vector2f textureTopLeft,
                  sf::Vector2f textureBottomRight)
        {
            const std::array positions{topLeft,
                                       sf::Vector2f{bottomRight.x, topLeft.y},
                                       bottomRight,
                                       topLeft,
                                       bottomRight,
                                       sf::Vector2f{topLeft.x, bottomRight.y}};
            const std::array textureCoordinates{textureTopLeft,
                                                sf::Vector2f{textureBottomRight.x, textureTopLeft.y},
                                                textureBottomRight,
                                                textureTopLeft,
                                                textureBottomRight,
                                                sf::Vector2f{textureTopLeft.x, textureBottomRight.y}};
            for (std::size_t index = 0; index < positions.size(); ++index)
            {
                vertices[offset + index].position = positions.at(index);
                vertices[offset + index].texCoords = textureCoordinates.at(index) + sf::Vector2f{1.0F, 1.0F};
                vertices[offset + index].color = sf::Color::White;
            }
        }
    } // namespace

    NinePatch::NinePatch(const sf::Texture& texture)
        : texture_(&texture)
        , size_(sf::Vector2f{texture.getSize()} - sf::Vector2f{2.0F, 2.0F})
    {
        const auto image = texture.copyToImage();
        const auto horizontal = markerRange(image, true);
        const auto vertical = markerRange(image, false);
        stretchStart_ = {horizontal.first, vertical.first};
        stretchEnd_ = {horizontal.second, vertical.second};
        rebuild();
    }

    void NinePatch::setSize(sf::Vector2f size)
    {
        const auto textureSize = sf::Vector2f{texture_->getSize()} - sf::Vector2f{2.0F, 2.0F};
        const auto minimum = stretchStart_ + textureSize - stretchEnd_;
        size_ = {std::max(size.x, minimum.x), std::max(size.y, minimum.y)};
        rebuild();
    }

    sf::Vector2f NinePatch::size() const noexcept
    {
        return size_;
    }

    sf::FloatRect NinePatch::globalBounds() const
    {
        return getTransform().transformRect({{}, size_});
    }

    void NinePatch::rebuild()
    {
        const auto textureSize = sf::Vector2f{texture_->getSize()} - sf::Vector2f{2.0F, 2.0F};
        const auto scaledEnd = size_ - (textureSize - stretchEnd_);
        const std::array x{0.0F, stretchStart_.x, scaledEnd.x, size_.x};
        const std::array y{0.0F, stretchStart_.y, scaledEnd.y, size_.y};
        const std::array tx{0.0F, stretchStart_.x, stretchEnd_.x, textureSize.x};
        const std::array ty{0.0F, stretchStart_.y, stretchEnd_.y, textureSize.y};
        auto vertex = 0U;
        for (auto row = 0U; row < 3U; ++row)
        {
            for (auto column = 0U; column < 3U; ++column)
            {
                quad(vertices_, vertex, {x.at(column), y.at(row)}, {x.at(column + 1), y.at(row + 1)},
                     {tx.at(column), ty.at(row)}, {tx.at(column + 1), ty.at(row + 1)});
                vertex += 6U;
            }
        }
    }

    void NinePatch::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        states.texture = texture_;
        states.transform *= getTransform();
        target.draw(vertices_, states);
    }
} // namespace soma::presentation
