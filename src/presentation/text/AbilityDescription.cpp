#include "presentation/text/AbilityDescription.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string_view>
#include <vector>

namespace soma::presentation
{
    namespace
    {
        void append(std::string& result, std::string_view color, const std::string& text)
        {
            if (text.empty()) return;
            result += "\n#";
            result += color;
            result += ' ';
            result += text;
        }

        std::string percent(double value)
        {
            return std::to_string(static_cast<int>(std::round(value * 100.0))) + "%";
        }

        std::string signedInteger(int value)
        {
            return std::string{value > 0 ? "+" : ""} + std::to_string(value);
        }

        struct DescriptionItem
        {
            std::string_view color;
            std::string text;
        };

        void addItem(std::vector<DescriptionItem>& items, std::string_view color, std::string text)
        {
            if (!text.empty())
            {
                items.push_back({color, std::move(text)});
            }
        }

        void addAttribute(std::vector<DescriptionItem>& items, std::string_view name, int value)
        {
            if (value != 0)
            {
                addItem(items, "aaaadd", std::string{name} + (value > 0 ? ": +" : ": ") + std::to_string(value));
            }
        }

        void appendRow(std::string& result, const std::vector<DescriptionItem>& items)
        {
            if (items.empty())
            {
                return;
            }
            result += '\n';
            for (std::size_t index = 0; index < items.size(); ++index)
            {
                if (index > 0)
                {
                    result += " #888888 · ";
                }
                result += '#';
                result += items.at(index).color;
                result += ' ';
                result += items.at(index).text;
            }
        }

        void appendBuffDescription(std::string& result, const content::BuffDefinition& definition, std::string_view recipient)
        {
            if (definition.duration <= 0)
            {
                return;
            }
            std::vector<DescriptionItem> buff;
            addItem(buff,
                    definition.positive ? "88cc88" : "cc8888",
                    std::string{recipient} + std::to_string(definition.duration) + " Runden");
            addAttribute(buff, "Rüstung", definition.attributes.armour);
            if (definition.attributes.damageMinimum == definition.attributes.damageMaximum)
            {
                addAttribute(buff, "Schaden", definition.attributes.damageMinimum);
            }
            else
            {
                addAttribute(buff, "Schaden (Minimum)", definition.attributes.damageMinimum);
                addAttribute(buff, "Schaden (Maximum)", definition.attributes.damageMaximum);
            }
            addAttribute(buff, "Initiative", definition.attributes.initiative);
            addAttribute(buff, "Maximale Gesundheit", definition.attributes.maximumHealth);
            addAttribute(buff, "Kritische Trefferchance", definition.attributes.criticalHitChance);
            addAttribute(buff, "Präzision", definition.attributes.precision);
            addAttribute(buff, "Ausweichen", definition.attributes.dodge);
            addAttribute(buff, "Heilung", definition.attributes.healing);
            addAttribute(buff, "Schlafresistenz", definition.attributes.sleepResistance);
            addAttribute(buff, "Verwirrungsresistenz", definition.attributes.confusionResistance);
            addAttribute(buff, "Debuffresistenz", definition.attributes.debuffResistance);
            addAttribute(buff, "Verfallsresistenz", definition.attributes.decayResistance);
            if (definition.healingReceived != 0.0)
            {
                addItem(buff, "00aa00", "Erhaltene Heilung: +" + percent(definition.healingReceived));
            }
            if (definition.damageAgainstMarked != 0.0)
            {
                addItem(buff, "ffaa00", "Schaden gegen Markierte: +" + percent(definition.damageAgainstMarked));
            }
            appendRow(result, buff);
            if (definition.damagePerNegativeStatus != 0.0)
            {
                append(result,
                       "cc8888",
                       "Schaden pro unterschiedlichem negativem Status beim Anwenden: " + percent(definition.damagePerNegativeStatus));
            }
            if (definition.damageFromFatigue)
            {
                append(result, "ffaa00", "Direktschaden: +relative Müdigkeit");
            }
            if (definition.blocksFatigueStageOne)
            {
                append(result, "88cc88", "Blockiert den Müdigkeitsmalus bei 9");
            }
            if (definition.blocksFatigueStageTwo)
            {
                append(result, "88cc88", "Blockiert den Müdigkeitsmalus bei 18");
            }
        }

