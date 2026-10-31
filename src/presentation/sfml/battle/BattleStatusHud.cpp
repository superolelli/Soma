#include "presentation/sfml/battle/BattleHud.hpp"
#include "presentation/sfml/battle/BattleHudDrawing.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <ranges>

namespace soma::presentation
{
    using simulation::CombatantState;

    void appendAttribute(std::string& tooltip, int value, const std::string& name)
    {
        if (value == 0)
        {
            return;
        }
        tooltip += "\n#aaaadd ";
        tooltip += value > 0 ? "+" : "";
        tooltip += std::to_string(value) + " " + name;
    }

    void appendPercentage(std::string& tooltip, double value, const std::string& name)
    {
        if (value == 0.0)
        {
            return;
        }
        tooltip += "\n#aaaadd ";
        tooltip += value > 0.0 ? "+" : "";
        tooltip += std::to_string(static_cast<int>(std::round(value * 100.0))) + "% " + name;
    }

    std::string buffTooltip(const soma::simulation::ActiveBuff& buff)
    {
        std::string result;
        appendAttribute(result, buff.attributes.armour, "Rüstung");
        if (buff.attributes.damageMinimum == buff.attributes.damageMaximum)
        {
            appendAttribute(result, buff.attributes.damageMinimum, "Schaden");
        }
        else
        {
            appendAttribute(result, buff.attributes.damageMinimum, "minimaler Schaden");
            appendAttribute(result, buff.attributes.damageMaximum, "maximaler Schaden");
        }
        appendAttribute(result, buff.attributes.initiative, "Initiative");
        appendAttribute(result, buff.attributes.maximumHealth, "maximale Gesundheit");
        appendAttribute(result, buff.attributes.criticalHitChance, "kritische Trefferchance");
        appendAttribute(result, buff.attributes.precision, "Präzision");
        appendAttribute(result, buff.attributes.dodge, "Ausweichen");
        appendAttribute(result, buff.attributes.healing, "Heilung");
        appendAttribute(result, buff.attributes.sleepResistance, "Schlafresistenz");
        appendAttribute(result, buff.attributes.confusionResistance, "Verwirrungsresistenz");
        appendAttribute(result, buff.attributes.debuffResistance, "Debuffresistenz");
        appendAttribute(result, buff.attributes.decayResistance, "Verfallsresistenz");
        appendPercentage(result, buff.healingReceived, "erhaltene Heilung");
        appendPercentage(result, buff.damageAgainstMarked, "Schaden gegen Markierte");
        appendPercentage(result,
                         buff.damagePerNegativeStatus * buff.negativeStatusCountAtApplication,
                         "verursachter Schaden");
        if (buff.damageFromFatigue)
        {
            result += "\n#aaaadd Direktschaden: +relative Müdigkeit";
        }
        if (buff.blocksFatigueStageOne)
        {
            result += "\n#aaaadd Blockiert den Müdigkeitsmalus bei 9";
        }
        if (buff.blocksFatigueStageTwo)
        {
            result += "\n#aaaadd Blockiert den Müdigkeitsmalus bei 18";
        }
        return result;
    }

    void mergeBuffTooltipEffect(simulation::ActiveBuff& destination, const simulation::ActiveBuff& source)
    {
        destination.attributes.armour += source.attributes.armour;
        destination.attributes.damageMinimum += source.attributes.damageMinimum;
        destination.attributes.damageMaximum += source.attributes.damageMaximum;
        destination.attributes.initiative += source.attributes.initiative;
        destination.attributes.maximumHealth += source.attributes.maximumHealth;
        destination.attributes.criticalHitChance += source.attributes.criticalHitChance;
        destination.attributes.precision += source.attributes.precision;
        destination.attributes.dodge += source.attributes.dodge;
        destination.attributes.healing += source.attributes.healing;
        destination.attributes.sleepResistance += source.attributes.sleepResistance;
        destination.attributes.confusionResistance += source.attributes.confusionResistance;
        destination.attributes.debuffResistance += source.attributes.debuffResistance;
        destination.attributes.decayResistance += source.attributes.decayResistance;
        destination.healingReceived += source.healingReceived;
        destination.damageAgainstMarked += source.damageAgainstMarked;
        destination.damagePerNegativeStatus += source.damagePerNegativeStatus * source.negativeStatusCountAtApplication;
        destination.negativeStatusCountAtApplication = 1;
        destination.damageFromFatigue = destination.damageFromFatigue || source.damageFromFatigue;
        destination.blocksFatigueStageOne = destination.blocksFatigueStageOne || source.blocksFatigueStageOne;
        destination.blocksFatigueStageTwo = destination.blocksFatigueStageTwo || source.blocksFatigueStageTwo;
    }

