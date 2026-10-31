#include "presentation/sfml/battle/BattleActionPresentation.hpp"

#include "simulation/battle/AbilityTargets.hpp"

#include "presentation/battle/BattleEventPresentation.hpp"
#include "presentation/sfml/common/SpriterAnimationPlayer.hpp"

#include <algorithm>
#include <array>
#include <memory>
#include <optional>
#include <ranges>
#include <string_view>

namespace
{
    std::optional<std::string_view> statusNotification(const soma::gameplay::StatusId& status)
    {
        using Entry = std::pair<std::string_view, std::string_view>;
        static constexpr std::array entries{Entry{"status.confused", "Verwirrt!"},
                                            Entry{"status.confused_action", "Verwirrt!"},
                                            Entry{"status.sleeping", "Eingeschlafen!"},
                                            Entry{"status.decay", "Verfall!"},
                                            Entry{"status.marked", "Markiert!"},
                                            Entry{"status.bounty", "Kopfgeld!"},
                                            Entry{"status.miss", "Fehlschuss!"},
                                            Entry{"status.dynamite", "Dynamit!"},
                                            Entry{"status.riposte", "Gegenschlag!"},
                                            Entry{"status.rage", "Raserei!"},
                                            Entry{"status.hacked", "Gehackt!"},
                                            Entry{"status.transfusion", "Transfusion!"},
                                            Entry{"status.panic", "Panik!"},
                                            Entry{"status.buffed", "Verstärkt!"},
                                            Entry{"status.debuffed", "Geschwächt!"}};
        const auto found = std::ranges::find(entries, status.value(), &Entry::first);
        return found == entries.end() ? std::nullopt : std::optional{found->second};
    }

    void addInterfaceNotification(std::vector<soma::presentation::FloatingText>& floating,
                                  const soma::gameplay::EntityId& combatant,
                                  std::string value)
    {
        floating.push_back({combatant,
                            std::move(value),
                            sf::Color::White,
                            0.0F,
                            30U,
                            soma::presentation::FloatingTextAnchor::combatant,
                            sf::Color::Black,
                            3.0F,
                            soma::presentation::FloatingTextFont::interface});
    }

    sf::Vector2f abilityEffectPosition(soma::presentation::CombatantView& target,
                                       const soma::presentation::AnimationPoint& attachment)
    {
        const auto fallback = soma::presentation::AnimationPoint{target.bounds.position.x + target.bounds.size.x / 2.0F,
                                                                  target.bounds.position.y + target.bounds.size.y / 2.0F};
        const auto impact = target.animation->point("ability_effect_point").value_or(fallback);
        return {impact.x - attachment.x, impact.y - attachment.y};
    }

    std::vector<soma::gameplay::EntityId> visualEffectTargets(const soma::presentation::PresentedAction& action,
                                                               const soma::content::AbilityDefinition& ability)
    {
        const auto actor = std::ranges::find(action.combatantsBeforeAction, action.command.actor, &soma::simulation::CombatantState::id);
        const auto primary =
            std::ranges::find(action.combatantsBeforeAction, action.command.primaryTarget, &soma::simulation::CombatantState::id);
        if (actor == action.combatantsBeforeAction.end() || primary == action.combatantsBeforeAction.end() || actor->confusedThisTurn ||
            ability.targeting.scope == soma::content::TargetScope::randomSingle)
        {
            return {action.command.primaryTarget};
        }
        return soma::simulation::abilityTargets(ability.targeting, *actor, *primary, action.combatantsBeforeAction);
    }

    void prepareAbilityEffect(soma::presentation::PresentedAction& action,
                              soma::presentation::CombatantViews& views,
                              soma::presentation::IAnimationPlayer& effect,
                              const soma::content::AbilityDefinition& ability,
                              const soma::simulation::CombatantState& actor,
                              const soma::simulation::CombatantState& primaryTarget)
    {
        const auto friendly = (primaryTarget.team == actor.team) != actor.confusedThisTurn;
        effect.play(friendly ? ability.friendlyEffectAnimation : ability.hostileEffectAnimation, 0.9F);
        effect.setPosition(0.0F, 0.0F);
        const auto attachment = effect.point("attaching_point").value_or(soma::presentation::AnimationPoint{});

        action.effectPositions.clear();
        const auto targets = visualEffectTargets(action, ability);
        for (const auto& target : targets)
        {
            action.effectPositions.push_back(abilityEffectPosition(views.at(target), attachment));
        }
        if (action.effectPositions.empty())
        {
            action.effectPositions.push_back(abilityEffectPosition(views.at(action.command.primaryTarget), attachment));
        }
        effect.setPosition(action.effectPositions.front().x, action.effectPositions.front().y);
    }
} // namespace