        std::string effectDescription(const content::AbilityEffect& effect)
        {
            std::string result;
            std::vector<DescriptionItem> directEffects;
            if (effect.damageFactor != 0.0) addItem(directEffects, "ffffff", "Schaden: " + percent(effect.damageFactor));
            if (effect.heal != 0) addItem(directEffects, "00aa00", "Heilung: " + std::to_string(effect.heal));
            if (effect.healSelf != 0) addItem(directEffects, "00aa00", "Eigene Heilung: " + std::to_string(effect.healSelf));
            if (effect.targetFatigue != 0)
                addItem(directEffects,
                        "503380",
                        std::string{"Ziel "} + (effect.targetFatigue > 0 ? "erhält " : "verliert ") +
                            std::to_string(std::abs(effect.targetFatigue)) + " Müdigkeit");
            if (effect.criticalHitModifier != 0)
                addItem(directEffects,
                        "ff8800",
                        "Kritische Trefferchance: " + signedInteger(effect.criticalHitModifier) + "%");
            if (effect.ignoreArmour) addItem(directEffects, "ffffff", "Ignoriert Rüstung");
            appendRow(result, directEffects);

            if (effect.lessTargetsMoreDamage != 0.0)
                append(result,
                       "ffffff",
                       "Zusätzlicher Schaden pro nicht getroffenem möglichen Ziel: " + percent(effect.lessTargetsMoreDamage));
            if (effect.damageAgainstMarked != 0.0)
                append(result, "ffaa00", "Zusätzlicher Schaden gegen Markierte: " + percent(effect.damageAgainstMarked));
            if (effect.damageAgainstRevengeMarked != 0.0)
                append(result, "ffaa00", "Zusätzlicher Schaden gegen Rachemarkierte: " + percent(effect.damageAgainstRevengeMarked));
            if (effect.successiveTargetDamageReduction != 0.0)
                append(result,
                       "ffffff",
                       "Schadensreduktion je weiterem Ziel: " + percent(effect.successiveTargetDamageReduction));
            if (effect.transferredMissingHealthDamageFactor != 0.0)
                append(result,
                       "ffffff",
                       "Zusatzschaden am Ziel: " + percent(effect.transferredMissingHealthDamageFactor) +
                           " des fehlenden Lebens des nächsten Kämpfers");
            if (effect.transferredMissingHealthHealingFactor != 0.0)
                append(result,
                       "00aa00",
                       "Heilung des nächsten Kämpfers: " + percent(effect.transferredMissingHealthHealingFactor) +
                           " seines fehlenden Lebens");
            if (effect.moveTargetsRandomly) append(result, "ffffff", "Betroffene Ziele wechseln zufällig ihre Position");

            std::vector<DescriptionItem> statuses;
            if (effect.confusionProbability != 0.0)
                addItem(statuses,
                        "bb77bb",
                        "Verwirrung: " + percent(effect.confusionProbability) + " / " + std::to_string(effect.confusionRounds) + " Runden");
            if (effect.decayRounds != 0)
                addItem(statuses,
                        "aa6644",
                        "Verfall: " + std::to_string(effect.decayDamage) + " Schaden / " + std::to_string(effect.decayRounds) + " Runden");
            if (effect.decayDamageAgainstMarked != 0)
                append(result,
                       "aa6644",
                       "Verfallsschaden gegen Markierte: +" + std::to_string(effect.decayDamageAgainstMarked));
            if (effect.decayRoundsAgainstConfused != 0)
                append(result,
                       "aa6644",
                       "Verfallsdauer gegen Verwirrte: +" + std::to_string(effect.decayRoundsAgainstConfused) + " Runden");
            if (effect.decayRoundsAgainstSleeping != 0)
                append(result,
                       "aa6644",
                       "Verfallsdauer gegen Schlafende: +" + std::to_string(effect.decayRoundsAgainstSleeping) + " Runden");
            if (effect.markRounds != 0) addItem(statuses, "ffaa00", "Markiert: " + std::to_string(effect.markRounds) + " Runden");
            if (effect.sleepProbability != 0.0) addItem(statuses, "7777cc", "Schlaf: " + percent(effect.sleepProbability));
            if (effect.addMiss != 0) addItem(statuses, "ffffff", "Fehlschläge: +" + std::to_string(effect.addMiss));
            if (effect.addMissSelf != 0) addItem(statuses, "ffffff", "Eigene Fehlschläge: +" + std::to_string(effect.addMissSelf));
            if (effect.dynamite) addItem(statuses, "ff5500", "Dynamit");
            if (effect.bountyRounds != 0)
                addItem(statuses, "ffaa00", "Kopfgeld: " + std::to_string(effect.bountyRounds) + " Runden");
            if (effect.riposteRounds != 0)
                addItem(statuses, "e0c050", "Gegenschlag: " + std::to_string(effect.riposteRounds) + " Runden");
            if (effect.riposteSelfRounds != 0)
                addItem(statuses, "e0c050", "Eigener Gegenschlag: " + std::to_string(effect.riposteSelfRounds) + " Runden");
            if (effect.rageRounds != 0) addItem(statuses, "cc5533", "Raserei: " + std::to_string(effect.rageRounds) + " Runden");
            if (effect.hackedRounds != 0)
                addItem(statuses, "cc8888", "Gehackt: " + std::to_string(effect.hackedRounds) + " Runden");
            if (effect.transfusionHealing != 0)
                addItem(statuses, "cc3355", "Transfusion: " + std::to_string(effect.transfusionHealing));
            if (effect.panicStacks != 0) addItem(statuses, "cc6688", "Panik: +" + std::to_string(effect.panicStacks));
            appendRow(result, statuses);

            std::vector<DescriptionItem> statusChanges;
            if (effect.removeBuffs) addItem(statusChanges, "ffffff", "Entfernt positive Effekte");
            if (effect.removeDebuffs) addItem(statusChanges, "ffffff", "Entfernt negative Effekte");
            if (effect.stealBuffs) addItem(statusChanges, "ffffff", "Stiehlt positive Effekte");
            appendRow(result, statusChanges);

            if (effect.doublePanic) append(result, "cc6688", "Verdoppelt zuerst bestehende Panik");
            if (effect.damageFromPanic)
                append(result, "cc6688", "Ab 10 Panik: halbiert Panik und verursacht ebenso viel rüstungsignorierenden Schaden");
            if (effect.panicAgainstMarked != 0)
                append(result, "cc6688", "Zusätzliche Panik gegen Markierte: +" + std::to_string(effect.panicAgainstMarked));
            if (effect.spreadPanicTargets != 0)
                append(result,
                       "cc6688",
                       "Panik kann auf " + std::to_string(effect.spreadPanicTargets) + " weitere Ziele überspringen");
            if (effect.ignoreDecayResistance) append(result, "aa6644", "Verfall ignoriert Verfallsresistenz");
            if (effect.inflictAllDecayAndRemove) append(result, "aa6644", "Löst allen verbleibenden Verfall sofort aus");
            if (effect.transferDecayOnKill) append(result, "aa6644", "Überträgt Verfall bei einem tödlichen Treffer");
            if (effect.removeDecayAndHealParty)
                append(result, "00aa00", "Entfernt Verfall und verteilt dessen Restschaden als Heilung auf alle lebenden Spieler");
            if (effect.randomNegativeStatusProbability != 0.0)
                append(result, "cc8888", "Zufälliger negativer Status: " + percent(effect.randomNegativeStatusProbability));
            if (effect.prolongNegativeEffects != 0)
                append(result, "cc8888", "Bestehende negative Effekte: +" + std::to_string(effect.prolongNegativeEffects) + " Runden");
            if (effect.increaseSleepProbabilityByFatigue)
                append(result, "7777cc", "Schlafchance: +relative Müdigkeit des Anwenders");
            if (effect.sleepFailureDecayRounds != 0)
                append(result,
                       "aa6644",
                       "Bei erfolglosem Schlaf: " + std::to_string(effect.sleepFailureDecayDamage) + " Verfallsschaden für " +
                           std::to_string(effect.sleepFailureDecayRounds) + " Runden");
            appendBuffDescription(result, effect.targetBuff, "Für ");
            appendBuffDescription(result, effect.actorBuff, "Auf Anwender für ");
            return result;
        }