    std::string combinedBuffTooltip(const std::vector<simulation::ActiveBuff>& effects)
    {
        std::vector<int> expirationTurns;
        expirationTurns.reserve(effects.size());
        for (const auto& effect : effects)
        {
            if (effect.remainingTurns > 0)
            {
                expirationTurns.push_back(effect.remainingTurns);
            }
        }
        std::ranges::sort(expirationTurns);
        const auto uniqueEnd = std::ranges::unique(expirationTurns).begin();
        expirationTurns.erase(uniqueEnd, expirationTurns.end());

        std::string result;
        auto previousExpiration = 0;
        for (std::size_t index = 0; index < expirationTurns.size(); ++index)
        {
            const auto expiration = expirationTurns.at(index);
            simulation::ActiveBuff combined;
            for (const auto& effect : effects)
            {
                if (effect.remainingTurns > previousExpiration)
                {
                    mergeBuffTooltipEffect(combined, effect);
                }
            }
            if (!result.empty())
            {
                result += "\n\n";
            }
            const auto phaseDuration = expiration - previousExpiration;
            result += index == 0 ? "#d6b45c Für " : "#d6b45c Danach für ";
            result += std::to_string(phaseDuration) + (phaseDuration == 1 ? " Runde#ffffff" : " Runden#ffffff");
            result += buffTooltip(combined);
            previousExpiration = expiration;
        }
        return result;
    }

    std::string decayTooltip(const std::vector<simulation::ActiveDecay>& effects)
    {
        std::string result = "Verfall";
        for (const auto& effect : effects)
        {
            const auto duration = std::max(0, effect.remainingTurns);
            result += "\n" + std::to_string(std::max(0, effect.damage)) + " Schaden für noch " + std::to_string(duration) +
                      (duration == 1 ? " Runde" : " Runden");
        }
        return result;
    }

    std::string transfusionTooltip(const std::vector<simulation::ActiveTransfusion>& effects)
    {
        std::string result = effects.size() == 1 ? "Transfusion" : std::to_string(effects.size()) + " Transfusionen";
        for (const auto& effect : effects)
        {
            result += "\nBei Verfallsschaden: Heilt einen zufälligen Gegner um " + std::to_string(std::max(0, effect.healing));
        }
        result += "\nEndet, sobald kein Verfall mehr vorhanden ist";
        return result;
    }

