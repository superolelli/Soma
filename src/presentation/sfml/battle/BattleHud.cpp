#include "presentation/sfml/battle/BattleHud.hpp"
#include "presentation/sfml/battle/BattleHudDrawing.hpp"

#include <algorithm>

namespace soma::presentation
{
    using simulation::CombatantState;

    void drawHealthBar(sf::RenderWindow& window, const BattleTextures& textures, const CombatantView& view)
    {
        const auto x = view.bounds.position.x + view.bounds.size.x / 2.0F - textures.healthBarFrame.getSize().x / 2.0F;
        const auto y = view.bounds.position.y + view.bounds.size.y + 30.0F;
        const auto ratio = std::clamp(static_cast<float>(view.displayedHealth) / static_cast<float>(view.maximumHealth), 0.0F, 1.0F);
        auto fill = detail::hudSprite(textures.healthBar, {x, y});
        const auto fillWidth = textures.healthBar.getSize().x / 2U;
        fill.setTextureRect({{0, 0}, {static_cast<int>(fillWidth * ratio), static_cast<int>(textures.healthBar.getSize().y)}});
        window.draw(fill);
        window.draw(detail::hudSprite(textures.healthBarFrame, {x, y}));
    }

    sf::Vector2f fatigueBarPosition(const BattleTextures& textures, const CombatantView& view)
    {
        const auto width = static_cast<float>(textures.fatigueBarFrame.getSize().x);
        const auto healthY = view.bounds.position.y + view.bounds.size.y + 30.0F;
        return {view.bounds.position.x + view.bounds.size.x / 2.0F - width / 2.0F,
                healthY + static_cast<float>(textures.healthBarFrame.getSize().y) + 2.0F};
    }

    void drawFatigueBar(sf::RenderWindow& window, const BattleTextures& textures, const CombatantState& state, const CombatantView& view)
    {
        if (state.team != gameplay::Team::players)
        {
            return;
        }
        const auto position = fatigueBarPosition(textures, view);
        const auto fillWidth = textures.fatigueBar.getSize().x / 2U;
        const auto ratio = std::clamp(view.displayedFatigue / static_cast<float>(gameplay::maximumFatigue), 0.0F, 1.0F);
        const auto visibleWidth = static_cast<unsigned int>(static_cast<float>(fillWidth) * ratio);
        if (visibleWidth > 0U)
        {
            auto fill = detail::hudSprite(textures.fatigueBar, position);
            fill.setTextureRect({{0, 0}, {static_cast<int>(visibleWidth), static_cast<int>(textures.fatigueBar.getSize().y)}});
            window.draw(fill);
        }
        window.draw(detail::hudSprite(textures.fatigueBarFrame, position));
    }

    void drawCombatantShadow(sf::RenderWindow& window, const CombatantView& view)
    {
        const auto fallback = AnimationPoint{view.bounds.position.x + view.bounds.size.x / 2.0F,
                                             view.bounds.position.y + view.bounds.size.y};
        const auto shadowPoint = view.animation->point("shadow_point").value_or(fallback);
        sf::CircleShape shadow{view.bounds.size.x / 2.0F + 10.0F, 60U};
        shadow.setScale({1.0F, 0.2F});
        shadow.setFillColor(sf::Color{0, 0, 0, 100});
        const auto bounds = shadow.getGlobalBounds();
        shadow.setPosition({shadowPoint.x - bounds.size.x / 2.0F, shadowPoint.y - bounds.size.y / 2.0F});
        window.draw(shadow);
    }
} // namespace soma::presentation