        std::string targetSummary(const content::Targeting& targeting)
        {
            if (targeting.relation == content::TargetRelation::self) return "Anwender";
            if (targeting.relation == content::TargetRelation::everyoneElse) return "alle anderen Kämpfer";
            const auto singular = targeting.relation == content::TargetRelation::ownTeam       ? "Freund"
                                  : targeting.relation == content::TargetRelation::opposingTeam ? "Gegner"
                                                                                               : "Kämpfer";
            const auto plural = targeting.relation == content::TargetRelation::ownTeam       ? "Freunde"
                                : targeting.relation == content::TargetRelation::opposingTeam ? "Gegner"
                                                                                             : "Kämpfer";
            if (targeting.scope == content::TargetScope::team) return std::string{"alle "} + plural;
            if (targeting.scope == content::TargetScope::randomSingle) return std::string{"1 zufälliger "} + singular;
            const auto targetCount = targeting.scope == content::TargetScope::count ? targeting.targetCount : 1;
            return std::to_string(targetCount) + ' ' + (targetCount == 1 ? singular : plural);
        }

        std::string fatigueChange(std::string_view subject, int amount, bool plural = false)
        {
            const auto verb = amount > 0 ? (plural ? " erhalten " : " erhält ") : (plural ? " verlieren " : " verliert ");
            return std::string{subject} + verb + std::to_string(std::abs(amount)) + " Müdigkeit";
        }

