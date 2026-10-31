#pragma once

#include "presentation/animation/IAnimationPlayer.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <filesystem>
#include <memory>
#include <string>

namespace sf
{
    class RenderTarget;
}

namespace soma::presentation
{
    /// Adapts SpriterPlusPlus models, attachment points, and embedded sounds to game presentation.
    class SpriterAnimationPlayer final : public IAnimationPlayer
    {
      public:
        SpriterAnimationPlayer(sf::RenderTarget& target,
                               const std::filesystem::path& modelPath,
                               std::string entityName);
        ~SpriterAnimationPlayer() override;

        SpriterAnimationPlayer(const SpriterAnimationPlayer&) = delete;
        SpriterAnimationPlayer& operator=(const SpriterAnimationPlayer&) = delete;

        void play(std::string_view animation, float speed) override;
        void setPosition(float x, float y) override;
        void setScale(float x, float y) override;
        void update(float elapsedMilliseconds) override;
        void render() override;
        void renderVisual();
        void setTint(sf::Color color);
        void setRenderOffset(sf::Vector2f offset);
        [[nodiscard]] AnimationBounds bounds() override;
        [[nodiscard]] std::optional<AnimationPoint> point(std::string_view name) override;
        [[nodiscard]] bool isPlaying() const override;
        [[nodiscard]] std::string currentAnimation() const override;

      private:
        class Implementation;
        std::unique_ptr<Implementation> implementation_;
    };
} // namespace soma::presentation
