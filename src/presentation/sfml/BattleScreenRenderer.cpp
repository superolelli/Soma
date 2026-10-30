#include "presentation/sfml/BattleScreenRenderer.hpp"

#include "presentation/sfml/BattleHud.hpp"

#include <algorithm>
#include <ranges>

namespace
{
    void restoreDeferredStatus(soma::simulation::CombatantState& displayed,
                               const soma::simulation::CombatantState& previous,
                               const soma::gameplay::StatusId& status)
    {
        const auto& id = status.value();
        if (id == "status.buffed")
        {
            displayed.buffs = previous.buffs;
        }
        else if (id == "status.debuffed")
        {
            displayed.debuffs = previous.debuffs;
        }
        else if (id == "status.decay")
        {
            displayed.decay = previous.decay;
        }
        else if (id == "status.transfusion")
        {
            displayed.transfusions = previous.transfusions;
        }
        else if (id == "status.confused")
        {
            displayed.confusedTurns = previous.confusedTurns;
        }
        else if (id == "status.marked")
        {
            displayed.markedTurns = previous.markedTurns;
        }
        else if (id == "status.bounty")
        {
            displayed.bountyTurns = previous.bountyTurns;
        }
        else if (id == "status.miss")
        {
            displayed.misses = previous.misses;
        }
        else if (id == "status.sleeping")
        {
            displayed.sleeping = previous.sleeping;
        }
        else if (id == "status.dynamite")
        {
            displayed.dynamite = previous.dynamite;
        }
        else if (id == "status.riposte")
        {
            displayed.riposteTurns = previous.riposteTurns;
        }
        else if (id == "status.rage")
        {
            displayed.rageTurns = previous.rageTurns;
        }
        else if (id == "status.hacked")
        {
            displayed.hackedTurns = previous.hackedTurns;
        }
        else if (id == "status.panic")
        {
            displayed.panicStacks = previous.panicStacks;
        }
    }
} // namespace

namespace soma::presentation
{
    BattleScreenRenderer::BattleScreenRenderer(sf::RenderWindow& window,
                                               const content::BattleContent& content,
                                               const simulation::BattleSimulation& battle,
                                               CombatantViews& views,
                                               IAnimationPlayer& effect,
                                               const BattleTextures& textures,
                                               const sf::Font& interfaceFont,
                                               const sf::Font& notificationFont,
                                               const std::vector<sf::Sprite>& backgrounds,
                                               ParticleSystem& particles,
                                               BattleSceneBlur& sceneBlur,
                                               const std::optional<PresentedAction>& action,
                                               const std::vector<FloatingText>& floatingTexts,
                                               const BattleInteractionController& interaction,
                                               const ResourceCounter& resources,
                                               sf::Vector2f skipTurnPosition,
                                               sf::FloatRect skipTurnBounds,
                                               unsigned int skipTurnFrameWidth)
        : window_(window)
        , content_(content)
        , battle_(battle)
        , views_(views)
        , effect_(effect)
        , textures_(textures)
        , interfaceFont_(interfaceFont)
        , notificationFont_(notificationFont)
        , backgrounds_(backgrounds)
        , particles_(particles)
        , sceneBlur_(sceneBlur)
        , action_(action)
        , floatingTexts_(floatingTexts)
        , interaction_(interaction)
        , resources_(resources)
        , skipTurnPosition_(skipTurnPosition)
        , skipTurnBounds_(skipTurnBounds)
        , skipTurnFrameWidth_(skipTurnFrameWidth)
    {
    }

    void BattleScreenRenderer::render(sf::Vector2f mousePosition)
    {
        draw(mousePosition);
        window_.display();
    }

    void BattleScreenRenderer::draw(sf::Vector2f mousePosition)
    {
        window_.clear(sf::Color::Black);
        for (const auto& background : backgrounds_)
        {
            window_.draw(background);
        }
        particles_.draw(window_, content::ParticleLayer::behindCharacters);
        drawRestingCombatants();
        std::optional<std::pair<std::string, sf::Vector2f>> statusTooltip;
        if (isExecutingAbility())
        {
            statusTooltip = drawCombatantOverlays(mousePosition, true);
            drawExecutingAction();
        }
        else
        {
            statusTooltip = drawCombatantOverlays(mousePosition);
        }
        particles_.draw(window_, content::ParticleLayer::foreground);
        drawPlayerControls(mousePosition);
        drawAnnouncement();
        drawInformationPanel();
        resources_.render(window_);
        drawFloatingTexts();
        drawItemInterruption();
        drawBattleResult();
        drawAbilityTooltip(mousePosition);
        if (statusTooltip.has_value())
        {
            static_cast<void>(drawTooltip(window_, interfaceFont_, statusTooltip->first, statusTooltip->second, true));
        }
        drawInformationStatTooltip(mousePosition);
    }

