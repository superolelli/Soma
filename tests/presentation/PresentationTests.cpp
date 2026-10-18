#include "presentation/battle/BattleEventPresentation.hpp"
#include "presentation/battle/BattlePresentation.hpp"
#include "presentation/battle/BattlePresentationSequence.hpp"
#include "presentation/sfml/BattleLaunchOptions.hpp"
#include "presentation/text/AbilityDescription.hpp"
#include "presentation/text/FormattedText.hpp"
#include "presentation/text/SkillDescription.hpp"

#include <cstdlib>
#include <iostream>
#include <ranges>
#include <source_location>

namespace
{
    void check(bool condition, const std::source_location& location = std::source_location::current())
    {
        if (!condition)
        {
            std::cerr << "Check failed at " << location.file_name() << ':' << location.line() << '\n';
            std::exit(EXIT_FAILURE);
        }
    }
} // namespace

int main()
{
    const soma::gameplay::EntityId simon{"player.simon"};
    const soma::gameplay::EntityId outlaw{"enemy.bang.outlaw"};
    const std::vector<soma::gameplay::GameplayEvent> events{
        soma::gameplay::AbilityUsed{simon, soma::gameplay::AbilityId{"ability.simon.basic_attack"}},
        soma::gameplay::DamageApplied{simon, outlaw, 7},
        soma::gameplay::CombatantDefeated{outlaw},
        soma::gameplay::BattleFinished{soma::gameplay::Team::players}};

    const auto cues = soma::presentation::BattlePresentation{}.map(events);

    check(cues.size() == 5);
    check(cues[0].kind == soma::presentation::CueKind::animation);
    check(cues[0].subject == simon);
    check(cues[1].subject == outlaw);
    check(cues[2].value == "-7");
    check(cues[3].value == "dying");
    check(cues[4].value == "Sieg!");
    const auto affected = soma::presentation::affectedCombatants(events);
    check(affected.size() == 1);
    check(affected.front() == outlaw);
    check(soma::presentation::isHostileImpact(outlaw, events));

    const std::vector<soma::gameplay::GameplayEvent> resistedEvents{
        soma::gameplay::EffectResisted{simon, soma::gameplay::StatusId{"status.confused"}}};
    const auto resistedCues = soma::presentation::BattlePresentation{}.map(resistedEvents);
    check(resistedCues.size() == 1);
    check(resistedCues.front().value == "Verwirrung widerstanden!");
    check(soma::presentation::resistedStatusNotification(soma::gameplay::StatusId{"status.sleeping"}) == "Schlaf widerstanden!");

    const soma::gameplay::EntityId secondTarget{"enemy.bang.second"};
    const soma::gameplay::EntityId thirdTarget{"enemy.bang.third"};
    const std::vector<soma::gameplay::GameplayEvent> groupEvents{soma::gameplay::DamageApplied{simon, outlaw, 3},
                                                                 soma::gameplay::DamageApplied{simon, secondTarget, 3},
                                                                 soma::gameplay::StatusGained{thirdTarget, soma::gameplay::StatusId{"confused"}}};
    const auto affectedGroup = soma::presentation::affectedCombatants(groupEvents);
    check(affectedGroup.size() == 3);
    check(std::ranges::find(affectedGroup, outlaw) != affectedGroup.end());
    check(std::ranges::find(affectedGroup, secondTarget) != affectedGroup.end());
    check(std::ranges::find(affectedGroup, thirdTarget) != affectedGroup.end());

    const auto groupedEvents = soma::presentation::groupedForPresentation(
        {soma::gameplay::DamageApplied{simon, outlaw, 3},
         soma::gameplay::StatusGained{outlaw, soma::gameplay::StatusId{"status.hacked"}},
         soma::gameplay::DamageApplied{simon, secondTarget, 4},
         soma::gameplay::StatusGained{secondTarget, soma::gameplay::StatusId{"status.hacked"}},
         soma::gameplay::StatusGained{simon, soma::gameplay::StatusId{"status.buffed"}}});
    check(std::holds_alternative<soma::gameplay::DamageApplied>(groupedEvents.at(0)));
    check(std::holds_alternative<soma::gameplay::DamageApplied>(groupedEvents.at(1)));
    check(std::get<soma::gameplay::StatusGained>(groupedEvents.at(2)).status == soma::gameplay::StatusId{"status.hacked"});
    check(std::get<soma::gameplay::StatusGained>(groupedEvents.at(3)).status == soma::gameplay::StatusId{"status.hacked"});
    check(std::get<soma::gameplay::StatusGained>(groupedEvents.at(4)).status == soma::gameplay::StatusId{"status.buffed"});

    soma::presentation::BattlePresentationSequence sequence;
    sequence.enqueue(events);
    check(!sequence.empty());
    check(sequence.current()->announcement == "Simon greift an");
    check(sequence.current()->animation == "all_on_one");

    sequence.update(1.4F);
    check(sequence.current()->healthChange == -7);
    check(sequence.current()->announcement == "-7 Leben");

    sequence.update(1.2F);
    check(sequence.current()->animation == "dying");

    const auto formatted = soma::presentation::parseFormattedText("*#ffa500 Ganzkörpertritt*\n#white Für 3 Runden");
    check(formatted.size() == 2);
    check(formatted.at(0).at(0).value == "Ganzkörpertritt");
    check(formatted.at(0).at(0).bold);
    check(formatted.at(0).at(0).color == soma::presentation::FormattedTextColor{255, 165, 0});

    const auto formattedSkill = soma::presentation::parseFormattedText(
        soma::presentation::skillDescription("Verbesserte Heilung", "Heilt 4 Leben mehr."));
    check(formattedSkill.size() == 2);
    check(formattedSkill.at(0).at(0).value == "Verbesserte Heilung");
    check(formattedSkill.at(0).at(0).bold);
    check(formattedSkill.at(0).at(0).color == soma::presentation::FormattedTextColor{255, 165, 0});
    check(formattedSkill.at(1).at(0).value == "Heilt 4 Leben mehr.");
    check(formatted.at(1).at(0).value == "Für 3 Runden");

    soma::content::AbilityDefinition fatigueAbility;
    fatigueAbility.name = "Müdigkeitstest";
    fatigueAbility.fatigue = 2;
    check(soma::presentation::fatigueDescription(fatigueAbility) == "Anwender erhält 2 Müdigkeit");
    const auto fatigueTooltip = soma::presentation::abilityDescription(fatigueAbility);
    check(fatigueTooltip.find("[arrow-up] 2") != std::string::npos);
    const auto formattedFatigue = soma::presentation::parseFormattedText(fatigueTooltip);
    check(std::ranges::any_of(formattedFatigue.front(), [](const auto& segment) {
        return segment.symbol == soma::presentation::FormattedTextSymbol::arrowUp;
    }));
    fatigueAbility.fatigue = -1;
    fatigueAbility.fatigueTarget = soma::content::FatigueTarget::targets;
    check(soma::presentation::fatigueDescription(fatigueAbility) == "Ziel verliert 1 Müdigkeit");
    fatigueAbility.fatigue = 3;
    fatigueAbility.fatigueTarget = soma::content::FatigueTarget::allPlayers;
    check(soma::presentation::fatigueDescription(fatigueAbility) == "Alle Spieler erhalten 3 Müdigkeit");

    soma::content::AbilityDefinition describedAbility;
    describedAbility.name = "Testangriff";
    describedAbility.targeting = {soma::content::TargetRelation::opposingTeam, soma::content::TargetScope::count, 2, false};
    describedAbility.precisionModifier = 20;
    describedAbility.friendlyEffect.damageFactor = 0.5;
    describedAbility.hostileEffect.damageFactor = 0.5;
    describedAbility.hostileEffect.decayDamage = 3;
    describedAbility.hostileEffect.decayRounds = 2;
    describedAbility.hostileEffect.panicStacks = 2;
    describedAbility.hostileEffect.transferredMissingHealthHealingFactor = 0.25;
    describedAbility.hostileEffect.successiveTargetDamageReduction = 0.75;
    describedAbility.hostileEffect.lessTargetsMoreDamage = 0.1;
    describedAbility.hostileEffect.criticalHitModifier = 5;
    describedAbility.hostileEffect.riposteRounds = 2;
    describedAbility.hostileEffect.rageRounds = 2;
    describedAbility.hostileEffect.actorBuff.duration = 2;
    describedAbility.hostileEffect.actorBuff.attributes.damageMinimum = 2;
    describedAbility.hostileEffect.actorBuff.attributes.damageMaximum = 2;
    describedAbility.hostileEffect.actorBuff.damagePerNegativeStatus = -0.05;
    const auto description = soma::presentation::abilityDescription(describedAbility);
    check(description.find("Testangriff") != std::string::npos);
    check(description.find("(2 Gegner)") != std::string::npos);
    check(description.find("Präzision des Angriffs: +20") != std::string::npos);
    check(description.find("Schaden: 50%") != std::string::npos);
    check(description.find("Zusätzlicher Schaden pro nicht getroffenem möglichen Ziel: 10%") != std::string::npos);
    check(description.find("Kritische Trefferchance: +5%") != std::string::npos);
    check(description.find("Gegenschlag: 2 Runden") != std::string::npos);
    check(description.find("Riposte") == std::string::npos);
    check(description.find("Auf Anwender für 2 Runden") != std::string::npos);
    check(description.find("Schaden: +2") != std::string::npos);
    check(description.find("Schaden (Minimum)") == std::string::npos);
    check(description.find("Schaden (Maximum)") == std::string::npos);
    check(description.find("Verfall: 3 Schaden / 2 Runden") != std::string::npos);

    check(description.find("Panik: +2") != std::string::npos);
    check(description.find("Schadensreduktion je weiterem Ziel: 75%") != std::string::npos);
    check(description.find("Heilung des nächsten Kämpfers: 25%") != std::string::npos);
    check(description.find("Schaden pro unterschiedlichem negativem Status") != std::string::npos);
    check(description.find("Raserei: 2 Runden") != std::string::npos);
    check(description.find("Rage") == std::string::npos);

    describedAbility.targeting = {soma::content::TargetRelation::ownTeam, soma::content::TargetScope::team, 1, false};
    check(soma::presentation::abilityDescription(describedAbility).find("(alle Freunde)") != std::string::npos);

    describedAbility.targeting = {soma::content::TargetRelation::opposingTeam, soma::content::TargetScope::single, 1, true};
    describedAbility.hostileEffect.criticalHitModifier = -15;
    const auto revengeDescription = soma::presentation::abilityDescription(describedAbility);
    check(revengeDescription.find("Markus zuvor angegriffen") != std::string::npos);
    check(revengeDescription.find("Kritische Trefferchance: -15%") != std::string::npos);
    check(revengeDescription.find("+-15%") == std::string::npos);

    soma::content::AbilityDefinition statusAbility;
    statusAbility.name = "Statustest";
    statusAbility.targeting = {soma::content::TargetRelation::opposingTeam, soma::content::TargetScope::single, 1, false};
    statusAbility.friendlyEffect.confusionProbability = 0.15;
    statusAbility.hostileEffect.confusionProbability = 0.45;
    statusAbility.hostileEffect.confusionRounds = 2;
    statusAbility.hostileEffect.hackedRounds = 2;
    statusAbility.hostileEffect.transfusionHealing = 3;
    const auto statusDescription = soma::presentation::abilityDescription(statusAbility);
    check(statusDescription.find("Auf Freunde") == std::string::npos);
    check(statusDescription.find("0 Runden") == std::string::npos);
    check(statusDescription.find("Transfusion: 3") != std::string::npos);
    check(statusDescription.find("Heilt bei Verfallsschaden") == std::string::npos);
    check(statusDescription.find("keine Stärkungen") == std::string::npos);

    const auto statusDetails = soma::presentation::abilityStatusDetails(statusAbility);
    const auto containsStatus = [&statusDetails](std::string_view name) {
        return std::ranges::any_of(statusDetails, [name](const auto& detail) { return detail.name == name; });
    };
    check(containsStatus("Verwirrung"));
    check(containsStatus("Gehackt"));
    check(containsStatus("Transfusion"));
    check(std::ranges::any_of(statusDetails, [](const auto& detail) {
        return detail.explanation.find("Blockiert positive Stärkungen") != std::string::npos;
    }));

    soma::content::AbilityUpgrade decaySkill;
    soma::content::SkillEffectModifier decayModifier;
    decayModifier.effect.sleepFailureDecayDamage = 2;
    decayModifier.effect.sleepFailureDecayRounds = 3;
    decaySkill.effects.push_back(decayModifier);
    const auto skillDetails = soma::presentation::skillStatusDetails(decaySkill);
    check(std::ranges::any_of(skillDetails, [](const auto& detail) { return detail.name == "Verfall"; }));

    char executable[] = "Soma.exe";
    char enemiesFlag[] = "--enemies";
    char escapedEnemy[] = "enemy.apache\\_kid";
    char autoFlag[] = "--auto";
    char seedFlag[] = "--seed";
    char seedValue[] = "123456";
    char* arguments[]{executable, enemiesFlag, escapedEnemy, autoFlag, seedFlag, seedValue};
    const auto options = soma::presentation::parseBattleLaunchOptions(6, arguments);
    check(options.automatic);
    check(options.randomSeed == 123456U);
    check(options.enemies.size() == 1);
    check(options.enemies.front() == soma::gameplay::ContentDefinitionId{"enemy.apache_kid"});
}
