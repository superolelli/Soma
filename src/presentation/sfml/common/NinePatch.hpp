#pragma once

#include <SFML/Graphics.hpp>

namespace soma::presentation
{
    /// Draws a resizable panel while preserving the borders and corners of its source texture.
    class NinePatch final : public sf::Drawable, public sf::Transformable
    {
      public:
        explicit NinePatch(const sf::Texture& texture);

        void setSize(sf::Vector2f size);
        [[nodiscard]] sf::Vector2f size() const noexcept;
        [[nodiscard]] sf::FloatRect globalBounds() const;

      private:
        void rebuild();
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

        const sf::Texture* texture_{};
        sf::VertexArray vertices_{sf::PrimitiveType::Triangles, 54};
        sf::Vector2f size_;
        sf::Vector2f stretchStart_;
        sf::Vector2f stretchEnd_;
    };
} // namespace soma::presentation
