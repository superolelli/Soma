#include "presentation/sfml/BattleScreenRenderer.hpp"

#include "presentation/sfml/BattleHud.hpp"
#include "presentation/sfml/NinePatch.hpp"
#include "presentation/sfml/StatusGlossary.hpp"
#include "presentation/text/AbilityDescription.hpp"
#include "simulation/battle/AbilityTargets.hpp"

#include <algorithm>
#include <array>
#include <iterator>
#include <ranges>
#include <sstream>
#include <unordered_map>

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
        return std::ranges::any_of(action_->events, defeatsCombatant) ||
               std::ranges::any_of(action_->deferredEvents, defeatsCombatant);
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
        auto label = makeText(interfaceFont_,
                              std::to_string(turnNumber),
                              static_cast<unsigned int>(content_.layout.turnOrderNumberCharacterSize),
                              {});
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
            const auto primaryBounds = drawTooltip(
                window_, interfaceFont_, tooltip, {button.bounds.position.x, button.bounds.position.y - 100.0F}, false, 500.0F);
            const auto statusDetails = abilityStatusDetails(ability);
            if (!statusDetails.empty())
            {
                const auto view = window_.getView();
                const auto viewOrigin = view.getCenter() - view.getSize() / 2.0F;
                static_cast<void>(drawStatusGlossary(window_,
                                                     interfaceFont_,
                                                     statusDetails,
                                                     [this](const auto& status) { return statusTexture(textures_, status); },
                                                     {viewOrigin.x + view.getSize().x - 20.0F, viewOrigin.y + 60.0F},
                                                     primaryBounds));
            }
            return;
        }
    }

    void BattleScreenRenderer::drawAnnouncement()
    {
        if (!action_.has_value() || action_->stage != ActionStage::announcement)
        {
            return;
        }
        const auto& layout = content_.layout;
        const auto& ability = content_.abilities.at(action_->command.ability);
        const auto bannerPosition = sf::Vector2f{static_cast<float>(layout.announcementX), static_cast<float>(layout.announcementY)};
        window_.draw(makeSprite(textures_.announcement, bannerPosition));
        auto label = makeText(interfaceFont_, ability.name, 25U, {});
        const auto labelBounds = label.getLocalBounds();
        const auto bannerSize =
            sf::Vector2f{static_cast<float>(textures_.announcement.getSize().x), static_cast<float>(textures_.announcement.getSize().y)};
        label.setPosition({bannerPosition.x + (bannerSize.x - labelBounds.size.x) / 2.0F - labelBounds.position.x,
                           bannerPosition.y + (bannerSize.y - labelBounds.size.y) / 2.0F - labelBounds.position.y});
        window_.draw(label);
        for (const auto& target : action_->affectedCombatants)
        {
            drawTargetMarker(target);
        }
    }

    void BattleScreenRenderer::drawItemInterruption()
    {
        if (!action_.has_value() || action_->stage != ActionStage::itemInterruption)
        {
            return;
        }
        const auto activation = std::ranges::find_if(action_->events, [](const auto& event) {
            return std::holds_alternative<gameplay::ItemActivated>(event);
        });
        if (activation == action_->events.end())
        {
            return;
        }
        const auto& item = std::get<gameplay::ItemActivated>(*activation);
        const auto presentation = content_.itemActivations.find(item.item);
        const auto speaker = views_.find(item.carrier);
        if (presentation == content_.itemActivations.end() || speaker == views_.end())
        {
            return;
        }

        const auto& layout = content_.layout;
        sf::Text text{interfaceFont_, "", static_cast<unsigned int>(layout.itemSpeechCharacterSize)};
        text.setFillColor(sf::Color::Black);
        std::istringstream words{presentation->second.speech};
        std::string word;
        std::string line;
        std::string wrapped;
        while (words >> word)
        {
            const auto candidate = line.empty() ? word : line + " " + word;
            text.setString(sf::String::fromUtf8(candidate.begin(), candidate.end()));
            if (!line.empty() && text.getLocalBounds().size.x > static_cast<float>(layout.itemSpeechMaximumWidth))
            {
                wrapped += (wrapped.empty() ? "" : "\n") + line;
                line = word;
            }
            else
            {
                line = candidate;
            }
        }
        wrapped += (wrapped.empty() ? "" : "\n") + line;
        text.setString(sf::String::fromUtf8(wrapped.begin(), wrapped.end()));

        const auto textBounds = text.getLocalBounds();
        NinePatch background{textures_.itemSpeechBubble};
        background.setSize({textBounds.size.x + static_cast<float>(layout.itemSpeechPaddingX),
                            textBounds.size.y + static_cast<float>(layout.itemSpeechPaddingY)});
        const auto bubbleBounds = background.globalBounds();
        const auto& speakerBounds = speaker->second.bounds;
        const sf::Vector2f position{speakerBounds.position.x + speakerBounds.size.x + static_cast<float>(layout.itemSpeechSpeakerOffsetX),
                                   speakerBounds.position.y - bubbleBounds.size.y + static_cast<float>(layout.itemSpeechSpeakerOffsetY)};
        background.setPosition(position);
        text.setPosition({position.x + (bubbleBounds.size.x - textBounds.size.x) / 2.0F - textBounds.position.x,
                          position.y + (bubbleBounds.size.y - textBounds.size.y) / 2.0F - textBounds.position.y +
                              static_cast<float>(layout.itemSpeechTextVerticalOffset)});
        window_.draw(background);
        window_.draw(text);
    }

    void BattleScreenRenderer::drawInformationPanel()
    {
        auto informationCombatant = interaction_.inspectedCombatant();
        if (!informationCombatant.has_value() && !battle_.finished())
        {
            informationCombatant = battle_.currentActor();
        }
        if (!informationCombatant.has_value())
        {
            return;
        }
        const auto* state = battle_.find(*informationCombatant);
        if (state == nullptr)
        {
            return;
        }

        const auto displayed = displayedState(*state);
        drawCombatantInformation(
            window_, textures_, interfaceFont_, displayed, content_.combatants.at(state->definition), content_.layout, views_.at(state->id));
        const auto& selectedAbility = interaction_.selectedAbility();
        if (!selectedAbility.has_value() || battle_.finished())
        {
            return;
        }
        const auto* actor = battle_.find(battle_.currentActor());
        const auto validTargets = battle_.validPrimaryTargets(*selectedAbility);
        if (actor != nullptr && std::ranges::find(validTargets, state->id) != validTargets.end())
        {
            drawExpectedAttack(window_, interfaceFont_, battle_, *actor, *state, content_.abilities.at(*selectedAbility), content_.layout);
        }
    }

    void BattleScreenRenderer::drawFloatingTexts()
    {
        std::unordered_map<gameplay::EntityId, int> rowsByCombatant;
        for (const auto& floating : floatingTexts_)
        {
            const auto& font = floating.font == FloatingTextFont::interface ? interfaceFont_ : notificationFont_;
            auto notification = makeText(font, floating.value, floating.size, {0.0F, 0.0F}, floating.color);
            notification.setOutlineColor(floating.outlineColor);
            notification.setOutlineThickness(floating.outlineThickness);
            const auto textBounds = notification.getLocalBounds();

            const auto found = views_.find(floating.subject);
            if (found == views_.end())
            {
                continue;
            }
            const auto& bounds = found->second.bounds;
            const auto row = rowsByCombatant[floating.subject]++;
            notification.setPosition({bounds.position.x + bounds.size.x / 2.0F - textBounds.size.x / 2.0F - textBounds.position.x,
                                      bounds.position.y - floating.elapsed * 120.0F - row * 45.0F});
            window_.draw(notification);
        }
    }

    void BattleScreenRenderer::drawBattleResult()
    {
        if (!battle_.finished() || action_.has_value())
        {
            return;
        }
        const auto message = battle_.winner() == gameplay::Team::players ? "Sieg!" : "Niederlage";
        window_.draw(makeText(
            interfaceFont_, message, 70U, {static_cast<float>(content_.layout.battleResultX), static_cast<float>(content_.layout.battleResultY)}));
    }

    void BattleScreenRenderer::drawTargetMarker(const gameplay::EntityId& target)
    {
        const auto& view = views_.at(target);
        const auto x = view.bounds.position.x + view.bounds.size.x / 2.0F - textures_.targetMarker.getSize().x / 2.0F;
        window_.draw(makeSprite(textures_.targetMarker, {x, view.bounds.position.y + view.bounds.size.y + 49.0F}));
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
