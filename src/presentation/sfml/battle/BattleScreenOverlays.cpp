#include "presentation/sfml/battle/BattleHud.hpp"
#include "presentation/sfml/battle/BattleScreenRenderer.hpp"
#include "presentation/sfml/common/StatusGlossary.hpp"
#include "presentation/text/AbilityDescription.hpp"
#include "simulation/battle/AbilityTargets.hpp"

#include <algorithm>
#include <iterator>
#include <ranges>

namespace
{
    sf::Text makeText(const sf::Font& font, const std::string& value, unsigned int size, sf::Vector2f position, sf::Color color = sf::Color::White)
    {
        sf::Text result{font, sf::String::fromUtf8(value.begin(), value.end()), size};
        result.setPosition(position);
        result.setFillColor(color);
        result.setOutlineColor(sf::Color::Black);
        result.setOutlineThickness(3.0F);
        return result;
    }

    sf::Sprite makeSprite(const sf::Texture& texture, sf::Vector2f position)
    {
        sf::Sprite result{texture};
        result.setPosition(position);
        return result;
    }

    const sf::Texture* statusTexture(const soma::presentation::BattleTextures& textures, const soma::gameplay::StatusId& status)
    {
        const auto& id = status.value();
        if (id == "status.confused")
        {
            return &textures.confused;
        }
        if (id == "status.decay")
        {
            return &textures.decay;
        }
        if (id == "status.marked")
        {
            return &textures.marked;
        }
        if (id == "status.sleeping")
        {
            return &textures.sleeping;
        }
        if (id == "status.miss")
        {
            return &textures.missed;
        }
        if (id == "status.dynamite")
        {
            return &textures.dynamite;
        }
        if (id == "status.bounty")
        {
            return &textures.bounty;
        }
        if (id == "status.riposte")
        {
            return &textures.riposte;
        }
        if (id == "status.rage")
        {
            return &textures.rage;
        }
        if (id == "status.hacked")
        {
            return &textures.hacked;
        }
        if (id == "status.transfusion")
        {
            return &textures.transfusion;
        }
        if (id == "status.panic")
        {
            return &textures.panic;
        }
        return nullptr;
    }
} // namespace

namespace soma::presentation
{
    std::optional<std::pair<std::string, sf::Vector2f>> BattleScreenRenderer::drawCombatantOverlays(sf::Vector2f mousePosition,
                                                                                                    bool skipActionParticipants)
    {
        std::optional<std::pair<std::string, sf::Vector2f>> tooltip;
        auto initiativeOrder = battle_.remainingInitiativeOrder();
        if (action_.has_value())
        {
            const auto actor = battle_.find(action_->command.actor);
            if (actor != nullptr && actor->alive() && std::ranges::find(initiativeOrder, actor->id) == initiativeOrder.end())
            {
                initiativeOrder.insert(initiativeOrder.begin(), actor->id);
            }
        }
        for (const auto& [id, view] : views_)
        {
            if (skipActionParticipants && participatesInAction(id))
            {
                continue;
            }
            const auto* state = battle_.find(id);
            if (state == nullptr)
            {
                continue;
            }
            const auto visibleState = displayedState(*state);
            const auto waitingForDeathAnimation = action_.has_value() && action_->stage == ActionStage::result && awaitsDefeatAnimation(id);
            if (!visibleState.alive() && !waitingForDeathAnimation)
            {
                continue;
            }
            drawTurnOrderNumber(id, view, initiativeOrder);
            drawHealthBar(window_, textures_, view);
            drawFatigueBar(window_, textures_, visibleState, view);
            std::vector<gameplay::StatusId> newlyGainedStatuses;
            if (action_.has_value() && action_->stage == ActionStage::result)
            {
                for (const auto& event : action_->events)
                {
                    const auto* gained = std::get_if<gameplay::StatusGained>(&event);
                    if (gained != nullptr && gained->combatant == id)
                    {
                        newlyGainedStatuses.push_back(gained->status);
                    }
                }
            }
            drawStatusIcons(window_, textures_, visibleState, view, newlyGainedStatuses, action_.has_value() ? action_->elapsed : 1.0F);

            const auto healthBarX = view.bounds.position.x + view.bounds.size.x / 2.0F - textures_.healthBarFrame.getSize().x / 2.0F;
            const auto healthBarY = view.bounds.position.y + view.bounds.size.y + 30.0F;
            auto statusPosition = sf::Vector2f{healthBarX, healthBarY + textures_.healthBarFrame.getSize().y + 2.0F};
            if (visibleState.team == gameplay::Team::players)
            {
                statusPosition.y += static_cast<float>(textures_.fatigueBarFrame.getSize().y) + 2.0F;
            }
            for (const auto& status : activeStatusIcons(textures_, visibleState))
            {
                const sf::FloatRect statusBounds{statusPosition,
                                                 {static_cast<float>(status.texture->getSize().x), static_cast<float>(status.texture->getSize().y)}};
                if (statusBounds.contains(mousePosition))
                {
                    tooltip = std::pair{status.tooltip, sf::Vector2f{healthBarX, healthBarY - 4.0F}};
                }
                statusPosition.x += 20.0F;
            }

            if (!battle_.finished())
            {
                const auto& displayedActor = action_.has_value() && action_->wakingCombatant.has_value() ? *action_->wakingCombatant
                                             : action_.has_value()                                       ? action_->command.actor
                                                                                                         : battle_.currentActor();
                if (displayedActor == id)
                {
                    const auto markerX = view.bounds.position.x + view.bounds.size.x / 2.0F - textures_.turnMarker.getSize().x / 2.0F;
                    window_.draw(makeSprite(textures_.turnMarker, {markerX, view.bounds.position.y + view.bounds.size.y + 49.0F}));
                }
            }
        }
        return tooltip;
    }