        template <typename Operation>
        void forEachRelevantEffect(const content::AbilityDefinition& ability, Operation&& operation)
        {
            switch (ability.targeting.relation)
            {
            case content::TargetRelation::self:
            case content::TargetRelation::ownTeam:
                operation(ability.friendlyEffect);
                break;
            case content::TargetRelation::opposingTeam:
                operation(ability.hostileEffect);
                break;
            case content::TargetRelation::eitherTeam:
            case content::TargetRelation::everyoneElse:
                operation(ability.friendlyEffect);
                operation(ability.hostileEffect);
                break;
            }
        }

        void appendRelevantEffects(std::string& result, const content::AbilityDefinition& ability)
        {
            if (ability.targeting.relation == content::TargetRelation::self ||
                ability.targeting.relation == content::TargetRelation::ownTeam)
            {
                result += effectDescription(ability.friendlyEffect);
                return;
            }
            if (ability.targeting.relation == content::TargetRelation::opposingTeam)
            {
                result += effectDescription(ability.hostileEffect);
                return;
            }

            const auto friendly = effectDescription(ability.friendlyEffect);
            const auto hostile = effectDescription(ability.hostileEffect);
            if (friendly == hostile)
            {
                result += friendly;
                return;
            }
            if (!friendly.empty())
            {
                append(result, "888888", "Auf Freunde");
                result += friendly;
            }
            if (!hostile.empty())
            {
                append(result, "888888", "Auf Gegner");
                result += hostile;
            }
        }

