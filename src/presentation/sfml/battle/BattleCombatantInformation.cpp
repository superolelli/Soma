#include "presentation/sfml/battle/BattleHud.hpp"
#include "presentation/sfml/battle/BattleHudDrawing.hpp"
#include "simulation/battle/AbilityTargets.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace soma::presentation
{
    using simulation::CombatantState;

    sf::Color changedValueColor(int current, int base)
    {
        if (current > base)
        {
            return sf::Color::Green;
        }
        if (current < base)
        {
            return sf::Color::Red;
        }
        return sf::Color{220, 220, 220};
    }

    void drawLargeFatigueBar(sf::RenderWindow& window,
                             const BattleTextures& textures,
                             const sf::Font& font,
                             const CombatantState& state,
                             const CombatantView& view,
                             sf::Vector2f healthPosition)
    {
        if (state.team != gameplay::Team::players)
        {
            return;
        }

        const auto healthFrameSize = detail::textureSize(textures.healthBarBigFrame);
        const auto fatigueFrameSize = detail::textureSize(textures.fatigueBarBigFrame);
        const auto fatiguePosition =
            healthPosition + sf::Vector2f{(healthFrameSize.x - fatigueFrameSize.x) / 2.0F, healthFrameSize.y + 4.0F};
        const auto fillWidth = textures.fatigueBarBig.getSize().x / 2U;
        const auto ratio = std::clamp(view.displayedFatigue / static_cast<float>(gameplay::maximumFatigue), 0.0F, 1.0F);
        const auto visibleWidth = static_cast<unsigned int>(static_cast<float>(fillWidth) * ratio);
        if (visibleWidth > 0U)
        {
            const auto fillSize = sf::Vector2f{static_cast<float>(fillWidth), static_cast<float>(textures.fatigueBarBig.getSize().y)};
            auto fill = detail::hudSprite(textures.fatigueBarBig,
                               fatiguePosition + sf::Vector2f{(fatigueFrameSize.x - fillSize.x) / 2.0F,
                                                              (fatigueFrameSize.y - fillSize.y) / 2.0F});
            fill.setTextureRect({{0, 0}, {static_cast<int>(visibleWidth), static_cast<int>(textures.fatigueBarBig.getSize().y)}});
            window.draw(fill);
        }
        window.draw(detail::hudSprite(textures.fatigueBarBigFrame, fatiguePosition));

        const auto fatigueText = std::to_string(static_cast<int>(std::round(view.displayedFatigue))) + "/" +
                                 std::to_string(gameplay::maximumFatigue);
        auto fatigueLabel = detail::hudText(font, fatigueText, 14U, {});
        const auto labelBounds = fatigueLabel.getLocalBounds();
        fatigueLabel.setPosition({fatiguePosition.x + (fatigueFrameSize.x - labelBounds.size.x) / 2.0F - labelBounds.position.x,
                                  fatiguePosition.y + (fatigueFrameSize.y - labelBounds.size.y) / 2.0F - labelBounds.position.y});
        window.draw(fatigueLabel);
    }

    void drawCombatantInformation(sf::RenderWindow& window,
                                  const BattleTextures& textures,
                                  const sf::Font& font,
                                  const CombatantState& state,
                                  const soma::content::BattleLayoutDefinition& layout,
                                  const CombatantView& view)
    {
        const sf::Vector2f panelPosition{static_cast<float>(layout.informationPanelX), static_cast<float>(layout.informationPanelY)};
        const auto healthPosition = panelPosition + sf::Vector2f{40.0F, 70.0F};
        window.draw(detail::hudSprite(textures.combatantInformationPanel, panelPosition));

        auto name = detail::hudText(font, state.name, 25U, {});
        const auto nameWidth = name.getLocalBounds().size.x;
        name.setPosition({healthPosition.x + (textures.healthBarBigFrame.getSize().x - nameWidth) / 2.0F, panelPosition.y + 25.0F});
        window.draw(name);

        const auto ratio = std::clamp(static_cast<float>(view.displayedHealth) / static_cast<float>(std::max(1, view.maximumHealth)), 0.0F, 1.0F);
        auto healthFill = detail::hudSprite(textures.healthBarBig, healthPosition + sf::Vector2f{17.0F, 20.0F});
        const auto healthFrameWidth = textures.healthBarBig.getSize().x / 2U;
        healthFill.setTextureRect({{0, 0}, {static_cast<int>(healthFrameWidth * ratio), static_cast<int>(textures.healthBarBig.getSize().y)}});
        window.draw(healthFill);
        window.draw(detail::hudSprite(textures.healthBarBigFrame, healthPosition));

        const auto healthText = std::to_string(static_cast<int>(view.displayedHealth)) + "/" + std::to_string(view.maximumHealth);
        auto healthLabel = detail::hudText(font, healthText, 14U, {});
        const auto healthBounds = healthLabel.getLocalBounds();
        const auto healthFrameSize = detail::textureSize(textures.healthBarBigFrame);
        healthLabel.setPosition({healthPosition.x + (healthFrameSize.x - healthBounds.size.x) / 2.0F - healthBounds.position.x,
                                 healthPosition.y + (healthFrameSize.y - healthBounds.size.y) / 2.0F - healthBounds.position.y});
        window.draw(healthLabel);
        drawLargeFatigueBar(window, textures, font, state, view, healthPosition);

        const std::array attributeLabels{"Rüstung:", "Schaden:", "Initiative:", "Kritische Trefferchance:", "Ausweichen:", "Präzision:"};
        const auto nonNegative = [](int value) {
            return std::max(0, value);
        };
        const std::array attributeValues{
            std::to_string(nonNegative(state.attributes.armour)),
            std::to_string(nonNegative(state.attributes.damageMinimum)) + "-" + std::to_string(nonNegative(state.attributes.damageMaximum)),
            std::to_string(nonNegative(state.attributes.initiative)),
            std::to_string(nonNegative(state.attributes.criticalHitChance)),
            std::to_string(nonNegative(state.attributes.dodge)),
            std::to_string(nonNegative(state.attributes.precision))};
        const std::array attributeColors{changedValueColor(state.attributes.armour, state.baseAttributes.armour),
                                         changedValueColor(state.attributes.damageMinimum, state.baseAttributes.damageMinimum),
                                         changedValueColor(state.attributes.initiative, state.baseAttributes.initiative),
                                         changedValueColor(state.attributes.criticalHitChance, state.baseAttributes.criticalHitChance),
                                         changedValueColor(state.attributes.dodge, state.baseAttributes.dodge),
                                         changedValueColor(state.attributes.precision, state.baseAttributes.precision)};
        sf::RectangleShape attributeBackground{{390.0F, 145.0F}};
        attributeBackground.setPosition(panelPosition + sf::Vector2f{648.0F, 16.0F});
        attributeBackground.setFillColor(sf::Color{0, 0, 0, 220});
        attributeBackground.setOutlineColor(sf::Color{40, 40, 40});
        attributeBackground.setOutlineThickness(2.0F);
        window.draw(attributeBackground);
        auto y = panelPosition.y + 25.0F;
        for (std::size_t index = 0; index < attributeLabels.size(); ++index)
        {
            window.draw(detail::hudText(font, attributeLabels.at(index), 16U, {panelPosition.x + 660.0F, y}, sf::Color{150, 150, 150}));
            window.draw(detail::hudText(font, attributeValues.at(index), 16U, {panelPosition.x + 895.0F, y}, attributeColors.at(index)));
            y += 20.0F;
        }

        window.draw(detail::hudText(font, "Resistenzen", 20U, panelPosition + sf::Vector2f{510.0F, 25.0F}));
        const std::array resistanceValues{nonNegative(state.secondaryAttributes.sleepResistance),
                                          nonNegative(state.secondaryAttributes.confusionResistance),
                                          nonNegative(state.secondaryAttributes.debuffResistance),
                                          nonNegative(state.secondaryAttributes.decayResistance)};
        const std::array<const sf::Texture*, 4> resistanceIcons{&textures.sleeping, &textures.confused, &textures.debuff, &textures.decay};
        y = panelPosition.y + 60.0F;
        for (std::size_t index = 0; index < resistanceValues.size(); ++index)
        {
            window.draw(detail::hudSprite(*resistanceIcons.at(index), {panelPosition.x + 540.0F, y - 2.0F}));
            window.draw(detail::hudText(font, std::to_string(resistanceValues.at(index)) + "%", 16U, {panelPosition.x + 570.0F, y}, sf::Color{220, 150, 0}));
            y += 25.0F;
        }
    }

    void drawExpectedAttack(sf::RenderWindow& window,
                            const sf::Font& font,
                            const soma::simulation::BattleSimulation& battle,
                            const CombatantState& actor,
                            const CombatantState& target,
                            const soma::content::AbilityDefinition& ability,
                            const soma::content::BattleLayoutDefinition& layout)
    {
        const auto friendly = actor.team == target.team;
        const auto& effect = friendly ? ability.friendlyEffect : ability.hostileEffect;
        const auto targetCount = static_cast<int>(simulation::abilityTargets(ability.targeting, actor, target, battle.combatants()).size());
        const auto targetAttributes = battle.effectiveAttributes(target);
        const auto actorAttributes = battle.effectiveAttributes(actor);
        const auto hitProbability = friendly || actor.id == target.id
                                        ? 100
                                        : target.misses > 0
                                              ? 0
                                              : std::clamp(100 -
                                                               (targetAttributes.dodge -
                                                                (actorAttributes.precision + ability.precisionModifier)) *
                                                                   2,
                                                           0,
                                                           100);

        std::vector<std::string> estimates;
        if (effect.damageFactor != 0.0)
        {
            const auto additionalFactor = effect.lessTargetsMoreDamage * (ability.targeting.targetCount - targetCount);
            auto conditionalFactor = effect.damageFactor + additionalFactor;
            if (target.markedTurns > 0)
            {
                conditionalFactor += effect.damageAgainstMarked;
                for (const auto& buff : actor.buffs)
                {
                    conditionalFactor += buff.damageAgainstMarked;
                }
            }
            if (target.fistOfRevengeTarget)
            {
                conditionalFactor += effect.damageAgainstRevengeMarked;
            }
            for (const auto& buff : actor.buffs)
            {
                if (buff.damageFromFatigue)
                {
                    conditionalFactor += static_cast<double>(actor.fatigue) / gameplay::maximumFatigue;
                }
            }
            const auto armour = effect.ignoreArmour ? 0 : targetAttributes.armour;
            const auto damageAfterArmour = [&battle, &actor, &target, armour, conditionalFactor](int damage) {
                const auto abilityDamage = static_cast<int>(damage * conditionalFactor);
                const auto modifiedDamage = battle.modifiedDamage(actor, target, abilityDamage);
                return modifiedDamage - static_cast<int>(std::round(armour / 100.0 * modifiedDamage));
            };
            const auto minimum = std::max(0, damageAfterArmour(actor.attributes.damageMinimum));
            const auto maximum = std::max(0, damageAfterArmour(actor.attributes.damageMaximum));
            const auto damageText = minimum == maximum ? std::to_string(minimum) : std::to_string(minimum) + "-" + std::to_string(maximum);
            estimates.push_back("Treffer: " + std::to_string(hitProbability) + "% | Schaden: " + damageText);
        }

        const auto statusProbability = [hitProbability](double baseProbability, int resistance) {
            const auto base = std::clamp(baseProbability, 0.0, 1.0);
            const auto resistanceFactor = 1.0 - std::clamp(resistance, 0, 100) / 100.0;
            return static_cast<int>(std::round(hitProbability * base * resistanceFactor));
        };
        std::vector<std::string> statusEstimates;
        if (effect.confusionRounds > 0)
        {
            statusEstimates.push_back("Verwirrung: " +
                                      std::to_string(statusProbability(effect.confusionProbability,
                                                                       target.secondaryAttributes.confusionResistance)) +
                                      "%");
        }
        if (effect.sleepProbability > 0.0)
        {
            auto probability = effect.sleepProbability;
            if (effect.increaseSleepProbabilityByFatigue)
            {
                probability += static_cast<double>(actor.fatigue) / gameplay::maximumFatigue;
            }
            statusEstimates.push_back("Schlaf: " +
                                      std::to_string(statusProbability(probability, target.secondaryAttributes.sleepResistance)) + "%");
        }
        if (effect.decayRounds > 0)
        {
            const auto resistance = effect.ignoreDecayResistance ? 0 : target.secondaryAttributes.decayResistance;
            statusEstimates.push_back("Verfall: " + std::to_string(statusProbability(1.0, resistance)) + "%");
        }
        if (effect.targetBuff.duration > 0 && !effect.targetBuff.positive)
        {
            statusEstimates.push_back("Schwächung: " +
                                      std::to_string(statusProbability(1.0, target.secondaryAttributes.debuffResistance)) + "%");
        }
        if (!statusEstimates.empty())
        {
            auto statusLine = statusEstimates.front();
            for (std::size_t index = 1; index < statusEstimates.size(); ++index)
            {
                statusLine += " | " + statusEstimates[index];
            }
            estimates.push_back(std::move(statusLine));
        }

        auto y = static_cast<float>(estimates.size() == 1 ? layout.expectedAttackSingleY : layout.expectedAttackMultipleY);
        for (const auto& value : estimates)
        {
            auto estimate = detail::hudText(font, value, static_cast<unsigned int>(layout.expectedAttackTextSize), {}, sf::Color{200, 200, 130});
            estimate.setStyle(sf::Text::Bold);
            estimate.setPosition({static_cast<float>(layout.expectedAttackCenterX) - estimate.getLocalBounds().size.x / 2.0F, y});
            window.draw(estimate);
            y += static_cast<float>(layout.expectedAttackLineSpacing);
        }
    }
} // namespace soma::presentation
