#pragma once

#include "presentation/sfml/EconomyTextures.hpp"
#include "presentation/sfml/EquipmentTransition.hpp"

#include <SFML/Graphics.hpp>
#include <array>
#include <memory>

namespace soma::presentation
{
    /// Draws persistent and animated color connections around the equipped-item diamond.
    class EquipmentConnectionRenderer
    {
      public:
        explicit EquipmentConnectionRenderer(const EconomyTextures& textures);

        void drawBase(sf::RenderWindow& window, std::size_t index, sf::Vector2f position) const;
        void draw(sf::RenderWindow& window,
                  std::size_t index,
                  sf::Vector2f position,
                  const ConnectionTransitionState& state) const;

      private:
        const EconomyTextures& textures_;
        mutable std::array<std::unique_ptr<sf::RenderTexture>, 4> targets_;
        mutable sf::Shader activationShader_;
        mutable sf::Shader deactivationShader_;
        bool available_{};
    };
} // namespace soma::presentation
