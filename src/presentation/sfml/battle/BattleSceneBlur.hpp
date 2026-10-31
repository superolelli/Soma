#pragma once

#include <SFML/Graphics.hpp>
#include <filesystem>

namespace soma::presentation
{
    /// Captures and blurs the complete battle scene behind foreground action animations.
    class BattleSceneBlur
    {
      public:
        BattleSceneBlur(const sf::RenderWindow& window, sf::Vector2u referenceSize, const std::filesystem::path& shaderPath);

        [[nodiscard]] bool available() const noexcept;
        void apply(sf::RenderWindow& window);

      private:
        sf::Vector2u referenceSize_;
        sf::Vector2u captureSize_;
        sf::Shader shader_;
        sf::Texture horizontalCapture_;
        sf::Texture verticalCapture_;
        bool available_{};
    };
} // namespace soma::presentation
