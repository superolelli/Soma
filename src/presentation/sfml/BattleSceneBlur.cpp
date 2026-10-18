#include "presentation/sfml/BattleSceneBlur.hpp"

namespace soma::presentation
{
    BattleSceneBlur::BattleSceneBlur(const sf::RenderWindow& window, sf::Vector2u referenceSize, const std::filesystem::path& shaderPath)
        : referenceSize_(referenceSize)
        , captureSize_(window.getSize())
        , horizontalCapture_(captureSize_)
        , verticalCapture_(captureSize_)
        , available_(shader_.loadFromFile(shaderPath, sf::Shader::Type::Fragment))
    {
        shader_.setUniform("u_ImageSize", sf::Glsl::Vec2{static_cast<float>(captureSize_.x), static_cast<float>(captureSize_.y)});
        shader_.setUniform("u_texture", sf::Shader::CurrentTexture);
        shader_.setUniform("u_kernelSize", 5.0F);
        shader_.setUniform("u_blurringRect", sf::Glsl::Vec4{0.0F, 0.0F, static_cast<float>(captureSize_.x), static_cast<float>(captureSize_.y)});
    }

    bool BattleSceneBlur::available() const noexcept
    {
        return available_;
    }

    void BattleSceneBlur::apply(sf::RenderWindow& window)
    {
        const sf::Vector2f scale{static_cast<float>(referenceSize_.x) / captureSize_.x, static_cast<float>(referenceSize_.y) / captureSize_.y};

        horizontalCapture_.update(window);
        shader_.setUniform("u_horizontalPass", 0.0F);
        window.clear(sf::Color::Black);
        sf::Sprite horizontal{horizontalCapture_};
        horizontal.setScale(scale);
        window.draw(horizontal, &shader_);

        verticalCapture_.update(window);
        shader_.setUniform("u_horizontalPass", 1.0F);
        window.clear(sf::Color::Black);
        sf::Sprite vertical{verticalCapture_};
        vertical.setScale(scale);
        window.draw(vertical, &shader_);
    }
} // namespace soma::presentation