namespace soma::presentation
{
    void updateCombatantBounds(CombatantView& view)
    {
        const auto bounds = view.animation->bounds();
        view.bounds = {{bounds.left, bounds.top}, {bounds.width, bounds.height}};
    }

    void restoreCombatants(CombatantViews& views)
    {
        for (auto& [id, view] : views)
        {
            static_cast<void>(id);
            const auto scale = view.normalScale + view.scaleAdjustment;
            view.animation->setScale(scale, scale);
            view.animation->setPosition(view.normalPosition.x, view.normalPosition.y);
            const auto animation = view.animation->currentAnimation();
            if (view.displayedHealth > 0 && animation != "idle" && animation != "sleeping")
            {
                view.animation->play("idle", 0.7F);
            }
            updateCombatantBounds(view);
        }
    }

    void applyPersistentAnimations(CombatantViews& views, const simulation::BattleSimulation& battle)
    {
        for (const auto& state : battle.combatants())
        {
            auto& animation = *views.at(state.id).animation;
            if (state.alive() && state.sleeping && animation.currentAnimation() != "sleeping")
            {
                animation.play("sleeping", 0.7F);
            }
            else if (state.alive() && !state.sleeping && animation.currentAnimation() == "sleeping")
            {
                animation.play("idle", 0.7F);
            }
        }
    }

    CombatantViews createCombatantViews(sf::RenderWindow& window, const content::BattleContent& content, const simulation::BattleSimulation& battle)
    {
        CombatantViews views;
        const auto& layout = content.layout;
        for (const auto& state : battle.combatants())
        {
            const auto& presentation = content.presentation.at(state.definition);
            auto animation = std::make_unique<SpriterAnimationPlayer>(window, presentation.model, presentation.entity);
            const auto scale = static_cast<float>(layout.normalScale) + static_cast<float>(presentation.scaleAdjustment);
            animation->setScale(scale, scale);
            animation->setPosition(0.0F, static_cast<float>(layout.groundY));
            animation->play("idle", 0.7F);
            views.emplace(state.id,
                          CombatantView{std::move(animation),
                                        state.definition,
                                        {},
                                        {},
                                        static_cast<float>(layout.normalScale),
                                        static_cast<float>(presentation.scaleAdjustment),
                                        static_cast<float>(state.health),
                                        static_cast<float>(state.health),
                                        state.health,
                                        0.7F,
                                        state.attributes.maximumHealth,
                                        static_cast<float>(state.fatigue),
                                        static_cast<float>(state.fatigue),
                                        state.fatigue,
                                        0.7F});
        }

        positionCombatantViews(views, battle);
        return views;
    }

    void positionEnemyViews(CombatantViews& views, const simulation::BattleSimulation& battle)
    {
        const auto& layout = battle.content().layout;
        auto enemyX = static_cast<float>(layout.referenceWidth / 2 + layout.enemyStartFromViewCenter);
        for (int slot = 4; slot < 8; ++slot)
        {
            const auto found = std::ranges::find_if(battle.combatants(), [slot](const auto& state) {
                return state.slot == slot && state.alive();
            });
            if (found == battle.combatants().end())
            {
                continue;
            }
            auto& view = views.at(found->id);
            view.animation->setPosition(0.0F, static_cast<float>(layout.groundY));
            updateCombatantBounds(view);
            view.normalPosition = {enemyX - view.bounds.position.x, static_cast<float>(layout.groundY)};
            view.animation->setPosition(view.normalPosition.x, view.normalPosition.y);
            updateCombatantBounds(view);
            enemyX += view.bounds.size.x + static_cast<float>(layout.enemySpacing);
        }
    }