        void addStatusDetail(std::vector<AbilityStatusDetail>& details, AbilityStatusDetail detail)
        {
            const auto alreadyPresent = std::ranges::any_of(details, [&detail](const auto& existing) {
                return existing.status == detail.status;
            });
            if (!alreadyPresent)
            {
                details.push_back(std::move(detail));
            }
        }

        void collectStatusDetails(std::vector<AbilityStatusDetail>& details, const content::AbilityEffect& effect)
        {
            if (effect.confusionProbability != 0.0 || effect.decayRoundsAgainstConfused != 0)
                addStatusDetail(details,
                                {gameplay::StatusId{"status.confused"},
                                 "bb77bb",
                                 "Verwirrung",
                                 "Eine gewählte Fähigkeit trifft mit einer gewissen Wahrscheinlichkeit die andere Kämpfergruppe."});
            if (effect.decayRounds != 0 || effect.sleepFailureDecayRounds != 0 || effect.decayDamageAgainstMarked != 0 ||
                effect.decayRoundsAgainstConfused != 0 ||
                effect.decayRoundsAgainstSleeping != 0 || effect.ignoreDecayResistance || effect.inflictAllDecayAndRemove ||
                effect.transferDecayOnKill || effect.removeDecayAndHealParty || effect.transfusionHealing != 0)
                addStatusDetail(details,
                                {gameplay::StatusId{"status.decay"},
                                 "aa6644",
                                 "Verfall",
                                 "Verursacht eine gewisse Anzahl Runden Schaden zu Zugbeginn."});
            if (effect.markRounds != 0 || effect.damageAgainstMarked != 0.0 || effect.decayDamageAgainstMarked != 0 ||
                effect.panicAgainstMarked != 0 || effect.targetBuff.damageAgainstMarked != 0.0 ||
                effect.actorBuff.damageAgainstMarked != 0.0)
                addStatusDetail(details,
                                {gameplay::StatusId{"status.marked"},
                                 "ffaa00",
                                 "Markiert",
                                 "Keine eigene Wirkung. Aktiviert zusätzliche Effekte bestimmter Fähigkeiten."});
            if (effect.sleepProbability != 0.0 || effect.decayRoundsAgainstSleeping != 0 || effect.increaseSleepProbabilityByFatigue)
                addStatusDetail(details,
                                {gameplay::StatusId{"status.sleeping"},
                                 "7777cc",
                                 "Schlaf",
                                 "Überspringt den nächsten Zug; danach steigt die Schlafresistenz."});
            if (effect.addMiss != 0 || effect.addMissSelf != 0)
                addStatusDetail(details,
                                {gameplay::StatusId{"status.miss"},
                                 "ffffff",
                                 "Fehlschlag",
                                 "Lässt den nächsten feindlichen Angriff verfehlen."});
            if (effect.dynamite)
                addStatusDetail(details,
                                {gameplay::StatusId{"status.dynamite"},
                                 "ff5500",
                                 "Dynamit",
                                 "Explodiert mit einer gewissen Wahrscheinlichkeit am Anfang des Zuges. Falls nicht, wird es an den nächsten "
                                 "Kämpfer weitergegeben."});
            if (effect.bountyRounds != 0)
                addStatusDetail(details,
                                {gameplay::StatusId{"status.bounty"},
                                 "ffaa00",
                                 "Kopfgeld",
                                 "Treffer gewähren einem Angreifer vorübergehend +2 Schaden."});
            if (effect.riposteRounds != 0 || effect.riposteSelfRounds != 0)
                addStatusDetail(details,
                                {gameplay::StatusId{"status.riposte"},
                                 "e0c050",
                                 "Gegenschlag",
                                 "Kontert jeden feindlichen Angriff mit einem eigenen Angriff."});
            if (effect.rageRounds != 0)
                addStatusDetail(details,
                                {gameplay::StatusId{"status.rage"},
                                 "cc5533",
                                 "Raserei",
                                 "Erhöhter verursachter und erlittener Schaden: +50%."});
            if (effect.hackedRounds != 0)
                addStatusDetail(details,
                                {gameplay::StatusId{"status.hacked"}, "cc8888", "Gehackt", "Blockiert positive Stärkungen."});
            if (effect.transfusionHealing != 0)
                addStatusDetail(details,
                                {gameplay::StatusId{"status.transfusion"},
                                 "cc3355",
                                 "Transfusion",
                                 "Bei Schaden durch Verfall wird ein zufälliger Gegner geheilt; endet wenn kein Verfall mehr vorhanden."});
            if (effect.panicStacks != 0 || effect.panicAgainstMarked != 0 || effect.spreadPanicTargets != 0 || effect.damageFromPanic ||
                effect.doublePanic)
                addStatusDetail(details,
                                {gameplay::StatusId{"status.panic"},
                                 "cc6688",
                                 "Panik",
                                 "Je Stapel −5 Präzision; Schaden und negative Statuseffekte erhöhen die Panik. Heilung, positive Statuseffekte "
                                 "und der Zugbeginn reduzieren sie."});
        }
    }

