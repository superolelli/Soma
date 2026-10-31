#include "presentation/sfml/exploration/ExplorationPartyStatus.hpp"

#include <algorithm>

namespace soma::presentation
{
    ExplorationPartyStatus::ExplorationPartyStatus(const content::WorldContent& world, const sf::Font& font)
        : layout_(world.presentation.partyStatus)
        , font_(font)
        , healthBar_(world.assets.at(layout_.healthBar))
        , healthBarFrame_(world.assets.at(layout_.healthBarFrame))
        , fatigueBar_(world.assets.at(layout_.fatigueBar))
        , fatigueBarFrame_(world.assets.at(layout_.fatigueBarFrame))
        , fatigueTired_(world.assets.at(layout_.fatigueTired))
        , fatigueStupid_(world.assets.at(layout_.fatigueStupid))
        , buff_(world.assets.at(layout_.buff))
    {
    }

    void ExplorationPartyStatus::update(sf::Time elapsed,
                                        const std::array<int, 4>& health,
                                        const std::array<int, 4>& fatigue,
                                        const std::array<bool, 4>& buffed)
    {
        constexpr float healthDuration = 0.7F;
        for (std::size_t index = 0; index < health.size(); ++index)
        {
            if (displayedHealth_.at(index) < 0.0F)
            {
                displayedHealth_.at(index) = static_cast<float>(health.at(index));
                healthStart_.at(index) = displayedHealth_.at(index);
                healthTarget_.at(index) = health.at(index);
            }
            else if (healthTarget_.at(index) != health.at(index))
            {
                healthStart_.at(index) = displayedHealth_.at(index);
                healthTarget_.at(index) = health.at(index);
                healthElapsed_.at(index) = 0.0F;
            }
            healthElapsed_.at(index) = std::min(healthDuration, healthElapsed_.at(index) + elapsed.asSeconds());
            const auto progress = healthElapsed_.at(index) / healthDuration;
            displayedHealth_.at(index) = healthStart_.at(index) + (static_cast<float>(healthTarget_.at(index)) - healthStart_.at(index)) * progress;
            if (displayedFatigue_.at(index) < 0.0F)
            {
                displayedFatigue_.at(index) = static_cast<float>(fatigue.at(index));
                fatigueStart_.at(index) = displayedFatigue_.at(index);
                fatigueTarget_.at(index) = fatigue.at(index);
            }
            else if (fatigueTarget_.at(index) != fatigue.at(index))
            {
                fatigueStart_.at(index) = displayedFatigue_.at(index);
                fatigueTarget_.at(index) = fatigue.at(index);
                fatigueElapsed_.at(index) = 0.0F;
            }
            fatigueElapsed_.at(index) = std::min(healthDuration, fatigueElapsed_.at(index) + elapsed.asSeconds());
            const auto fatigueProgress = fatigueElapsed_.at(index) / healthDuration;
            displayedFatigue_.at(index) =
                fatigueStart_.at(index) + (static_cast<float>(fatigueTarget_.at(index)) - fatigueStart_.at(index)) * fatigueProgress;
            updateTransition(buffTransitions_.at(index), buffed.at(index), elapsed.asSeconds());
            const auto stage = gameplay::fatigueStage(fatigue.at(index));
            updateTransition(tiredTransitions_.at(index), stage == 1, elapsed.asSeconds());
            updateTransition(stupidTransitions_.at(index), stage >= 2, elapsed.asSeconds());
        }
    }

    void ExplorationPartyStatus::render(sf::RenderTarget& target,
                                        const std::array<AnimationBounds, 4>& bounds,
                                        const std::array<int, 4>& health,
                                        const std::array<int, 4>& maximumHealth,
                                        sf::Vector2f mouse) const
    {
        for (std::size_t index = 0; index < bounds.size(); ++index)
        {
            if (health.at(index) > 0)
            {
                renderMember(target, bounds.at(index), maximumHealth.at(index), index, mouse);
            }
        }
    }