    bool BattleScreenRenderer::isExecutingAbility() const
    {
        return action_.has_value() && action_->stage == ActionStage::execution;
    }

    bool BattleScreenRenderer::participatesInAction(const gameplay::EntityId& id) const
    {
        if (!isExecutingAbility())
        {
            return false;
        }
        return id == action_->command.actor || std::ranges::find(action_->affectedCombatants, id) != action_->affectedCombatants.end();
    }

    bool BattleScreenRenderer::awaitsDefeatAnimation(const gameplay::EntityId& id) const
    {
        if (!action_.has_value() || (action_->stage != ActionStage::result && action_->stage != ActionStage::defeat))
        {
            return false;
        }
        const auto defeatsCombatant = [&id](const auto& event) {
            const auto* defeated = std::get_if<gameplay::CombatantDefeated>(&event);
            return defeated != nullptr && defeated->combatant == id;
        };
        return std::ranges::any_of(action_->events, defeatsCombatant) || std::ranges::any_of(action_->deferredEvents, defeatsCombatant);
    }

    void BattleScreenRenderer::drawRestingCombatants()
    {
        for (const auto& state : battle_.combatants())
        {
            auto& view = views_.at(state.id);
            if (!participatesInAction(state.id) && (view.displayedHealth > 0.0F || awaitsDefeatAnimation(state.id)))
            {
                drawCombatantShadow(window_, view);
                view.animation->render();
            }
        }
    }

    void BattleScreenRenderer::drawExecutingAction()
    {
        if (!isExecutingAbility())
        {
            return;
        }
        if (sceneBlur_.available())
        {
            sceneBlur_.apply(window_);
        }
        for (const auto& state : battle_.combatants())
        {
            if (participatesInAction(state.id))
            {
                views_.at(state.id).animation->render();
            }
        }
        for (const auto position : action_->effectPositions)
        {
            effect_.setPosition(position.x, position.y);
            effect_.render();
        }
    }

    simulation::CombatantState BattleScreenRenderer::displayedState(const simulation::CombatantState& state) const
    {
        auto displayed = state;
        if (!action_.has_value())
        {
            displayed.attributes = battle_.effectiveAttributes(displayed);
            return displayed;
        }
        const auto previous = std::ranges::find(action_->combatantsBeforeAction, state.id, &simulation::CombatantState::id);
        if (action_->stage == ActionStage::announcement || action_->stage == ActionStage::execution)
        {
            if (previous != action_->combatantsBeforeAction.end())
            {
                displayed = *previous;
            }
        }
        else if (action_->stage == ActionStage::itemInterruption)
        {
            displayed.health = views_.at(state.id).targetHealth;
        }
        if (previous != action_->combatantsBeforeAction.end())
        {
            for (const auto& event : action_->deferredEvents)
            {
                if (const auto* gained = std::get_if<gameplay::StatusGained>(&event); gained != nullptr && gained->combatant == state.id)
                {
                    restoreDeferredStatus(displayed, *previous, gained->status);
                }
                if (const auto* expired = std::get_if<gameplay::StatusExpired>(&event); expired != nullptr && expired->combatant == state.id)
                {
                    restoreDeferredStatus(displayed, *previous, expired->status);
                }
            }
        }
        if (action_->stage == ActionStage::result)
        {
            const auto rageGained = std::ranges::any_of(action_->events, [&state](const auto& event) {
                const auto* gained = std::get_if<gameplay::StatusGained>(&event);
                return gained != nullptr && gained->combatant == state.id && gained->status == gameplay::StatusId{"status.rage"};
            });
            if (rageGained)
            {
                displayed.rageTurns = std::max(1, displayed.rageTurns);
            }
        }
        displayed.attributes = battle_.effectiveAttributes(displayed);
        return displayed;
    }
} // namespace soma::presentation
