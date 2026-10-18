#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace soma::presentation
{
    struct AnimationBounds
    {
        float left{};
        float top{};
        float width{};
        float height{};
    };

    struct AnimationPoint
    {
        float x{};
        float y{};
    };

    /// Decouples battle presentation timing and attachment points from the Spriter renderer.
    class IAnimationPlayer
    {
      public:
        virtual ~IAnimationPlayer() = default;

        virtual void play(std::string_view animation, float speed = 1.0F) = 0;
        virtual void setPosition(float x, float y) = 0;
        virtual void setScale(float x, float y) = 0;
        virtual void update(float elapsedMilliseconds) = 0;
        virtual void render() = 0;
        [[nodiscard]] virtual AnimationBounds bounds() = 0;
        [[nodiscard]] virtual std::optional<AnimationPoint> point(std::string_view name) = 0;
        [[nodiscard]] virtual bool isPlaying() const = 0;
        [[nodiscard]] virtual std::string currentAnimation() const = 0;
    };
} // namespace soma::presentation