    void ExplorationPartyStatus::renderMember(
        sf::RenderTarget& target, const AnimationBounds& bounds, int maximumHealth, std::size_t index, sf::Vector2f mouse) const
    {
        const auto x = bounds.left + bounds.width / 2.0F - static_cast<float>(healthBarFrame_.getSize().x) / 2.0F;
        const auto y = bounds.top + bounds.height + static_cast<float>(layout_.verticalOffset);
        const auto ratio = std::clamp(displayedHealth_.at(index) / static_cast<float>(maximumHealth), 0.0F, 1.0F);
        sf::Sprite fill{healthBar_};
        fill.setPosition({x, y});
        const auto fillWidth = healthBar_.getSize().x / 2U;
        fill.setTextureRect({{0, 0}, {static_cast<int>(static_cast<float>(fillWidth) * ratio), static_cast<int>(healthBar_.getSize().y)}});
        target.draw(fill);
        sf::Sprite frame{healthBarFrame_};
        frame.setPosition({x, y});
        target.draw(frame);

        const auto fatigueY = y + static_cast<float>(healthBarFrame_.getSize().y) + 2.0F;
        const auto fatigueRatio = std::clamp(displayedFatigue_.at(index) / static_cast<float>(gameplay::maximumFatigue), 0.0F, 1.0F);
        const auto fatigueFillWidth = fatigueBar_.getSize().x / 2U;
        const auto visibleWidth = static_cast<unsigned int>(static_cast<float>(fatigueFillWidth) * fatigueRatio);
        if (visibleWidth > 0U)
        {
            sf::Sprite fatigueFill{fatigueBar_};
            fatigueFill.setPosition({x, fatigueY});
            fatigueFill.setTextureRect({{0, 0}, {static_cast<int>(visibleWidth), static_cast<int>(fatigueBar_.getSize().y)}});
            target.draw(fatigueFill);
        }
        sf::Sprite fatigueFrame{fatigueBarFrame_};
        fatigueFrame.setPosition({x, fatigueY});
        target.draw(fatigueFrame);

        auto iconPosition = sf::Vector2f{x, fatigueY + static_cast<float>(fatigueBarFrame_.getSize().y + layout_.statusGap)};
        const auto drawStatus = [&](const sf::Texture& texture, const StatusTransition& transition, std::string_view tooltip) {
            if (!transition.visible)
            {
                return;
            }
            sf::Sprite icon{texture};
            const auto size = sf::Vector2f{texture.getSize()};
            const auto scale = transitionScale(transition);
            icon.setOrigin(size / 2.0F);
            icon.setScale({scale, scale});
            icon.setPosition(iconPosition + size / 2.0F);
            target.draw(icon);
            if (sf::FloatRect{iconPosition, size}.contains(mouse))
            {
                renderTooltip(target, tooltip, {x, y - 4.0F});
            }
            iconPosition.x += 20.0F;
        };
        drawStatus(fatigueTired_, tiredTransitions_.at(index), "Müde: Abzüge durch Müdigkeit");
        drawStatus(fatigueStupid_, stupidTransitions_.at(index), "Erschöpft: starke Abzüge durch Müdigkeit");
        drawStatus(buff_, buffTransitions_.at(index), "Gestärkt: temporäre Attributsboni");
    }

    void ExplorationPartyStatus::renderTooltip(sf::RenderTarget& target, std::string_view value, sf::Vector2f position) const
    {
        sf::Text text{font_, sf::String::fromUtf8(value.begin(), value.end()), 16U};
        text.setFillColor(sf::Color::White);
        text.setOutlineColor(sf::Color::Black);
        text.setOutlineThickness(3.0F);
        const auto bounds = text.getLocalBounds();
        text.setPosition({position.x, position.y - bounds.size.y - bounds.position.y});
        target.draw(text);
    }

    void ExplorationPartyStatus::updateTransition(StatusTransition& transition, bool desired, float elapsed)
    {
        if (transition.desired != desired)
        {
            transition.desired = desired;
            transition.visible = true;
            transition.removing = !desired;
            transition.elapsed = 0.0F;
        }
        if (!transition.visible)
        {
            return;
        }
        transition.elapsed = std::min(0.5F, transition.elapsed + elapsed);
        if (transition.removing && transition.elapsed >= 0.5F)
        {
            transition.visible = false;
        }
    }

    float ExplorationPartyStatus::transitionScale(const StatusTransition& transition)
    {
        const auto progress = std::clamp(transition.elapsed * 2.0F, 0.0F, 1.0F);
        if (transition.removing)
        {
            return -2.4F * progress * progress + 1.4F * progress + 1.0F;
        }
        const auto inverse = 1.0F - progress;
        return inverse * inverse * inverse * 1.5F + 3.0F * progress * inverse * inverse * 0.64F + 3.0F * progress * progress * inverse * 0.635F +
               progress * progress * progress;
    }
} // namespace soma::presentation