    void positionCombatantViews(CombatantViews& views, const simulation::BattleSimulation& battle)
    {
        const auto& layout = battle.content().layout;
        for (const auto& state : battle.combatants())
        {
            if (!state.alive() || state.team != gameplay::Team::players)
            {
                continue;
            }
            auto& view = views.at(state.id);
            view.normalPosition = {static_cast<float>(layout.playerStartX + layout.playerSpacing * state.slot),
                                   static_cast<float>(layout.groundY)};
            view.animation->setPosition(view.normalPosition.x, view.normalPosition.y);
            updateCombatantBounds(view);
        }
        positionEnemyViews(views, battle);
    }

    std::vector<AbilityButton> createAbilityButtons(const content::BattleContent& content, const simulation::CombatantState& actor)
    {
        std::vector<AbilityButton> result;
        const auto& combatant = content.combatants.at(actor.definition);
        const auto& presentation = content.presentation.at(actor.definition);
        auto x = static_cast<float>(content.layout.abilityButtonStartX);
        for (std::size_t index = 0; index < combatant.abilities.size(); ++index)
        {
            auto texture = std::make_shared<sf::Texture>(presentation.abilityIcons.at(combatant.abilities.at(index)));
            result.emplace_back(
                std::move(texture), sf::Vector2f{x, static_cast<float>(content.layout.abilityButtonY)}, combatant.abilities.at(index));
            x += static_cast<float>(content.layout.abilityButtonSpacing);
        }
        return result;
    }

    void moveToAbilityStage(PresentedAction& action, CombatantViews& views, IAnimationPlayer& effect, const simulation::BattleSimulation& battle)
    {
        auto& actor = views.at(action.command.actor);
        const auto* actorState = battle.find(action.command.actor);
        const auto& ability = battle.content().abilities.at(action.command.ability);
        const auto& layout = battle.content().layout;
        const auto positionFactor = static_cast<float>(1.0 + layout.abilityScale - layout.normalScale);

        std::ranges::sort(action.affectedCombatants, [&battle](const auto& left, const auto& right) {
            return battle.find(left)->slot < battle.find(right)->slot;
        });

        std::unordered_map<gameplay::EntityId, float> normalWidths;
        for (const auto& target : action.affectedCombatants)
        {
            normalWidths.emplace(target, views.at(target).bounds.size.x);
        }
        normalWidths.insert_or_assign(action.command.actor, actor.bounds.size.x);

        const auto selfOnly = action.affectedCombatants.size() == 1 && action.affectedCombatants.front() == action.command.actor;
        const auto* primaryState = battle.find(action.command.primaryTarget);
        const auto centersOwnGroup = ability.targeting.scope == content::TargetScope::team &&
                                     simulation::targetsActor(ability.targeting, *actorState, *primaryState);

        auto actorX = actor.normalPosition.x;
        if (selfOnly)
        {
            actorX = static_cast<float>(layout.referenceWidth) / 2.0F - normalWidths.at(actorState->id) * positionFactor / 2.0F;
        }
        else if (ability.targeting.relation != content::TargetRelation::everyoneElse && !centersOwnGroup)
        {
            const auto actorCenter = actorState->team == gameplay::Team::players ? static_cast<float>(layout.abilityAttackerXPlayer)
                                                                                 : static_cast<float>(layout.abilityTargetXPlayer);
            actorX = actorCenter - normalWidths.at(actorState->id) * positionFactor / 2.0F;
        }
        const auto actorScale = static_cast<float>(layout.abilityScale) + actor.scaleAdjustment;
        actor.animation->setScale(actorScale, actorScale);
        actor.animation->setPosition(actorX, static_cast<float>(layout.abilityStageY));
        actor.animation->play(ability.animation);
        updateCombatantBounds(actor);

        auto totalTargetWidth = -static_cast<float>(layout.abilityGroupPadding);
        for (const auto& target : action.affectedCombatants)
        {
            totalTargetWidth += normalWidths.at(target) * positionFactor + static_cast<float>(layout.abilityGroupPadding);
        }
        const auto targetCenter = centersOwnGroup                               ? static_cast<float>(layout.referenceWidth) / 2.0F
                                  : actorState->team == gameplay::Team::players ? static_cast<float>(layout.abilityTargetXPlayer)
                                                                                : static_cast<float>(layout.abilityAttackerXPlayer);
        auto targetX = targetCenter - totalTargetWidth / 2.0F;
        for (const auto& targetId : action.affectedCombatants)
        {
            if (targetId == action.command.actor && !centersOwnGroup)
            {
                continue;
            }
            auto& target = views.at(targetId);
            if (ability.targeting.relation == content::TargetRelation::everyoneElse)
            {
                targetX = target.bounds.position.x;
            }
            const auto targetScale = static_cast<float>(layout.abilityScale) + target.scaleAdjustment;
            target.animation->setScale(targetScale, targetScale);
            target.animation->setPosition(targetX, static_cast<float>(layout.abilityStageY));
            if (targetId != action.command.actor)
            {
                const auto* targetState = battle.find(targetId);
                const auto friendlyImpact = (actorState->team == targetState->team) != actorState->confusedThisTurn;
                target.animation->play(friendlyImpact ? "attacked_friendly" : "attacked");
            }
            updateCombatantBounds(target);
            if (ability.targeting.relation != content::TargetRelation::everyoneElse)
            {
                targetX += normalWidths.at(targetId) * positionFactor + static_cast<float>(layout.abilityGroupPadding);
            }
        }

        prepareAbilityEffect(action, views, effect, ability, *actorState, *primaryState);
        action.stage = ActionStage::execution;
        action.elapsed = 0.0F;
    }