    void BattleScreenRenderer::drawTurnOrderNumber(const gameplay::EntityId& combatant,
                                                   const CombatantView& view,
                                                   const std::vector<gameplay::EntityId>& initiativeOrder)
    {
        const auto position = std::ranges::find(initiativeOrder, combatant);
        if (position == initiativeOrder.end())
        {
            return;
        }

        const auto turnNumber = std::distance(initiativeOrder.begin(), position) + 1;
        auto label =
            makeText(interfaceFont_, std::to_string(turnNumber), static_cast<unsigned int>(content_.layout.turnOrderNumberCharacterSize), {});
        const auto bounds = label.getLocalBounds();
        label.setOrigin({bounds.position.x + bounds.size.x / 2.0F, bounds.position.y});
        label.setPosition({view.bounds.position.x + view.bounds.size.x / 2.0F,
                           view.bounds.position.y + view.bounds.size.y + static_cast<float>(content_.layout.turnOrderNumberOffsetY)});
        window_.draw(label);
    }

    void BattleScreenRenderer::drawPlayerControls(sf::Vector2f mousePosition)
    {
        if (action_.has_value() || battle_.finished() || battle_.find(battle_.currentActor())->team != gameplay::Team::players)
        {
            return;
        }

        const auto& layout = content_.layout;
        window_.draw(makeSprite(textures_.abilityPanel, {static_cast<float>(layout.abilityPanelX), static_cast<float>(layout.abilityPanelY)}));
        sf::Sprite skipTurnSprite{textures_.skipTurn};
        const auto skipPressed = skipTurnBounds_.contains(mousePosition) && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
        skipTurnSprite.setTextureRect({{static_cast<int>(skipPressed ? skipTurnFrameWidth_ : 0U), 0},
                                       {static_cast<int>(skipTurnFrameWidth_), static_cast<int>(textures_.skipTurn.getSize().y)}});
        skipTurnSprite.setPosition(skipTurnPosition_);
        window_.draw(skipTurnSprite);

        const auto& selectedAbility = interaction_.selectedAbility();
        const auto& abilityButtons = interaction_.abilityButtons();
        if (selectedAbility.has_value())
        {
            const auto selected = std::ranges::find(abilityButtons, *selectedAbility, &AbilityButton::ability);
            if (selected != abilityButtons.end())
            {
                window_.draw(makeSprite(textures_.currentAbilityFrame, {selected->bounds.position.x - 10.0F, selected->bounds.position.y - 10.0F}));
            }
        }
        for (const auto& button : abilityButtons)
        {
            window_.draw(button.sprite);
        }
        drawHoveredTargetMarkers(mousePosition);
    }

    void BattleScreenRenderer::drawHoveredTargetMarkers(sf::Vector2f mousePosition)
    {
        const auto& selectedAbility = interaction_.selectedAbility();
        if (!selectedAbility.has_value())
        {
            return;
        }
        const auto validTargets = battle_.validPrimaryTargets(*selectedAbility);
        const auto hoveredTarget = std::ranges::find_if(validTargets, [this, mousePosition](const auto& target) {
            return views_.at(target).bounds.contains(mousePosition);
        });
        if (hoveredTarget == validTargets.end())
        {
            return;
        }
        for (const auto& target : targetsMarkedBy(*selectedAbility, *hoveredTarget))
        {
            drawTargetMarker(target);
        }
    }

    std::vector<gameplay::EntityId> BattleScreenRenderer::targetsMarkedBy(const gameplay::AbilityId& abilityId,
                                                                          const gameplay::EntityId& primaryTarget) const
    {
        const auto& targeting = content_.abilities.at(abilityId).targeting;
        const auto* actor = battle_.find(battle_.currentActor());
        const auto* primary = battle_.find(primaryTarget);
        return actor != nullptr && primary != nullptr ? simulation::abilityTargets(targeting, *actor, *primary, battle_.combatants())
                                                      : std::vector<gameplay::EntityId>{primaryTarget};
    }

    void BattleScreenRenderer::drawAbilityTooltip(sf::Vector2f mousePosition)
    {
        if (action_.has_value() || battle_.finished() || battle_.find(battle_.currentActor())->team != gameplay::Team::players)
        {
            return;
        }
        if (skipTurnBounds_.contains(mousePosition))
        {
            static_cast<void>(drawTooltip(window_,
                                          interfaceFont_,
                                          "*#d6b45c Zug aussetzen*\n#dddddd Beendet den Zug, ohne eine Fähigkeit einzusetzen.",
                                          {skipTurnBounds_.position.x, skipTurnBounds_.position.y - 10.0F},
                                          true,
                                          330.0F));
            return;
        }
        for (const auto& button : interaction_.abilityButtons())
        {
            if (!button.bounds.contains(mousePosition))
            {
                continue;
            }
            const auto& ability = content_.abilities.at(button.ability);
            const auto tooltip = abilityDescription(ability);
            const auto primaryBounds =
                drawTooltip(window_, interfaceFont_, tooltip, {button.bounds.position.x, button.bounds.position.y - 100.0F}, false, 500.0F);
            const auto statusDetails = abilityStatusDetails(ability);
            if (!statusDetails.empty())
            {
                const auto view = window_.getView();
                const auto viewOrigin = view.getCenter() - view.getSize() / 2.0F;
                static_cast<void>(drawStatusGlossary(
                    window_,
                    interfaceFont_,
                    statusDetails,
                    [this](const auto& status) {
                        return statusTexture(textures_, status);
                    },
                    {viewOrigin.x + view.getSize().x - 20.0F, viewOrigin.y + 60.0F},
                    primaryBounds));
            }
            return;
        }
    }

} // namespace soma::presentation
