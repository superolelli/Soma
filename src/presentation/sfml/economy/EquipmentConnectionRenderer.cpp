#include "presentation/sfml/economy/EquipmentConnectionRenderer.hpp"

namespace soma::presentation
{
    namespace
    {
        constexpr std::array<std::string_view, 4> assets{"inventory.connection_upper",
                                                         "inventory.connection_right",
                                                         "inventory.connection_lower",
                                                         "inventory.connection_left"};
        const std::array<std::array<sf::Glsl::Vec2, 2>, 4> emitters{
            std::array<sf::Glsl::Vec2, 2>{sf::Glsl::Vec2{0.005F, 0.98F}, sf::Glsl::Vec2{0.98F, 0.98F}},
            std::array<sf::Glsl::Vec2, 2>{sf::Glsl::Vec2{0.98F, 0.98F}, sf::Glsl::Vec2{0.98F, 0.005F}},
            std::array<sf::Glsl::Vec2, 2>{sf::Glsl::Vec2{0.98F, 0.005F}, sf::Glsl::Vec2{0.005F, 0.005F}},
            std::array<sf::Glsl::Vec2, 2>{sf::Glsl::Vec2{0.005F, 0.005F}, sf::Glsl::Vec2{0.005F, 0.98F}}};
    } // namespace

    EquipmentConnectionRenderer::EquipmentConnectionRenderer(const EconomyTextures& textures)
        : textures_(textures),
          available_(activationShader_.loadFromFile("Data/Shader/equipment_connections_activate.frag", sf::Shader::Type::Fragment) &&
                     deactivationShader_.loadFromFile("Data/Shader/equipment_connections_deactivate.frag", sf::Shader::Type::Fragment))
    {
        for (std::size_t index = 0; index < targets_.size(); ++index)
        {
            targets_.at(index) = std::make_unique<sf::RenderTexture>(textures_.at(assets.at(index)).getSize());
        }
    }

    void EquipmentConnectionRenderer::drawBase(sf::RenderWindow& window, std::size_t index, sf::Vector2f position) const
    {
        sf::Sprite base{textures_.at(assets.at(index))};
        base.setPosition(position);
        base.setColor(sf::Color::Black);
        window.draw(base);
    }

    void EquipmentConnectionRenderer::draw(sf::RenderWindow& window,
                                             std::size_t index,
                                             sf::Vector2f position,
                                             const ConnectionTransitionState& state) const
    {
        const auto& texture = textures_.at(assets.at(index));
        if (!available_)
        {
            sf::Sprite sprite{texture};
            sprite.setPosition(position);
            sprite.setColor(state.deactivating ? sf::Color::Black : state.color);
            window.draw(sprite);
            return;
        }
        auto& target = *targets_.at(index);
        auto& shader = state.deactivating ? deactivationShader_ : activationShader_;
        shader.setUniform("u_resolution", sf::Glsl::Vec2{static_cast<float>(texture.getSize().x), static_cast<float>(texture.getSize().y)});
        shader.setUniform("u_baseTexture", texture);
        shader.setUniform("u_progress", state.progress);
        shader.setUniform("u_emitterColor", sf::Glsl::Vec4{state.color});
        if (state.deactivating)
        {
            shader.setUniform("u_emitter", emitters.at(index).front());
        }
        else
        {
            shader.setUniformArray("u_emitter", emitters.at(index).data(), emitters.at(index).size());
        }
        target.clear(sf::Color::Transparent);
        target.draw(sf::Sprite{texture}, &shader);
        target.display();
        sf::Sprite result{target.getTexture()};
        result.setPosition(position);
        window.draw(result);
    }
} // namespace soma::presentation