    std::vector<StatusIconInfo> activeStatusIcons(const BattleTextures& textures, const CombatantState& state)
    {
        std::vector<StatusIconInfo> icons;
        if (state.dynamite)
        {
            icons.push_back({&textures.dynamite, "Dynamit\n20% Explosion: 15 Schaden\nSonst Weitergabe", gameplay::StatusId{"status.dynamite"}});
        }
        if (state.sleeping)
        {
            icons.push_back({&textures.sleeping, "Schläft und überspringt den nächsten Zug", gameplay::StatusId{"status.sleeping"}});
        }
        if (state.fistOfRevengeTarget)
        {
            icons.push_back({&textures.fistOfRevenge, "Angreifbar durch Fist of Revenge", gameplay::StatusId{"status.fist_of_revenge"}});
        }
        if (state.nofaceBuffLevel >= 0)
        {
            icons.push_back({&textures.noface,
                             "Seine Wunden machen ihn stärker\nStufe " + std::to_string(state.nofaceBuffLevel),
                             gameplay::StatusId{"status.noface"}});
        }
        if (!state.decay.empty())
        {
            icons.push_back({&textures.decay, decayTooltip(state.decay), gameplay::StatusId{"status.decay"}});
        }
        if (state.confusedTurns > 0)
        {
            icons.push_back(
                {&textures.confused, "Verwirrt (" + std::to_string(state.confusedTurns) + " Runden)", gameplay::StatusId{"status.confused"}});
        }
        if (state.hackedTurns > 0)
        {
            icons.push_back({&textures.hacked,
                             "Gehackt (" + std::to_string(state.hackedTurns) + " Runden)\nKann keine Stärkungen erhalten",
                             gameplay::StatusId{"status.hacked"}});
        }
        if (!state.transfusions.empty())
        {
            icons.push_back({&textures.transfusion, transfusionTooltip(state.transfusions), gameplay::StatusId{"status.transfusion"}});
        }
        if (state.panicStacks > 0)
        {
            icons.push_back({&textures.panic,
                             "Panik (" + std::to_string(state.panicStacks) + " Stapel)\nPräzision: -" +
                                 std::to_string(state.panicStacks * 5),
                             gameplay::StatusId{"status.panic"}});
        }
        if (!state.buffs.empty())
        {
            icons.push_back({&textures.buff, combinedBuffTooltip(state.buffs), gameplay::StatusId{"status.buffed"}});
        }
        if (!state.debuffs.empty())
        {
            icons.push_back({&textures.debuff, combinedBuffTooltip(state.debuffs), gameplay::StatusId{"status.debuffed"}});
        }
        if (state.markedTurns > 0)
        {
            icons.push_back({&textures.marked, "Markiert (" + std::to_string(state.markedTurns) + " Runden)", gameplay::StatusId{"status.marked"}});
        }
        if (state.misses > 0)
        {
            icons.push_back({&textures.missed,
                             std::to_string(state.misses) + (state.misses == 1 ? " Fehlschuss" : " Fehlschüsse"),
                             gameplay::StatusId{"status.miss"}});
        }
        if (state.bountyTurns > 0)
        {
            icons.push_back({&textures.bounty,
                             "Kopfgeld (" + std::to_string(state.bountyTurns) + " Runden)\nBei Schaden: +2 Schaden für Angreifer",
                             gameplay::StatusId{"status.bounty"}});
        }
        if (state.riposteTurns > 0)
        {
            icons.push_back({&textures.riposte,
                             "Gegenschlag (" + std::to_string(state.riposteTurns) + " Runden)\nKontert jeden feindlichen Angriff",
                             gameplay::StatusId{"status.riposte"}});
        }
        if (state.rageTurns > 0)
        {
            icons.push_back({&textures.rage,
                             "Raserei (" + std::to_string(state.rageTurns) + " Runden)\nVerursachter und erlittener Schaden: +50%",
                             gameplay::StatusId{"status.rage"}});
        }
        const auto fatigueStage = gameplay::fatigueStage(state.fatigue);
        if (fatigueStage > 0)
        {
            const auto appliedStages = std::popcount(static_cast<unsigned int>(state.appliedFatiguePenaltyStages));
            const auto damagePenalty = appliedStages * 3;
            const auto generalPenalty = appliedStages * 5;
            const auto blocked = appliedStages == 0         ? "\n#88cc88 Dieser Effekt ist blockiert"
                                 : appliedStages < fatigueStage ? "\n#88cc88 Dieser Effekt ist teilweise blockiert"
                                                               : "";
            icons.push_back({fatigueStage == 1 ? &textures.fatigueTired : &textures.fatigueStupid,
                             "Müdigkeit Stufe " + std::to_string(fatigueStage) + "\n-" + std::to_string(damagePenalty) +
                                 " Schaden"
                                 "\n-" +
                                 std::to_string(generalPenalty) +
                                 " Ausweichen und Präzision"
                                 "\n-" +
                                 std::to_string(appliedStages) + " Initiative" + blocked,
                             gameplay::StatusId{"status.fatigue." + std::to_string(fatigueStage)}});
        }
        return icons;
    }

    void drawStatusIcons(sf::RenderWindow& window,
                         const BattleTextures& textures,
                         const CombatantState& state,
                         const CombatantView& view,
                         const std::vector<gameplay::StatusId>& newlyGainedStatuses,
                         float statusAnimationElapsed)
    {
        const auto healthBarX = view.bounds.position.x + view.bounds.size.x / 2.0F - textures.healthBarFrame.getSize().x / 2.0F;
        auto statusY = view.bounds.position.y + view.bounds.size.y + 30.0F + textures.healthBarFrame.getSize().y + 2.0F;
        if (state.team == gameplay::Team::players)
        {
            statusY += static_cast<float>(textures.fatigueBarFrame.getSize().y) + 2.0F;
        }
        auto position = sf::Vector2f{healthBarX, statusY};
        for (const auto& status : activeStatusIcons(textures, state))
        {
            auto icon = detail::hudSprite(*status.texture, position);
            if (std::ranges::find(newlyGainedStatuses, status.status) != newlyGainedStatuses.end() && statusAnimationElapsed < 0.5F)
            {
                const auto progress = std::clamp(statusAnimationElapsed / 0.5F, 0.0F, 1.0F);
                const auto inverse = 1.0F - progress;
                const auto scale = inverse * inverse * inverse * 1.5F + 3.0F * progress * inverse * inverse * 0.64F +
                                   3.0F * progress * progress * inverse * 0.635F + progress * progress * progress;
                const auto iconSize = detail::textureSize(*status.texture);
                icon.setOrigin(iconSize / 2.0F);
                icon.setPosition(position + iconSize / 2.0F);
                icon.setScale({scale, scale});
            }
            window.draw(icon);
            position.x += 20.0F;
        }
    }
} // namespace soma::presentation