    void applyPresentedResults(PresentedAction& action, CombatantViews& views, std::vector<FloatingText>& floating)
    {
        applyPresentedEvents(action.events, views, floating);
    }

    void applyPresentedEvents(const std::vector<gameplay::GameplayEvent>& events, CombatantViews& views, std::vector<FloatingText>& floating)
    {
        for (const auto& event : events)
        {
            if (const auto* damage = std::get_if<gameplay::DamageApplied>(&event))
            {
                auto& view = views.at(damage->target);
                view.healthAnimationStart = view.displayedHealth;
                view.targetHealth = std::max(0, view.targetHealth - damage->amount);
                view.healthAnimationElapsed = 0.0F;
                floating.push_back({damage->target, std::to_string(damage->amount), sf::Color::Red});
            }
            else if (const auto* healing = std::get_if<gameplay::HealingApplied>(&event))
            {
                auto& view = views.at(healing->target);
                view.healthAnimationStart = view.displayedHealth;
                view.targetHealth = std::min(view.maximumHealth, view.targetHealth + healing->amount);
                view.healthAnimationElapsed = 0.0F;
                floating.push_back({healing->target, std::to_string(healing->amount), sf::Color::Green});
            }
            else if (const auto* dodge = std::get_if<gameplay::AttackDodged>(&event))
            {
                floating.push_back({dodge->target, "Ausgewichen!", sf::Color::White});
            }
            else if (const auto* critical = std::get_if<gameplay::CriticalHit>(&event))
            {
                const auto criticalHealing = std::ranges::any_of(events, [critical](const auto& result) {
                    const auto* healing = std::get_if<gameplay::HealingApplied>(&result);
                    return healing != nullptr && healing->target == critical->combatant;
                });
                const auto color = criticalHealing ? sf::Color{0, 160, 0} : sf::Color{180, 0, 0};
                floating.push_back({critical->combatant, "Kritisch!", color, 0.0F, 40U});
            }
            else if (const auto* fatigue = std::get_if<gameplay::FatigueChanged>(&event))
            {
                const auto prefix = fatigue->amount > 0 ? "+" : "";
                const std::string fatigueLabel = prefix + std::to_string(fatigue->amount);
                auto& view = views.at(fatigue->combatant);
                view.fatigueAnimationStart = view.displayedFatigue;
                view.targetFatigue = fatigue->current;
                view.fatigueAnimationElapsed = 0.0F;
                floating.push_back({fatigue->combatant, fatigueLabel, sf::Color{80, 51, 128}});
            }
            else if (const auto* status = std::get_if<gameplay::StatusGained>(&event))
            {
                if (const auto label = statusNotification(status->status); label.has_value())
                {
                    floating.push_back({status->combatant,
                                        std::string{*label},
                                        sf::Color::White,
                                        0.0F,
                                        30U,
                                        FloatingTextAnchor::combatant,
                                        sf::Color::Black,
                                        3.0F,
                                        FloatingTextFont::interface});
                }
            }
            else if (const auto* expired = std::get_if<gameplay::StatusExpired>(&event);
                     expired != nullptr && expired->status == gameplay::StatusId{"status.sleeping"})
            {
                floating.push_back({expired->combatant,
                                    "Aufgewacht!",
                                    sf::Color::White,
                                    0.0F,
                                    30U,
                                    FloatingTextAnchor::combatant,
                                    sf::Color::Black,
                                    3.0F,
                                    FloatingTextFont::interface});
            }
            else if (const auto* expiredDecay = std::get_if<gameplay::StatusExpired>(&event);
                     expiredDecay != nullptr && expiredDecay->status == gameplay::StatusId{"status.decay"})
            {
                addInterfaceNotification(floating, expiredDecay->combatant, "Verfall entfernt!");
            }
            else if (const auto* expiredBuff = std::get_if<gameplay::StatusExpired>(&event);
                     expiredBuff != nullptr && expiredBuff->status == gameplay::StatusId{"status.buffed"})
            {
                addInterfaceNotification(floating, expiredBuff->combatant, "Stärkung beendet!");
            }
            else if (const auto* expiredDebuff = std::get_if<gameplay::StatusExpired>(&event);
                     expiredDebuff != nullptr && expiredDebuff->status == gameplay::StatusId{"status.debuffed"})
            {
                addInterfaceNotification(floating, expiredDebuff->combatant, "Schwächung beendet!");
            }
            else if (const auto* resisted = std::get_if<gameplay::EffectResisted>(&event))
            {
                floating.push_back({resisted->combatant,
                                    resistedStatusNotification(resisted->status),
                                    sf::Color::White,
                                    0.0F,
                                    30U,
                                    FloatingTextAnchor::combatant,
                                    sf::Color::Black,
                                    3.0F,
                                    FloatingTextFont::interface});
            }
            else if (const auto* triggered = std::get_if<gameplay::StatusTriggered>(&event);
                     triggered != nullptr && triggered->status == gameplay::StatusId{"status.hacked"})
            {
                floating.push_back({triggered->combatant, "Stärkung blockiert!", sf::Color::White});
            }
            else if (const auto* activated = std::get_if<gameplay::RuleActivated>(&event))
            {
                floating.push_back({activated->trigger,
                                    "Regel aktiviert!",
                                    sf::Color{235, 196, 65},
                                    0.0F,
                                    26U,
                                    FloatingTextAnchor::combatant,
                                    sf::Color::Black,
                                    3.0F,
                                    FloatingTextFont::interface});
            }
            else if (const auto* removed = std::get_if<gameplay::BuffsRemoved>(&event))
            {
                addInterfaceNotification(floating, removed->combatant, "Stärkungen entfernt!");
            }
            else if (const auto* removedDebuffs = std::get_if<gameplay::DebuffsRemoved>(&event))
            {
                addInterfaceNotification(floating, removedDebuffs->combatant, "Schwächungen entfernt!");
            }
            else if (const auto* transferred = std::get_if<gameplay::DecayTransferred>(&event))
            {
                addInterfaceNotification(floating, transferred->target, "Verfall übertragen!");
            }
            else if (const auto* detonated = std::get_if<gameplay::DecayDetonated>(&event))
            {
                addInterfaceNotification(floating, detonated->combatant, "Gesamter Verfall ausgelöst!");
            }
            else if (const auto* prolonged = std::get_if<gameplay::NegativeEffectsProlonged>(&event))
            {
                addInterfaceNotification(
                    floating, prolonged->combatant, "Negative Effekte +" + std::to_string(prolonged->rounds) + " Runden!");
            }
        }
    }
} // namespace soma::presentation