    std::string abilityDescription(const content::AbilityDefinition& ability)
    {
        std::string result = "*#ffa500 " + ability.name + "* #888888 (" + targetSummary(ability.targeting) + ')';
        if (ability.fatigue != 0 && ability.fatigueTarget == content::FatigueTarget::actor)
        {
            result += " #7654a8 ";
            result += ability.fatigue > 0 ? "[arrow-up] " : "[arrow-down] ";
            result += std::to_string(std::abs(ability.fatigue));
        }
        std::vector<DescriptionItem> overview;
        if (ability.precisionModifier != 0)
        {
            addItem(overview,
                    "aaaadd",
                    "Präzision des Angriffs: " + signedInteger(ability.precisionModifier));
        }
        if (ability.targeting.requiresRevengeMarker)
        {
            addItem(overview, "ffaa00", "Nur gegen Gegner, die Markus zuvor angegriffen haben");
        }
        if (ability.fatigue != 0 && ability.fatigueTarget != content::FatigueTarget::actor)
        {
            addItem(overview, "7654a8", fatigueDescription(ability));
        }
        appendRow(result, overview);
        appendRelevantEffects(result, ability);
        if (ability.allCombatantsBuff.duration > 0)
        {
            append(result, "888888", "Anschließend auf alle Kämpfer");
            content::AbilityEffect globalEffect;
            globalEffect.targetBuff = ability.allCombatantsBuff;
            result += effectDescription(globalEffect);
        }
        return result;
    }

    std::vector<AbilityStatusDetail> abilityStatusDetails(const content::AbilityDefinition& ability)
    {
        std::vector<AbilityStatusDetail> details;
        forEachRelevantEffect(ability, [&details](const auto& effect) {
            collectStatusDetails(details, effect);
        });
        return details;
    }

    std::vector<AbilityStatusDetail> skillStatusDetails(const content::AbilityUpgrade& upgrade)
    {
        std::vector<AbilityStatusDetail> details;
        for (const auto& modifier : upgrade.effects)
        {
            collectStatusDetails(details, modifier.effect);
        }
        return details;
    }

    std::string fatigueDescription(const content::AbilityDefinition& ability)
    {
        if (ability.fatigue == 0)
        {
            return {};
        }

        switch (ability.fatigueTarget)
        {
        case content::FatigueTarget::actor:
            return fatigueChange("Anwender", ability.fatigue);
        case content::FatigueTarget::targets:
            return fatigueChange("Ziel", ability.fatigue);
        case content::FatigueTarget::allPlayers:
            return fatigueChange("Alle Spieler", ability.fatigue, true);
        case content::FatigueTarget::randomPlayer:
            return fatigueChange("Zufälliger Spieler", ability.fatigue);
        }
        return {};
    }
}
