#include "content/BattleContentLoader.hpp"
#include "content/RuleContentLoader.hpp"
#include "gameplay/random/SeededRandom.hpp"
#include "simulation/battle/AbilityTargets.hpp"
#include "simulation/battle/BattleSimulation.hpp"
#include "simulation/battle/EnemyPolicy.hpp"

#include <cstdlib>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <set>
#include <source_location>

namespace
{
    void check(bool condition, const std::source_location location = std::source_location::current())
    {
        if (!condition)
        {
            std::cerr << "Check failed at line " << location.line() << '\n';
            std::exit(1);
        }
    }

    soma::gameplay::EntityId entity(const std::string& definition, int slot)
    {
        return soma::gameplay::EntityId{definition + ".slot" + std::to_string(slot)};
    }
} // namespace

int main(int argc, char** argv)
{
    check(argc == 2);
    const auto loaded = soma::content::BattleContentLoader{}.load(std::filesystem::absolute(argv[1]));
    check(loaded.valid());
    const auto rules = soma::content::RuleContentLoader{}.load(std::filesystem::absolute(argv[1]));
    check(rules.valid());

    const auto gregId = soma::gameplay::ContentDefinitionId{"enemy.greg"};
    const auto& gregDefinition = loaded.content.combatants.at(gregId);
    soma::gameplay::SeededRandom bossRandom{0};
    soma::simulation::BattleSimulation bossBattle{
        loaded.content,
        {{soma::gameplay::ContentDefinitionId{"player.simon"}, 0},
         {gregId, 4, std::nullopt, {}, 0, loaded.content.bossAttributeMultiplier}},
        bossRandom};
    const auto* scaledBoss = bossBattle.find(entity("enemy.greg", 4));
    check(scaledBoss != nullptr);
    const auto scaled = [](int value) {
        return static_cast<int>(std::round(static_cast<double>(value) * 1.5));
    };
    check(scaledBoss->attributes.maximumHealth == scaled(gregDefinition.attributes.maximumHealth));
    check(scaledBoss->attributes.damageMinimum == scaled(gregDefinition.attributes.damageMinimum));
    check(scaledBoss->attributes.armour == scaled(gregDefinition.attributes.armour));
    check(scaledBoss->secondaryAttributes.sleepResistance == scaled(gregDefinition.resistances.sleepResistance));
    check(scaledBoss->health == scaledBoss->attributes.maximumHealth);
    check(scaledBoss->baseAttributes.maximumHealth == scaledBoss->attributes.maximumHealth);

    check(soma::gameplay::fatigueStage(8) == 0);
    check(soma::gameplay::fatigueStage(9) == 1);
    check(soma::gameplay::fatigueStage(17) == 1);
    check(soma::gameplay::fatigueStage(18) == 2);
    check(soma::gameplay::fatigueStage(26) == 2);
    check(soma::gameplay::fatigueStage(27) == 3);

    soma::gameplay::SeededRandom fatigueRandom{1};
    soma::simulation::BattleSimulation fatiguedBattle{
        loaded.content,
        {{soma::gameplay::ContentDefinitionId{"player.simon"}, 0, 4, {{soma::content::AttributeDelta{.armour = 5, .precision = 3}, 10, true}}, 40},
         {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}},
        fatigueRandom};
    check(fatiguedBattle.find(entity("player.simon", 0))->fatigue == 27);
    check(fatiguedBattle.find(entity("player.simon", 0))->health == 4);
    check(fatiguedBattle.find(entity("player.simon", 0))->attributes.armour ==
          loaded.content.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"}).attributes.armour + 5);
    check(fatiguedBattle.find(entity("player.simon", 0))->attributes.precision ==
          loaded.content.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"}).attributes.precision + 3 - 15);
    check(fatiguedBattle.find(entity("player.simon", 0))->buffs.front().remainingTurns == 10);

    const auto simon = soma::gameplay::ContentDefinitionId{"player.simon"};
    const auto baseMaximumHealth = loaded.content.combatants.at(simon).attributes.maximumHealth;
    soma::gameplay::SeededRandom maximumHealthRandom{2};
    soma::simulation::BattleSimulation maximumHealthBattle{
        loaded.content,
        {{simon, 0, baseMaximumHealth + 6, {{soma::content::AttributeDelta{.maximumHealth = 6}, 10, true}}},
         {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}},
        maximumHealthRandom};
    check(maximumHealthBattle.find(entity("player.simon", 0))->attributes.maximumHealth == baseMaximumHealth + 6);
    check(maximumHealthBattle.find(entity("player.simon", 0))->health == baseMaximumHealth + 6);

    soma::gameplay::SeededRandom initiativeRandom{3};
    soma::simulation::BattleSimulation initiativeBattle{
        loaded.content,
        {{soma::gameplay::ContentDefinitionId{"player.simon"}, 0},
         {soma::gameplay::ContentDefinitionId{"player.ole"}, 1},
         {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}},
        initiativeRandom};
    check(initiativeBattle.initiativeOrder() ==
          std::vector{soma::gameplay::EntityId{"player.ole.slot1"},
                      soma::gameplay::EntityId{"player.simon.slot0"},
                      soma::gameplay::EntityId{"enemy.gesetzloser.slot4"}});
    const soma::content::Targeting otherAllyOnly{
        soma::content::TargetRelation::ownTeam, soma::content::TargetScope::single, 1, false, true};
    check(!soma::simulation::isValidPrimaryTarget(
        otherAllyOnly, *initiativeBattle.find(entity("player.simon", 0)), *initiativeBattle.find(entity("player.simon", 0))));
    check(soma::simulation::isValidPrimaryTarget(
        otherAllyOnly, *initiativeBattle.find(entity("player.simon", 0)), *initiativeBattle.find(entity("player.ole", 1))));

    soma::gameplay::ConsumableItem initiativeBuff;
    initiativeBuff.duration = 2;
    initiativeBuff.temporaryBonuses.initiative = 5;
    check(initiativeBattle.useConsumable(entity("player.simon", 0), initiativeBuff).hasValue());
    check(initiativeBattle.initiativeOrder() ==
          std::vector{soma::gameplay::EntityId{"player.simon.slot0"},
                      soma::gameplay::EntityId{"player.ole.slot1"},
                      soma::gameplay::EntityId{"enemy.gesetzloser.slot4"}});

    auto initiativeDebuffContent = loaded.content;
    auto& initiativeDebuffAbility = initiativeDebuffContent.abilities.at(soma::gameplay::AbilityId{"ability.ole.0"});
    initiativeDebuffAbility.targeting = {soma::content::TargetRelation::ownTeam, soma::content::TargetScope::single, 1, false};
    initiativeDebuffAbility.friendlyEffect = {};
    initiativeDebuffAbility.friendlyEffect.targetBuff.duration = 2;
    initiativeDebuffAbility.friendlyEffect.targetBuff.positive = false;
    initiativeDebuffAbility.friendlyEffect.targetBuff.attributes.initiative = -5;
    initiativeDebuffAbility.hostileEffect = initiativeDebuffAbility.friendlyEffect;
    initiativeDebuffAbility.fatigue = 0;
    soma::gameplay::SeededRandom initiativeDebuffRandom{4};
    soma::simulation::BattleSimulation initiativeDebuffBattle{
        initiativeDebuffContent,
        {{soma::gameplay::ContentDefinitionId{"player.ole"}, 1},
         {soma::gameplay::ContentDefinitionId{"player.simon"}, 0},
         {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}},
        initiativeDebuffRandom};
    check(initiativeDebuffBattle.execute({entity("player.ole", 1),
                                          soma::gameplay::AbilityId{"ability.ole.0"},
                                          entity("player.simon", 0)})
              .hasValue());
    check(initiativeDebuffBattle.initiativeOrder() ==
          std::vector{soma::gameplay::EntityId{"player.ole.slot1"},
                      soma::gameplay::EntityId{"enemy.gesetzloser.slot4"},
                      soma::gameplay::EntityId{"player.simon.slot0"}});
    check(initiativeDebuffBattle.currentActor() == entity("enemy.gesetzloser", 4));
    check(initiativeDebuffBattle.remainingInitiativeOrder() ==
          std::vector{entity("enemy.gesetzloser", 4), entity("player.simon", 0)});

    soma::gameplay::SeededRandom wrapAroundRandom{31};
    soma::simulation::BattleSimulation wrapAroundBattle{
        loaded.content,
        {{soma::gameplay::ContentDefinitionId{"player.anna"}, 2},
         {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4},
         {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 6},
         {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 7}},
        wrapAroundRandom};
    const soma::content::Targeting threeTargets{
        soma::content::TargetRelation::opposingTeam, soma::content::TargetScope::count, 3, false};
    const auto* wrapAroundActor = wrapAroundBattle.find(entity("player.anna", 2));
    const auto* middlePrimary = wrapAroundBattle.find(entity("enemy.gesetzloser", 6));
    const auto middleTargets =
        soma::simulation::abilityTargets(threeTargets, *wrapAroundActor, *middlePrimary, wrapAroundBattle.combatants());
    check(middleTargets ==
          std::vector{entity("enemy.gesetzloser", 6), entity("enemy.gesetzloser", 7), entity("enemy.gesetzloser", 4)});

    const auto* lastPrimary = wrapAroundBattle.find(entity("enemy.gesetzloser", 7));
    const auto targetsAcrossEmptySlot =
        soma::simulation::abilityTargets(threeTargets, *wrapAroundActor, *lastPrimary, wrapAroundBattle.combatants());
    check(targetsAcrossEmptySlot == std::vector{entity("enemy.gesetzloser", 7), entity("enemy.gesetzloser", 4)});

    for (const auto& [definitionId, definition] : loaded.content.combatants)
    {
        if (definition.team != soma::gameplay::Team::enemies)
        {
            continue;
        }

        soma::gameplay::SeededRandom random{100U};
        soma::simulation::BattleSimulation battle{
            loaded.content, {{definitionId, 4}, {soma::gameplay::ContentDefinitionId{"player.simon"}, 0}}, random};
        soma::simulation::EnemyPolicy policy{random};
        const auto command = policy.choose(battle);
        check(command.hasValue());
        const auto result = battle.execute(command.value());
        check(result.hasValue());
        check(!result.value().empty());

        for (const auto& ability : definition.abilities)
        {
            soma::gameplay::SeededRandom abilityRandom{500U};
            soma::simulation::BattleSimulation abilityBattle{loaded.content,
                                                             {{definitionId, 4},
                                                              {definitionId, 5},
                                                              {definitionId, 6},
                                                              {definitionId, 7},
                                                              {soma::gameplay::ContentDefinitionId{"player.simon"}, 0},
                                                              {soma::gameplay::ContentDefinitionId{"player.simon"}, 1},
                                                              {soma::gameplay::ContentDefinitionId{"player.simon"}, 2},
                                                              {soma::gameplay::ContentDefinitionId{"player.simon"}, 3}},
                                                             abilityRandom};
            check(abilityBattle.currentActor() == entity(definitionId.value(), 4));
            const auto targets = abilityBattle.validPrimaryTargets(ability);
            if (targets.empty())
            {
                std::cerr << "No target for enemy ability " << ability.value() << '\n';
            }
            check(!targets.empty());
            const auto abilityResult = abilityBattle.execute({abilityBattle.currentActor(), ability, targets.front()});
            check(abilityResult.hasValue());
        }
    }

    const std::array<std::string, 4> players{"player.simon", "player.ole", "player.anna", "player.markus"};
    for (std::size_t playerIndex = 0; playerIndex < players.size(); ++playerIndex)
    {
        const auto playerId = soma::gameplay::ContentDefinitionId{players.at(playerIndex)};
        const auto& player = loaded.content.combatants.at(playerId);
        for (const auto& ability : player.abilities)
        {
            if (ability == soma::gameplay::AbilityId{"ability.markus.0"})
            {
                continue;
            }

            soma::gameplay::SeededRandom random{200U + static_cast<std::uint64_t>(playerIndex)};
            const auto fillerSlot = static_cast<int>((playerIndex + 1) % players.size());
            soma::simulation::BattleSimulation battle{loaded.content,
                                                      {{playerId, static_cast<int>(playerIndex)},
                                                       {soma::gameplay::ContentDefinitionId{"player.simon"}, fillerSlot},
                                                       {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4},
                                                       {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 5},
                                                       {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 6},
                                                       {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 7}},
                                                      random};
            check(battle.currentActor() == entity(players.at(playerIndex), static_cast<int>(playerIndex)));
            const auto targets = battle.validPrimaryTargets(ability);
            if (targets.empty())
            {
                std::cerr << "No target for " << ability.value() << '\n';
            }
            check(!targets.empty());
            const auto result = battle.execute({battle.currentActor(), ability, targets.front()});
            check(result.hasValue());
            const auto fatigue = loaded.content.abilities.at(ability).fatigue;
            const auto fatigueEvent = std::ranges::find_if(result.value(), [](const auto& event) {
                return std::holds_alternative<soma::gameplay::FatigueChanged>(event);
            });
            const auto expectedFatigue = std::clamp(fatigue, 0, soma::gameplay::maximumFatigue);
            check(battle.find(entity(players.at(playerIndex), static_cast<int>(playerIndex)))->fatigue == expectedFatigue);
            check(battle.find(entity("player.simon", fillerSlot))->fatigue == 0);
            check((expectedFatigue == 0) == (fatigueEvent == result.value().end()));
        }
    }

    soma::gameplay::SeededRandom revengeRandom{777};
    soma::simulation::BattleSimulation revengeBattle{
        loaded.content,
        {{soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}, {soma::gameplay::ContentDefinitionId{"player.markus"}, 0}},
        revengeRandom};
    const auto attack =
        revengeBattle.execute({entity("enemy.gesetzloser", 4), soma::gameplay::AbilityId{"ability.enemy.springfield"}, entity("player.markus", 0)});
    check(attack.hasValue());
    const auto revengeTargets = revengeBattle.validPrimaryTargets(soma::gameplay::AbilityId{"ability.markus.0"});
    check(revengeTargets.size() == 1);
    check(revengeTargets.front() == entity("enemy.gesetzloser", 4));
    const auto revenge = revengeBattle.execute({entity("player.markus", 0), soma::gameplay::AbilityId{"ability.markus.0"}, revengeTargets.front()});
    check(revenge.hasValue());
    check(std::ranges::count_if(revenge.value(), [](const auto& event) {
              return std::holds_alternative<soma::gameplay::DamageApplied>(event);
          }) <= 1);

    soma::gameplay::SeededRandom confusionRandom{889};
    soma::simulation::BattleSimulation confusedBattle{
        loaded.content,
        {{soma::gameplay::ContentDefinitionId{"enemy.tequila_joe"}, 4}, {soma::gameplay::ContentDefinitionId{"player.simon"}, 0, 10}},
        confusionRandom};
    const auto tequilaJoe = entity("enemy.tequila_joe", 4);
    const auto confusedTarget = entity("player.simon", 0);
    const auto simonBaseArmour = confusedBattle.find(confusedTarget)->attributes.armour;
    const auto tequilaJoeBaseArmour = confusedBattle.find(tequilaJoe)->attributes.armour;
    const_cast<soma::simulation::CombatantState*>(confusedBattle.find(tequilaJoe))->confusedThisTurn = true;
    const auto confusedBuff = confusedBattle.execute({tequilaJoe, soma::gameplay::AbilityId{"ability.enemy.beer"}, tequilaJoe});
    check(confusedBuff.hasValue());
    check(std::ranges::any_of(confusedBuff.value(), [&confusedTarget](const auto& event) {
        const auto* redirected = std::get_if<soma::gameplay::TargetRedirected>(&event);
        return redirected != nullptr && redirected->resolvedTarget == confusedTarget;
    }));
    check(confusedBattle.find(confusedTarget)->attributes.armour == simonBaseArmour + 5);
    check(confusedBattle.find(tequilaJoe)->attributes.armour == tequilaJoeBaseArmour);

    soma::gameplay::SeededRandom enemyFatigueRandom{894};
    soma::simulation::BattleSimulation enemyFatigueBattle{loaded.content,
                                                          {{soma::gameplay::ContentDefinitionId{"enemy.greg"}, 4},
                                                           {soma::gameplay::ContentDefinitionId{"player.simon"}, 0},
                                                           {soma::gameplay::ContentDefinitionId{"player.markus"}, 1}},
                                                          enemyFatigueRandom};
    const auto targetedFatigue =
        enemyFatigueBattle.execute({entity("enemy.greg", 4), soma::gameplay::AbilityId{"ability.enemy.gravedigger_gaze"}, entity("player.simon", 0)});
    check(targetedFatigue.hasValue());
    check(enemyFatigueBattle.find(entity("player.simon", 0))->fatigue == 2);
    check(enemyFatigueBattle.find(entity("player.markus", 1))->fatigue == 0);
    check(enemyFatigueBattle.find(entity("enemy.greg", 4))->fatigue == 0);

    soma::gameplay::SeededRandom partyFatigueRandom{895};
    soma::simulation::BattleSimulation partyFatigueBattle{loaded.content,
                                                          {{soma::gameplay::ContentDefinitionId{"enemy.theodora_krayenborg"}, 4},
                                                           {soma::gameplay::ContentDefinitionId{"player.simon"}, 0},
                                                           {soma::gameplay::ContentDefinitionId{"player.markus"}, 1}},
                                                          partyFatigueRandom};
    const auto partyFatigue = partyFatigueBattle.execute(
        {entity("enemy.theodora_krayenborg", 4), soma::gameplay::AbilityId{"ability.enemy.psychic"}, entity("enemy.theodora_krayenborg", 4)});
    check(partyFatigue.hasValue());
    check(partyFatigueBattle.find(entity("player.simon", 0))->fatigue == 1);
    check(partyFatigueBattle.find(entity("player.markus", 1))->fatigue == 1);
    check(partyFatigueBattle.find(entity("enemy.theodora_krayenborg", 4))->fatigue == 0);

    soma::gameplay::ComposedRules standingRules;
    standingRules.at(0).subject = soma::gameplay::RuleFragmentId{"rule.subject.all_full_health"};
    standingRules.at(0).modifier = soma::gameplay::RuleFragmentId{"rule.modifier.armour_up"};
    soma::gameplay::SeededRandom standingRandom{890};
    soma::simulation::BattleSimulation standingRuleBattle{
        loaded.content,
        {{soma::gameplay::ContentDefinitionId{"player.simon"}, 0}, {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}},
        standingRandom,
        &rules.content,
        &standingRules};
    check(standingRuleBattle.effectiveAttributes(*standingRuleBattle.find(entity("player.simon", 0))).armour ==
          loaded.content.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"}).attributes.armour + 10);
    check(standingRuleBattle.effectiveAttributes(*standingRuleBattle.find(entity("enemy.gesetzloser", 4))).armour ==
          loaded.content.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}).attributes.armour + 10);

    soma::gameplay::ComposedRules dynamicStandingRules;
    dynamicStandingRules.at(0).trigger = soma::gameplay::RuleFragmentId{"rule.trigger.turn_started"};
    dynamicStandingRules.at(0).subject = soma::gameplay::RuleFragmentId{"rule.subject.this_combatant"};
    dynamicStandingRules.at(0).effect = soma::gameplay::RuleFragmentId{"rule.effect.marked_three"};
    dynamicStandingRules.at(1).subject = soma::gameplay::RuleFragmentId{"rule.subject.all_marked"};
    dynamicStandingRules.at(1).modifier = soma::gameplay::RuleFragmentId{"rule.modifier.armour_up"};
    soma::gameplay::SeededRandom dynamicStandingRandom{897};
    soma::simulation::BattleSimulation dynamicStandingBattle{
        loaded.content,
        {{soma::gameplay::ContentDefinitionId{"player.simon"}, 0}, {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}},
        dynamicStandingRandom,
        &rules.content,
        &dynamicStandingRules};
    const auto dynamicallyModified = dynamicStandingBattle.currentActor();
    const auto baseDynamicArmour = dynamicStandingBattle.find(dynamicallyModified)->attributes.armour;
    check(dynamicStandingBattle.effectiveAttributes(*dynamicStandingBattle.find(dynamicallyModified)).armour == baseDynamicArmour);
    static_cast<void>(dynamicStandingBattle.begin());
    check(dynamicStandingBattle.effectiveAttributes(*dynamicStandingBattle.find(dynamicallyModified)).armour == baseDynamicArmour + 10);

    soma::gameplay::ComposedRules triggeredRules;
    triggeredRules.at(0).trigger = soma::gameplay::RuleFragmentId{"rule.trigger.health_lost"};
    triggeredRules.at(0).subject = soma::gameplay::RuleFragmentId{"rule.subject.this_combatant"};
    triggeredRules.at(0).effect = soma::gameplay::RuleFragmentId{"rule.effect.damage_one"};
    soma::gameplay::SeededRandom triggeredRandom{891};
    soma::simulation::BattleSimulation triggeredRuleBattle{
        loaded.content,
        {{soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}, {soma::gameplay::ContentDefinitionId{"player.simon"}, 0}},
        triggeredRandom,
        &rules.content,
        &triggeredRules};
    const auto triggeredResult = triggeredRuleBattle.execute(
        {entity("enemy.gesetzloser", 4), soma::gameplay::AbilityId{"ability.enemy.springfield"}, entity("player.simon", 0)});
    check(triggeredResult.hasValue());
    check(std::ranges::count_if(triggeredResult.value(), [](const auto& event) {
              return std::holds_alternative<soma::gameplay::RuleActivated>(event);
          }) == 1);
    check(std::ranges::count_if(triggeredResult.value(), [](const auto& event) {
              return std::holds_alternative<soma::gameplay::DamageApplied>(event);
          }) == 2);

    soma::gameplay::ComposedRules relativeSubjectRules;
    relativeSubjectRules.at(0).trigger = soma::gameplay::RuleFragmentId{"rule.trigger.health_lost"};
    relativeSubjectRules.at(0).subject = soma::gameplay::RuleFragmentId{"rule.subject.all_friends"};
    relativeSubjectRules.at(0).effect = soma::gameplay::RuleFragmentId{"rule.effect.damage_one"};
    soma::gameplay::SeededRandom relativeSubjectRandom{891};
    soma::simulation::BattleSimulation relativeSubjectBattle{loaded.content,
                                                             {{soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4},
                                                              {soma::gameplay::ContentDefinitionId{"player.simon"}, 0},
                                                              {soma::gameplay::ContentDefinitionId{"player.markus"}, 1}},
                                                             relativeSubjectRandom,
                                                             &rules.content,
                                                             &relativeSubjectRules};
    const auto relativeSubjectResult = relativeSubjectBattle.execute(
        {entity("enemy.gesetzloser", 4), soma::gameplay::AbilityId{"ability.enemy.springfield"}, entity("player.simon", 0)});
    check(relativeSubjectResult.hasValue());
    check(std::ranges::any_of(relativeSubjectResult.value(), [](const auto& event) {
        const auto* damage = std::get_if<soma::gameplay::DamageApplied>(&event);
        return damage != nullptr && damage->source == entity("player.simon", 0) && damage->target == entity("player.markus", 1);
    }));
    check(std::ranges::none_of(relativeSubjectResult.value(), [](const auto& event) {
        const auto* damage = std::get_if<soma::gameplay::DamageApplied>(&event);
        return damage != nullptr && damage->source == entity("player.simon", 0) && damage->target == entity("player.simon", 0);
    }));

    soma::gameplay::ComposedRules relativeOpponentRules;
    relativeOpponentRules.at(0).trigger = soma::gameplay::RuleFragmentId{"rule.trigger.health_lost"};
    relativeOpponentRules.at(0).subject = soma::gameplay::RuleFragmentId{"rule.subject.all_opponents"};
    relativeOpponentRules.at(0).effect = soma::gameplay::RuleFragmentId{"rule.effect.damage_one"};
    soma::gameplay::SeededRandom relativeOpponentRandom{891};
    soma::simulation::BattleSimulation relativeOpponentBattle{loaded.content,
                                                               {{soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4},
                                                                {soma::gameplay::ContentDefinitionId{"player.simon"}, 0}},
                                                               relativeOpponentRandom,
                                                               &rules.content,
                                                               &relativeOpponentRules};
    const auto relativeOpponentResult = relativeOpponentBattle.execute(
        {entity("enemy.gesetzloser", 4), soma::gameplay::AbilityId{"ability.enemy.springfield"}, entity("player.simon", 0)});
    check(relativeOpponentResult.hasValue());
    check(std::ranges::any_of(relativeOpponentResult.value(), [](const auto& event) {
        const auto* damage = std::get_if<soma::gameplay::DamageApplied>(&event);
        return damage != nullptr && damage->source == entity("player.simon", 0) && damage->target == entity("enemy.gesetzloser", 4);
    }));

    soma::gameplay::SeededRandom multiTargetRuleRandom{893};
    soma::simulation::BattleSimulation multiTargetRuleBattle{loaded.content,
                                                             {{soma::gameplay::ContentDefinitionId{"enemy.bill_noface"}, 4},
                                                              {soma::gameplay::ContentDefinitionId{"player.simon"}, 0},
                                                              {soma::gameplay::ContentDefinitionId{"player.markus"}, 1}},
                                                             multiTargetRuleRandom,
                                                             &rules.content,
                                                             &triggeredRules};
    const auto multiTargetResult = multiTargetRuleBattle.execute(
        {entity("enemy.bill_noface", 4), soma::gameplay::AbilityId{"ability.enemy.shotgun"}, entity("player.simon", 0)});
    check(multiTargetResult.hasValue());
    check(std::ranges::count_if(multiTargetResult.value(), [](const auto& event) {
              return std::holds_alternative<soma::gameplay::RuleActivated>(event);
          }) == 2);
    check(std::ranges::count_if(multiTargetResult.value(), [](const auto& event) {
              return std::holds_alternative<soma::gameplay::DamageApplied>(event);
          }) == 4);

    soma::gameplay::ComposedRules turnRules;
    turnRules.at(0).trigger = soma::gameplay::RuleFragmentId{"rule.trigger.turn_started"};
    turnRules.at(0).subject = soma::gameplay::RuleFragmentId{"rule.subject.this_combatant"};
    turnRules.at(0).effect = soma::gameplay::RuleFragmentId{"rule.effect.damage_one"};
    soma::gameplay::SeededRandom turnRandom{892};
    soma::simulation::BattleSimulation turnRuleBattle{
        loaded.content,
        {{soma::gameplay::ContentDefinitionId{"player.anna"}, 2}, {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}},
        turnRandom,
        &rules.content,
        &turnRules};
    const auto initialTurnEvents = turnRuleBattle.begin();
    check(std::ranges::count_if(initialTurnEvents, [](const auto& event) {
              return std::holds_alternative<soma::gameplay::RuleActivated>(event);
          }) == 1);
    check(turnRuleBattle.find(entity("player.anna", 2))->health ==
          loaded.content.combatants.at(soma::gameplay::ContentDefinitionId{"player.anna"}).attributes.maximumHealth - 1);

    soma::gameplay::ComposedRules fatigueRules;
    fatigueRules.at(0).trigger = soma::gameplay::RuleFragmentId{"rule.trigger.fatigue_reached_nine"};
    fatigueRules.at(0).subject = soma::gameplay::RuleFragmentId{"rule.subject.this_combatant"};
    fatigueRules.at(0).effect = soma::gameplay::RuleFragmentId{"rule.effect.lose_one_fatigue"};
    soma::gameplay::SeededRandom fatigueRuleRandom{896};
    soma::simulation::BattleSimulation fatigueRuleBattle{
        loaded.content,
        {{soma::gameplay::ContentDefinitionId{"player.anna"}, 2, std::nullopt, {}, 8}, {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 7}},
        fatigueRuleRandom,
        &rules.content,
        &fatigueRules};
    const auto fatigueRuleResult =
        fatigueRuleBattle.execute({entity("player.anna", 2), soma::gameplay::AbilityId{"ability.anna.2"}, entity("enemy.gesetzloser", 7)});
    check(fatigueRuleResult.hasValue());
    check(fatigueRuleBattle.find(entity("player.anna", 2))->fatigue == 8);
    check(std::ranges::count_if(fatigueRuleResult.value(), [](const auto& event) {
              return std::holds_alternative<soma::gameplay::RuleActivated>(event);
          }) == 1);

    soma::gameplay::ComposedRules decayDamageRules;
    decayDamageRules.at(0).trigger = soma::gameplay::RuleFragmentId{"rule.trigger.decay_damage_received"};
    decayDamageRules.at(0).subject = soma::gameplay::RuleFragmentId{"rule.subject.this_combatant"};
    decayDamageRules.at(0).effect = soma::gameplay::RuleFragmentId{"rule.effect.rage_two"};
    auto decayTriggerContent = loaded.content;
    decayTriggerContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}).attributes.initiative = 100;
    decayTriggerContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"}).attributes.initiative = 0;
    soma::gameplay::SeededRandom decayDamageRandom{898};
    soma::simulation::BattleSimulation decayDamageBattle{
        decayTriggerContent,
        {{soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}, {soma::gameplay::ContentDefinitionId{"player.simon"}, 0}},
        decayDamageRandom,
        &rules.content,
        &decayDamageRules};
    const auto decayingPlayer = entity("player.simon", 0);
    const_cast<soma::simulation::CombatantState*>(decayDamageBattle.find(decayingPlayer))->decay.push_back({2, 1});
    const auto decayDamageResult = decayDamageBattle.passTurn(entity("enemy.gesetzloser", 4));
    check(decayDamageResult.hasValue());
    check(decayDamageBattle.find(decayingPlayer)->rageTurns == 2);
    check(std::ranges::any_of(decayDamageResult.value(), [](const auto& event) {
        return std::holds_alternative<soma::gameplay::RuleActivated>(event);
    }));

    soma::gameplay::SeededRandom passRandom{888};
    soma::simulation::BattleSimulation passBattle{
        loaded.content,
        {{soma::gameplay::ContentDefinitionId{"player.anna"}, 2}, {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}},
        passRandom};
    const auto passingPlayer = passBattle.currentActor();
    const auto passResult = passBattle.passTurn(passingPlayer);
    check(passResult.hasValue());
    check(passBattle.currentActor() != passingPlayer);

    auto statusContent = loaded.content;
    auto& statusAbility = statusContent.abilities.at(soma::gameplay::AbilityId{"ability.simon.0"});
    statusAbility.targeting = {soma::content::TargetRelation::self, soma::content::TargetScope::single, 1, false};
    statusAbility.friendlyEffect = {};
    statusAbility.friendlyEffect.riposteRounds = 3;
    statusAbility.friendlyEffect.rageRounds = 3;
    statusAbility.hostileEffect = statusAbility.friendlyEffect;
    statusAbility.fatigue = 0;
    soma::gameplay::SeededRandom statusRandom{998};
    soma::simulation::BattleSimulation statusBattle{
        statusContent,
        {{soma::gameplay::ContentDefinitionId{"player.simon"}, 0}, {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}},
        statusRandom};
    const auto statusResult = statusBattle.execute(
        {entity("player.simon", 0), soma::gameplay::AbilityId{"ability.simon.0"}, entity("player.simon", 0)});
    check(statusResult.hasValue());
    check(statusBattle.find(entity("player.simon", 0))->riposteTurns == 3);
    check(statusBattle.find(entity("player.simon", 0))->rageTurns == 3);
    check(statusBattle.modifiedDamage(*statusBattle.find(entity("player.simon", 0)),
                                      *statusBattle.find(entity("enemy.gesetzloser", 4)),
                                      10) == 15);
    check(statusBattle.modifiedDamage(*statusBattle.find(entity("enemy.gesetzloser", 4)),
                                      *statusBattle.find(entity("player.simon", 0)),
                                      10) == 15);
    const auto counterResult = statusBattle.execute({entity("enemy.gesetzloser", 4),
                                                     soma::gameplay::AbilityId{"ability.enemy.springfield"},
                                                     entity("player.simon", 0)});
    check(counterResult.hasValue());
    check(std::ranges::any_of(counterResult.value(), [](const auto& event) {
        const auto* status = std::get_if<soma::gameplay::StatusTriggered>(&event);
        return status != nullptr && status->status == soma::gameplay::StatusId{"status.riposte"};
    }));
    check(std::ranges::any_of(counterResult.value(), [](const auto& event) {
        const auto* damage = std::get_if<soma::gameplay::DamageApplied>(&event);
        return damage != nullptr && damage->source == entity("player.simon", 0) && damage->target == entity("enemy.gesetzloser", 4);
    }));

    auto neutralRageContent = loaded.content;
    auto& neutralRageAbility = neutralRageContent.abilities.at(soma::gameplay::AbilityId{"ability.simon.0"});
    neutralRageAbility.targeting = {soma::content::TargetRelation::self, soma::content::TargetScope::single, 1, false};
    neutralRageAbility.friendlyEffect = {};
    neutralRageAbility.friendlyEffect.rageRounds = 3;
    neutralRageAbility.friendlyEffect.removeBuffs = true;
    neutralRageAbility.fatigue = 0;
    soma::gameplay::SeededRandom neutralRageRandom{999};
    soma::simulation::BattleSimulation neutralRageBattle{
        neutralRageContent,
        {{soma::gameplay::ContentDefinitionId{"player.simon"}, 0}, {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}},
        neutralRageRandom};
    const auto neutralRageResult = neutralRageBattle.execute(
        {entity("player.simon", 0), soma::gameplay::AbilityId{"ability.simon.0"}, entity("player.simon", 0)});
    check(neutralRageResult.hasValue());
    check(neutralRageBattle.find(entity("player.simon", 0))->rageTurns == 3);
    check(std::ranges::none_of(neutralRageResult.value(), [](const auto& event) {
        return std::holds_alternative<soma::gameplay::BuffsRemoved>(event);
    }));

    auto skillEffectContent = loaded.content;
    auto& skilledSimon = skillEffectContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"});
    skilledSimon.attributes.damageMinimum = 10;
    skilledSimon.attributes.damageMaximum = 10;
    skilledSimon.attributes.initiative = 100;
    skilledSimon.attributes.criticalHitChance = 0;
    skilledSimon.attributes.precision = 100;
    auto& durableEnemy = skillEffectContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"});
    durableEnemy.attributes.maximumHealth = 100;
    durableEnemy.attributes.armour = 0;
    durableEnemy.attributes.dodge = 0;
    durableEnemy.attributes.initiative = 0;
    auto& markedAttack = skillEffectContent.abilities.at(soma::gameplay::AbilityId{"ability.simon.2"});
    markedAttack.targeting = {soma::content::TargetRelation::opposingTeam, soma::content::TargetScope::single, 1, false};
    markedAttack.friendlyEffect = {};
    markedAttack.hostileEffect = {};
    markedAttack.hostileEffect.damageFactor = 0.5;
    markedAttack.hostileEffect.damageAgainstMarked = 0.4;
    markedAttack.allCombatantsBuff.duration = 4;
    markedAttack.allCombatantsBuff.damageAgainstMarked = 0.2;
    markedAttack.fatigue = 0;
    soma::gameplay::SeededRandom skillEffectRandom{1000};
    soma::simulation::BattleSimulation skillEffectBattle{
        skillEffectContent,
        {{soma::gameplay::ContentDefinitionId{"player.simon"}, 0}, {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}},
        skillEffectRandom};
    const auto markedEnemy = entity("enemy.gesetzloser", 4);
    const_cast<soma::simulation::CombatantState*>(skillEffectBattle.find(markedEnemy))->markedTurns = 5;
    const auto firstMarkedAttack =
        skillEffectBattle.execute({entity("player.simon", 0), soma::gameplay::AbilityId{"ability.simon.2"}, markedEnemy});
    check(firstMarkedAttack.hasValue());
    const auto firstDamage = std::ranges::find_if(firstMarkedAttack.value(), [](const auto& event) {
        const auto* damage = std::get_if<soma::gameplay::DamageApplied>(&event);
        return damage != nullptr && damage->source == entity("player.simon", 0);
    });
    check(firstDamage != firstMarkedAttack.value().end());
    check(std::get<soma::gameplay::DamageApplied>(*firstDamage).amount == 9);
    check(skillEffectBattle.find(entity("player.simon", 0))->buffs.back().damageAgainstMarked == 0.2);
    check(skillEffectBattle.find(markedEnemy)->buffs.back().damageAgainstMarked == 0.2);
    check(skillEffectBattle.modifiedDamage(*skillEffectBattle.find(entity("player.simon", 0)), *skillEffectBattle.find(markedEnemy), 10) == 10);
    check(skillEffectBattle.passTurn(markedEnemy).hasValue());
    const auto subsequentMarkedAttack =
        skillEffectBattle.execute({entity("player.simon", 0), soma::gameplay::AbilityId{"ability.simon.2"}, markedEnemy});
    check(subsequentMarkedAttack.hasValue());
    const auto subsequentDamage = std::ranges::find_if(subsequentMarkedAttack.value(), [](const auto& event) {
        const auto* damage = std::get_if<soma::gameplay::DamageApplied>(&event);
        return damage != nullptr && damage->source == entity("player.simon", 0);
    });
    check(subsequentDamage != subsequentMarkedAttack.value().end());
    check(std::get<soma::gameplay::DamageApplied>(*subsequentDamage).amount == 11);

    auto healingModifierContent = loaded.content;
    auto& healingSimon = healingModifierContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"});
    healingSimon.attributes.initiative = 100;
    healingSimon.attributes.criticalHitChance = 0;
    auto& healingOle = healingModifierContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.ole"});
    healingOle.attributes.initiative = 0;
    auto& targetedHealing = healingModifierContent.abilities.at(soma::gameplay::AbilityId{"ability.simon.2"});
    targetedHealing.targeting = {soma::content::TargetRelation::ownTeam, soma::content::TargetScope::single, 1, false};
    targetedHealing.friendlyEffect = {};
    targetedHealing.friendlyEffect.heal = 3;
    targetedHealing.friendlyEffect.targetFatigue = -3;
    targetedHealing.hostileEffect = targetedHealing.friendlyEffect;
    targetedHealing.fatigue = 0;
    soma::simulation::ActiveBuff healingReceivedBuff;
    healingReceivedBuff.remainingTurns = 5;
    healingReceivedBuff.positive = true;
    healingReceivedBuff.healingReceived = 0.25;
    soma::gameplay::SeededRandom healingModifierRandom{1001};
    soma::simulation::BattleSimulation healingModifierBattle{
        healingModifierContent,
        {{soma::gameplay::ContentDefinitionId{"player.simon"}, 0},
         {soma::gameplay::ContentDefinitionId{"player.ole"}, 1, 5, {healingReceivedBuff}, 5},
         {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}},
        healingModifierRandom};
    const auto healedOle = entity("player.ole", 1);
    const auto targetedHealingResult =
        healingModifierBattle.execute({entity("player.simon", 0), soma::gameplay::AbilityId{"ability.simon.2"}, healedOle});
    check(targetedHealingResult.hasValue());
    check(healingModifierBattle.find(healedOle)->health == 9);
    check(healingModifierBattle.find(healedOle)->fatigue == 2);
    soma::gameplay::ConsumableItem smallHealingConsumable;
    smallHealingConsumable.healing = 3;
    check(healingModifierBattle.useConsumable(healedOle, smallHealingConsumable).hasValue());
    check(healingModifierBattle.find(healedOle)->health == 13);

    auto wakeResistanceContent = loaded.content;
    auto& wakingPlayerDefinition = wakeResistanceContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"});
    wakingPlayerDefinition.attributes.initiative = 0;
    wakingPlayerDefinition.attributes.dodge = 0;
    wakingPlayerDefinition.resistances.sleepResistance = 60;
    auto& wakingEnemyDefinition = wakeResistanceContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"});
    wakingEnemyDefinition.attributes.initiative = 100;
    wakingEnemyDefinition.attributes.precision = 100;
    wakingEnemyDefinition.attributes.criticalHitChance = 0;
    auto& sleepAttack = wakeResistanceContent.abilities.at(soma::gameplay::AbilityId{"ability.enemy.springfield"});
    sleepAttack.hostileEffect = {};
    sleepAttack.hostileEffect.sleepProbability = 1.0;
    sleepAttack.fatigue = 0;
    soma::gameplay::SeededRandom wakeResistanceRandom{1002};
    soma::simulation::BattleSimulation wakeResistanceBattle{
        wakeResistanceContent,
        {{soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}, {soma::gameplay::ContentDefinitionId{"player.simon"}, 0}},
        wakeResistanceRandom};
    const auto wakingPlayer = entity("player.simon", 0);
    const auto wakingEnemy = entity("enemy.gesetzloser", 4);
    const_cast<soma::simulation::CombatantState*>(wakeResistanceBattle.find(wakingPlayer))->sleeping = true;
    const auto wakeEvents = wakeResistanceBattle.passTurn(wakingEnemy);
    check(wakeEvents.hasValue());
    check(!wakeResistanceBattle.find(wakingPlayer)->sleeping);
    check(wakeResistanceBattle.find(wakingPlayer)->secondaryAttributes.sleepResistance == 100);
    check(wakeResistanceBattle.find(wakingPlayer)->buffs.size() == 1);
    check(wakeResistanceBattle.find(wakingPlayer)->buffs.front().remainingTurns == 3);
    check(wakeResistanceBattle.find(wakingPlayer)->buffs.front().attributes.sleepResistance == 40);
    const auto resistedSleep = wakeResistanceBattle.execute(
        {wakingEnemy, soma::gameplay::AbilityId{"ability.enemy.springfield"}, wakingPlayer});
    check(resistedSleep.hasValue());
    check(std::ranges::any_of(resistedSleep.value(), [&wakingPlayer](const auto& event) {
        const auto* resisted = std::get_if<soma::gameplay::EffectResisted>(&event);
        return resisted != nullptr && resisted->combatant == wakingPlayer &&
               resisted->status == soma::gameplay::StatusId{"status.sleeping"};
    }));

    auto hackedContent = loaded.content;
    auto& hackingEnemy = hackedContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"});
    hackingEnemy.attributes.initiative = 100;
    hackingEnemy.attributes.precision = 100;
    hackingEnemy.abilities.push_back(soma::gameplay::AbilityId{"ability.ole.2"});
    auto& hackedPlayer = hackedContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"});
    hackedPlayer.attributes.initiative = 0;
    hackedPlayer.attributes.dodge = 0;
    hackedPlayer.resistances.debuffResistance = 0;
    auto& hackingAbility = hackedContent.abilities.at(soma::gameplay::AbilityId{"ability.ole.2"});
    hackingAbility.targeting = {soma::content::TargetRelation::opposingTeam, soma::content::TargetScope::single, 1, false};
    hackingAbility.friendlyEffect = {};
    hackingAbility.hostileEffect = {};
    hackingAbility.hostileEffect.hackedRounds = 2;
    soma::gameplay::SeededRandom hackedRandom{1003};
    soma::simulation::BattleSimulation hackedBattle{
        hackedContent,
        {{soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}, {soma::gameplay::ContentDefinitionId{"player.simon"}, 0}},
        hackedRandom};
    const auto hackedTarget = entity("player.simon", 0);
    const auto hackedResult =
        hackedBattle.execute({entity("enemy.gesetzloser", 4), soma::gameplay::AbilityId{"ability.ole.2"}, hackedTarget});
    check(hackedResult.hasValue());
    check(hackedBattle.find(hackedTarget)->hackedTurns == 1);
    const auto armourBeforeBlockedBuff = hackedBattle.find(hackedTarget)->attributes.armour;
    soma::gameplay::ConsumableItem blockedBuff;
    blockedBuff.duration = 2;
    blockedBuff.temporaryBonuses.armour = 10;
    const auto blockedBuffResult = hackedBattle.useConsumable(hackedTarget, blockedBuff);
    check(blockedBuffResult.hasValue());
    check(hackedBattle.find(hackedTarget)->attributes.armour == armourBeforeBlockedBuff);
    check(std::ranges::any_of(blockedBuffResult.value(), [](const auto& event) {
        const auto* triggered = std::get_if<soma::gameplay::StatusTriggered>(&event);
        return triggered != nullptr && triggered->status == soma::gameplay::StatusId{"status.hacked"};
    }));

    auto oleMechanicsContent = loaded.content;
    auto& mechanicsOle = oleMechanicsContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.ole"});
    mechanicsOle.attributes.initiative = 100;
    mechanicsOle.attributes.damageMinimum = 10;
    mechanicsOle.attributes.damageMaximum = 10;
    mechanicsOle.attributes.criticalHitChance = 0;
    mechanicsOle.attributes.precision = 100;
    auto& mechanicsEnemy = oleMechanicsContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"});
    mechanicsEnemy.attributes.initiative = 0;
    mechanicsEnemy.attributes.maximumHealth = 100;
    mechanicsEnemy.attributes.armour = 0;
    mechanicsEnemy.attributes.dodge = 0;
    auto& acceleratedDecay = oleMechanicsContent.abilities.at(soma::gameplay::AbilityId{"ability.ole.0"});
    acceleratedDecay.targeting = {soma::content::TargetRelation::opposingTeam, soma::content::TargetScope::single, 1, false};
    acceleratedDecay.hostileEffect = {};
    acceleratedDecay.hostileEffect.damageFactor = 0.5;
    acceleratedDecay.hostileEffect.inflictAllDecayAndRemove = true;
    acceleratedDecay.fatigue = 0;
    soma::simulation::ActiveBuff fatigueConversion;
    fatigueConversion.remainingTurns = 3;
    fatigueConversion.positive = true;
    fatigueConversion.damageFromFatigue = true;
    fatigueConversion.blocksFatigueStageOne = true;
    fatigueConversion.blocksFatigueStageTwo = true;
    soma::gameplay::SeededRandom oleMechanicsRandom{1004};
    soma::simulation::BattleSimulation oleMechanicsBattle{
        oleMechanicsContent,
        {{soma::gameplay::ContentDefinitionId{"player.ole"}, 0, std::nullopt, {fatigueConversion}, 27},
         {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}},
        oleMechanicsRandom};
    const auto mechanicsTarget = entity("enemy.gesetzloser", 4);
    auto* mutableMechanicsTarget = const_cast<soma::simulation::CombatantState*>(oleMechanicsBattle.find(mechanicsTarget));
    mutableMechanicsTarget->decay.push_back({3, 2});
    check(oleMechanicsBattle.find(entity("player.ole", 0))->attributes.damageMinimum == 7);
    check(oleMechanicsBattle.find(entity("player.ole", 0))->appliedFatiguePenaltyStages == 4);
    const auto acceleratedResult =
        oleMechanicsBattle.execute({entity("player.ole", 0), soma::gameplay::AbilityId{"ability.ole.0"}, mechanicsTarget});
    check(acceleratedResult.hasValue());
    check(oleMechanicsBattle.find(mechanicsTarget)->health == 84);
    check(oleMechanicsBattle.find(mechanicsTarget)->decay.empty());
    check(std::ranges::count_if(acceleratedResult.value(), [](const auto& event) {
              return std::holds_alternative<soma::gameplay::DamageApplied>(event);
          }) == 2);
    check(std::ranges::any_of(acceleratedResult.value(), [](const auto& event) {
        return std::holds_alternative<soma::gameplay::DecayDetonated>(event);
    }));

    auto failedSleepContent = loaded.content;
    auto& sleepyOle = failedSleepContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.ole"});
    sleepyOle.attributes.initiative = 100;
    sleepyOle.attributes.precision = 100;
    sleepyOle.attributes.criticalHitChance = 0;
    sleepyOle.abilities.push_back(soma::gameplay::AbilityId{"ability.ole.3.alternative"});
    auto& sleepyEnemy = failedSleepContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"});
    sleepyEnemy.attributes.initiative = 0;
    sleepyEnemy.attributes.dodge = 0;
    sleepyEnemy.resistances.decayResistance = 0;
    auto& failedSleep = failedSleepContent.abilities.at(soma::gameplay::AbilityId{"ability.ole.3.alternative"});
    failedSleep.targeting = {soma::content::TargetRelation::opposingTeam, soma::content::TargetScope::single, 1, false};
    failedSleep.hostileEffect = {};
    failedSleep.hostileEffect.sleepProbability = 0.000001;
    failedSleep.hostileEffect.sleepFailureDecayDamage = 1;
    failedSleep.hostileEffect.sleepFailureDecayRounds = 4;
    failedSleep.fatigue = 0;
    soma::gameplay::SeededRandom failedSleepRandom{1005};
    soma::simulation::BattleSimulation failedSleepBattle{
        failedSleepContent,
        {{soma::gameplay::ContentDefinitionId{"player.ole"}, 0}, {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}},
        failedSleepRandom};
    const auto failedSleepTarget = entity("enemy.gesetzloser", 4);
    const auto failedSleepResult =
        failedSleepBattle.execute({entity("player.ole", 0), soma::gameplay::AbilityId{"ability.ole.3.alternative"}, failedSleepTarget});
    check(failedSleepResult.hasValue());
    check(!failedSleepBattle.find(failedSleepTarget)->decay.empty());
    check(failedSleepBattle.find(failedSleepTarget)->decay.front().remainingTurns == 3);
    check(failedSleepBattle.find(failedSleepTarget)->decay.front().damage == 1);

    auto transfusionContent = loaded.content;
    transfusionContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"}).attributes.initiative = 100;
    transfusionContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}).attributes.initiative = 0;
    soma::gameplay::SeededRandom transfusionRandom{1006};
    soma::simulation::BattleSimulation transfusionBattle{
        transfusionContent,
        {{soma::gameplay::ContentDefinitionId{"player.simon"}, 0},
         {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}},
        transfusionRandom};
    auto* transfusionRecipient =
        const_cast<soma::simulation::CombatantState*>(transfusionBattle.find(entity("player.simon", 0)));
    auto* transfusionSource =
        const_cast<soma::simulation::CombatantState*>(transfusionBattle.find(entity("enemy.gesetzloser", 4)));
    transfusionRecipient->health = 5;
    transfusionSource->decay.push_back({1, 1});
    transfusionSource->transfusions.push_back({1});
    transfusionSource->transfusions.push_back({2});
    const auto transfusionResult = transfusionBattle.passTurn(transfusionRecipient->id);
    check(transfusionResult.hasValue());
    check(transfusionRecipient->health == 8);
    check(transfusionSource->decay.empty());
    check(transfusionSource->transfusions.empty());
    check(std::ranges::count_if(transfusionResult.value(), [](const auto& event) {
              const auto* triggered = std::get_if<soma::gameplay::StatusTriggered>(&event);
              return triggered != nullptr && triggered->status == soma::gameplay::StatusId{"status.transfusion"};
          }) == 2);
    check(std::ranges::count_if(transfusionResult.value(), [](const auto& event) {
              const auto* expired = std::get_if<soma::gameplay::StatusExpired>(&event);
              return expired != nullptr && expired->status == soma::gameplay::StatusId{"status.transfusion"};
          }) == 1);

    auto conditionalDecayContent = loaded.content;
    auto& conditionalAnna = conditionalDecayContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.anna"});
    conditionalAnna.attributes.initiative = 100;
    conditionalAnna.attributes.criticalHitChance = 100;
    auto& conditionalEnemy = conditionalDecayContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"});
    conditionalEnemy.attributes.initiative = 0;
    conditionalEnemy.resistances.decayResistance = 0;
    auto& conditionalDecay = conditionalDecayContent.abilities.at(soma::gameplay::AbilityId{"ability.anna.2"});
    conditionalDecay.hostileEffect = {};
    conditionalDecay.hostileEffect.decayDamage = 2;
    conditionalDecay.hostileEffect.decayRounds = 3;
    conditionalDecay.hostileEffect.decayDamageAgainstMarked = 1;
    conditionalDecay.hostileEffect.decayRoundsAgainstConfused = 2;
    conditionalDecay.hostileEffect.decayRoundsAgainstSleeping = 3;
    conditionalDecay.hostileEffect.targetBuff.duration = 4;
    conditionalDecay.hostileEffect.targetBuff.attributes.armour = 1;
    conditionalDecay.fatigue = 0;
    soma::gameplay::SeededRandom conditionalDecayRandom{1007};
    soma::simulation::BattleSimulation conditionalDecayBattle{
        conditionalDecayContent,
        {{soma::gameplay::ContentDefinitionId{"player.anna"}, 2},
         {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}},
        conditionalDecayRandom};
    auto* conditionalTarget =
        const_cast<soma::simulation::CombatantState*>(conditionalDecayBattle.find(entity("enemy.gesetzloser", 4)));
    conditionalTarget->markedTurns = 2;
    conditionalTarget->confusedTurns = 2;
    conditionalTarget->sleeping = true;
    const auto conditionalResult = conditionalDecayBattle.execute(
        {entity("player.anna", 2), soma::gameplay::AbilityId{"ability.anna.2"}, conditionalTarget->id});
    check(conditionalResult.hasValue());
    conditionalTarget =
        const_cast<soma::simulation::CombatantState*>(conditionalDecayBattle.find(entity("enemy.gesetzloser", 4)));
    check(conditionalTarget->decay.size() == 1);
    check(conditionalTarget->decay.front().damage == 3);
    check(conditionalTarget->decay.front().remainingTurns == 9);
    const auto criticalBuff = std::ranges::find_if(conditionalTarget->buffs, [](const auto& buff) {
        return buff.attributes.armour == 1;
    });
    check(criticalBuff != conditionalTarget->buffs.end());
    check(criticalBuff->remainingTurns == 5);

    auto decayConversionContent = loaded.content;
    auto& conversionAnna = decayConversionContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.anna"});
    conversionAnna.attributes.initiative = 100;
    auto& conversionSimon = decayConversionContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"});
    conversionSimon.attributes.initiative = 50;
    decayConversionContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}).attributes.initiative = 0;
    auto& decayConversion = decayConversionContent.abilities.at(soma::gameplay::AbilityId{"ability.anna.2"});
    decayConversion.hostileEffect = {};
    decayConversion.hostileEffect.removeDecayAndHealParty = true;
    decayConversion.fatigue = 0;
    soma::gameplay::SeededRandom decayConversionRandom{1008};
    soma::simulation::BattleSimulation decayConversionBattle{
        decayConversionContent,
        {{soma::gameplay::ContentDefinitionId{"player.anna"}, 2},
         {soma::gameplay::ContentDefinitionId{"player.simon"}, 0},
         {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}},
        decayConversionRandom};
    auto* conversionTarget =
        const_cast<soma::simulation::CombatantState*>(decayConversionBattle.find(entity("enemy.gesetzloser", 4)));
    auto* firstHealingTarget =
        const_cast<soma::simulation::CombatantState*>(decayConversionBattle.find(entity("player.anna", 2)));
    auto* secondHealingTarget =
        const_cast<soma::simulation::CombatantState*>(decayConversionBattle.find(entity("player.simon", 0)));
    firstHealingTarget->health = 1;
    secondHealingTarget->health = 1;
    conversionTarget->decay.push_back({1, 1});
    conversionTarget->decay.push_back({1, 2});
    const auto decayConversionResult = decayConversionBattle.execute(
        {firstHealingTarget->id, soma::gameplay::AbilityId{"ability.anna.2"}, conversionTarget->id});
    check(decayConversionResult.hasValue());
    check(conversionTarget->decay.empty());
    check(firstHealingTarget->health + secondHealingTarget->health == 5);
    check(std::abs(firstHealingTarget->health - secondHealingTarget->health) == 1);

    auto randomTargetContent = loaded.content;
    auto& randomAnna = randomTargetContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.anna"});
    randomAnna.attributes.initiative = 100;
    randomAnna.attributes.damageMinimum = 10;
    randomAnna.attributes.damageMaximum = 10;
    randomAnna.attributes.criticalHitChance = 0;
    randomAnna.abilities.push_back(soma::gameplay::AbilityId{"ability.anna.1.alternative"});
    auto& randomEnemy = randomTargetContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"});
    randomEnemy.attributes.initiative = 0;
    randomEnemy.attributes.maximumHealth = 100;
    randomEnemy.attributes.armour = 0;
    randomEnemy.attributes.dodge = 0;
    soma::gameplay::SeededRandom randomTargetRandom{1009};
    soma::simulation::BattleSimulation randomTargetBattle{
        randomTargetContent,
        {{soma::gameplay::ContentDefinitionId{"player.anna"}, 2},
         {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4},
         {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 5}},
        randomTargetRandom};
    const auto randomTargetResult = randomTargetBattle.execute(
        {entity("player.anna", 2), soma::gameplay::AbilityId{"ability.anna.1.alternative"}, entity("enemy.gesetzloser", 4)});
    check(randomTargetResult.hasValue());
    check(randomTargetBattle.find(entity("player.anna", 2))->fatigue == 2);
    check(std::ranges::count_if(randomTargetResult.value(), [](const auto& event) {
              return std::holds_alternative<soma::gameplay::DamageApplied>(event);
          }) == 1);
    check(std::ranges::count_if(randomTargetResult.value(), [](const auto& event) {
              return std::holds_alternative<soma::gameplay::TargetRedirected>(event);
          }) == 1);

    auto transferContent = loaded.content;
    auto& transferMarkus = transferContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.markus"});
    transferMarkus.attributes.initiative = 100;
    transferMarkus.attributes.criticalHitChance = 0;
    auto& transferEnemy = transferContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"});
    transferEnemy.attributes.initiative = 0;
    transferEnemy.attributes.maximumHealth = 100;
    transferEnemy.attributes.dodge = 0;
    auto& transferAbility = transferContent.abilities.at(soma::gameplay::AbilityId{"ability.markus.1"});
    transferAbility.hostileEffect = {};
    transferAbility.hostileEffect.transferredMissingHealthDamageFactor = 0.25;
    transferAbility.hostileEffect.transferredMissingHealthHealingFactor = 0.25;
    soma::gameplay::SeededRandom transferRandom{1010};
    soma::simulation::BattleSimulation transferBattle{
        transferContent,
        {{soma::gameplay::ContentDefinitionId{"player.markus"}, 3},
         {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4},
         {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 6, 60}},
        transferRandom};
    const auto transferResult = transferBattle.execute(
        {entity("player.markus", 3), soma::gameplay::AbilityId{"ability.markus.1"}, entity("enemy.gesetzloser", 4)});
    check(transferResult.hasValue());
    check(transferBattle.find(entity("enemy.gesetzloser", 4))->health == 90);
    check(transferBattle.find(entity("enemy.gesetzloser", 6))->health == 70);

    auto lethalTransferContent = transferContent;
    auto& lethalTransferMarkus = lethalTransferContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.markus"});
    lethalTransferMarkus.attributes.damageMinimum = 100;
    lethalTransferMarkus.attributes.damageMaximum = 100;
    lethalTransferContent.abilities.at(soma::gameplay::AbilityId{"ability.markus.1"}).hostileEffect.damageFactor = 1.0;
    soma::gameplay::SeededRandom lethalTransferRandom{1011};
    soma::simulation::BattleSimulation lethalTransferBattle{
        lethalTransferContent,
        {{soma::gameplay::ContentDefinitionId{"player.markus"}, 3},
         {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4, 5},
         {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 6, 60}},
        lethalTransferRandom};
    check(lethalTransferBattle
              .execute({entity("player.markus", 3), soma::gameplay::AbilityId{"ability.markus.1"}, entity("enemy.gesetzloser", 4)})
              .hasValue());
    check(lethalTransferBattle.find(entity("enemy.gesetzloser", 6))->health == 70);

    auto successiveContent = loaded.content;
    auto& successiveMarkus = successiveContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.markus"});
    successiveMarkus.attributes.initiative = 100;
    successiveMarkus.attributes.damageMinimum = 20;
    successiveMarkus.attributes.damageMaximum = 20;
    successiveMarkus.attributes.criticalHitChance = 0;
    successiveMarkus.attributes.precision = 100;
    successiveMarkus.abilities.push_back(soma::gameplay::AbilityId{"ability.markus.1.alternative"});
    auto& successiveEnemy = successiveContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"});
    successiveEnemy.attributes.initiative = 0;
    successiveEnemy.attributes.maximumHealth = 100;
    successiveEnemy.attributes.armour = 0;
    successiveEnemy.attributes.dodge = 0;
    auto& successiveAbility = successiveContent.abilities.at(soma::gameplay::AbilityId{"ability.markus.1.alternative"});
    successiveAbility.hostileEffect = {};
    successiveAbility.hostileEffect.damageFactor = 1.0;
    successiveAbility.hostileEffect.successiveTargetDamageReduction = 0.75;
    successiveAbility.hostileEffect.moveTargetsRandomly = true;
    successiveAbility.fatigue = 0;
    soma::gameplay::SeededRandom successiveRandom{1011};
    soma::simulation::BattleSimulation successiveBattle{
        successiveContent,
        {{soma::gameplay::ContentDefinitionId{"player.markus"}, 3},
         {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4},
         {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 5},
         {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 6}},
        successiveRandom};
    const auto successiveResult = successiveBattle.execute(
        {entity("player.markus", 3), soma::gameplay::AbilityId{"ability.markus.1.alternative"}, entity("enemy.gesetzloser", 4)});
    check(successiveResult.hasValue());
    check(successiveBattle.find(entity("enemy.gesetzloser", 4))->health == 80);
    check(successiveBattle.find(entity("enemy.gesetzloser", 5))->health == 95);
    check(successiveBattle.find(entity("enemy.gesetzloser", 6))->health == 99);
    std::set<int> movedSlots;
    for (const auto& enemyId : {entity("enemy.gesetzloser", 4), entity("enemy.gesetzloser", 5), entity("enemy.gesetzloser", 6)})
    {
        const auto slot = successiveBattle.find(enemyId)->slot;
        check(slot >= 4 && slot <= 7);
        movedSlots.insert(slot);
    }
    check(movedSlots.size() == 3);

    auto panicContent = loaded.content;
    auto& panicMarkus = panicContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.markus"});
    panicMarkus.attributes.initiative = 100;
    panicMarkus.abilities.push_back(soma::gameplay::AbilityId{"ability.markus.3.alternative"});
    auto& panicSimon = panicContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"});
    panicSimon.attributes.initiative = 50;
    panicSimon.attributes.damageMinimum = 10;
    panicSimon.attributes.damageMaximum = 10;
    panicSimon.attributes.criticalHitChance = 0;
    panicSimon.attributes.precision = 100;
    auto& panicEnemy = panicContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"});
    panicEnemy.attributes.initiative = 0;
    panicEnemy.attributes.armour = 0;
    panicEnemy.attributes.dodge = 0;
    panicEnemy.resistances.debuffResistance = 0;
    auto& panicAbility = panicContent.abilities.at(soma::gameplay::AbilityId{"ability.markus.3.alternative"});
    panicAbility.hostileEffect = {};
    panicAbility.hostileEffect.panicStacks = 2;
    panicAbility.hostileEffect.targetBuff.duration = 2;
    panicAbility.hostileEffect.targetBuff.positive = false;
    panicAbility.hostileEffect.targetBuff.damagePerNegativeStatus = -0.05;
    panicAbility.fatigue = 0;
    auto& panicReactionAbility = panicContent.abilities.at(soma::gameplay::AbilityId{"ability.simon.0"});
    panicReactionAbility.targeting = {soma::content::TargetRelation::opposingTeam, soma::content::TargetScope::single, 1, false};
    panicReactionAbility.hostileEffect = {};
    panicReactionAbility.hostileEffect.damageFactor = 1.0;
    panicReactionAbility.fatigue = 0;
    soma::gameplay::SeededRandom panicRandom{1012};
    soma::simulation::BattleSimulation panicBattle{
        panicContent,
        {{soma::gameplay::ContentDefinitionId{"player.markus"}, 3},
         {soma::gameplay::ContentDefinitionId{"player.simon"}, 0},
         {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}},
        panicRandom};
    auto* panicTarget = const_cast<soma::simulation::CombatantState*>(panicBattle.find(entity("enemy.gesetzloser", 4)));
    panicTarget->markedTurns = 2;
    panicTarget->confusedTurns = 2;
    const auto basePrecision = panicTarget->attributes.precision;
    const auto panicResult = panicBattle.execute(
        {entity("player.markus", 3), soma::gameplay::AbilityId{"ability.markus.3.alternative"}, panicTarget->id});
    check(panicResult.hasValue());
    check(panicTarget->panicStacks == 3);
    check(panicBattle.effectiveAttributes(*panicTarget).precision == basePrecision - 15);
    check(panicTarget->debuffs.size() == 1);
    check(panicTarget->debuffs.front().negativeStatusCountAtApplication == 3);
    check(panicBattle.modifiedDamage(*panicTarget, *panicBattle.find(entity("player.markus", 3)), 100) == 85);
    const auto panicReactionResult = panicBattle.execute(
        {entity("player.simon", 0), soma::gameplay::AbilityId{"ability.simon.0"}, panicTarget->id});
    check(panicReactionResult.hasValue());
    const auto damageEvent = std::ranges::find_if(panicReactionResult.value(), [](const auto& event) {
        return std::holds_alternative<soma::gameplay::DamageApplied>(event);
    });
    const auto panicEvent = std::ranges::find_if(panicReactionResult.value(), [](const auto& event) {
        const auto* status = std::get_if<soma::gameplay::StatusGained>(&event);
        return status != nullptr && status->status == soma::gameplay::StatusId{"status.panic"};
    });
    check(damageEvent != panicReactionResult.value().end());
    check(panicEvent != panicReactionResult.value().end());
    check(damageEvent < panicEvent);
    check(panicTarget->panicStacks == 3);

    auto* panickedMarkus = const_cast<soma::simulation::CombatantState*>(panicBattle.find(entity("player.markus", 3)));
    panickedMarkus->panicStacks = 2;
    panickedMarkus->health -= 5;
    soma::gameplay::ConsumableItem panicHealing;
    panicHealing.healing = 5;
    check(panicBattle.useConsumable(panickedMarkus->id, panicHealing).hasValue());
    check(panickedMarkus->panicStacks == 1);
    soma::gameplay::ConsumableItem panicBuff;
    panicBuff.duration = 1;
    panicBuff.temporaryBonuses.armour = 1;
    check(panicBattle.useConsumable(panickedMarkus->id, panicBuff).hasValue());
    check(panickedMarkus->panicStacks == 0);

    auto panicConversionContent = loaded.content;
    auto& panicConversionMarkus =
        panicConversionContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.markus"});
    panicConversionMarkus.attributes.initiative = 100;
    panicConversionMarkus.attributes.criticalHitChance = 0;
    panicConversionMarkus.abilities.push_back(soma::gameplay::AbilityId{"ability.markus.3.alternative"});
    panicConversionContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"}).attributes.initiative = 50;
    auto& panicConversionEnemy =
        panicConversionContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"});
    panicConversionEnemy.attributes.initiative = 0;
    panicConversionEnemy.attributes.maximumHealth = 100;
    panicConversionEnemy.attributes.armour = 100;
    auto& panicConversionAbility =
        panicConversionContent.abilities.at(soma::gameplay::AbilityId{"ability.markus.3.alternative"});
    panicConversionAbility.hostileEffect = {};
    panicConversionAbility.hostileEffect.doublePanic = true;
    panicConversionAbility.hostileEffect.damageFromPanic = true;
    panicConversionAbility.hostileEffect.panicStacks = 2;
    soma::gameplay::SeededRandom panicConversionRandom{1013};
    soma::simulation::BattleSimulation panicConversionBattle{
        panicConversionContent,
        {{soma::gameplay::ContentDefinitionId{"player.markus"}, 3},
         {soma::gameplay::ContentDefinitionId{"player.simon"}, 0},
         {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}},
        panicConversionRandom};
    auto* panicConversionTarget =
        const_cast<soma::simulation::CombatantState*>(panicConversionBattle.find(entity("enemy.gesetzloser", 4)));
    panicConversionTarget->panicStacks = 12;
    const auto panicConversionResult = panicConversionBattle.execute(
        {entity("player.markus", 3), soma::gameplay::AbilityId{"ability.markus.3.alternative"}, panicConversionTarget->id});
    check(panicConversionResult.hasValue());
    check(panicConversionTarget->health == 88);
    check(panicConversionTarget->panicStacks == 15);

    auto lodgeContent = loaded.content;
    lodgeContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"})
        .equipmentEffects.push_back({soma::gameplay::ItemId{"item.kutschfahrt.lodge_ring"}, soma::gameplay::LoneVictoryEffect{10, 10}});
    soma::gameplay::SeededRandom lodgeRandom{999};
    soma::simulation::BattleSimulation lodgeBattle{
        lodgeContent,
        {{soma::gameplay::ContentDefinitionId{"player.simon"}, 0},
         {soma::gameplay::ContentDefinitionId{"player.ole"}, 1, 4},
         {soma::gameplay::ContentDefinitionId{"player.anna"}, 2, 3},
         {soma::gameplay::ContentDefinitionId{"player.markus"}, 3, 3},
         {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}},
        lodgeRandom};
    const auto lodgeEvents = lodgeBattle.begin();
    check(std::ranges::count_if(lodgeEvents, [](const auto& event) {
              return std::holds_alternative<soma::gameplay::ItemActivated>(event);
          }) == 1);
    check(lodgeBattle.find(entity("player.simon", 0))->health == 25);
    check(lodgeBattle.find(entity("player.ole", 1))->health == 0);
    check(lodgeBattle.find(entity("player.anna", 2))->health == 0);
    check(lodgeBattle.find(entity("player.markus", 3))->health == 0);
    check(lodgeBattle.find(entity("enemy.gesetzloser", 4))->health == 10);

    // Keep automated battles headless in tests: this follows the same first-valid-ability
    // strategy as the development launcher without opening a window or waiting for input.
    soma::gameplay::SeededRandom automatedRandom{12345};
    soma::simulation::BattleSimulation automatedBattle{
        loaded.content,
        {{soma::gameplay::ContentDefinitionId{"player.simon"}, 0},
         {soma::gameplay::ContentDefinitionId{"player.ole"}, 1},
         {soma::gameplay::ContentDefinitionId{"player.anna"}, 2},
         {soma::gameplay::ContentDefinitionId{"player.markus"}, 3},
         {soma::gameplay::ContentDefinitionId{"enemy.abtruenniger"}, 4},
         {soma::gameplay::ContentDefinitionId{"enemy.abtruenniger"}, 5},
         {soma::gameplay::ContentDefinitionId{"enemy.abtruenniger"}, 6}},
        automatedRandom};
    soma::simulation::EnemyPolicy automatedEnemyPolicy{automatedRandom};
    auto automatedTurns = 0;
    while (!automatedBattle.finished() && automatedTurns++ < 500)
    {
        const auto* actor = automatedBattle.find(automatedBattle.currentActor());
        check(actor != nullptr);
        if (actor->team == soma::gameplay::Team::enemies)
        {
            const auto command = automatedEnemyPolicy.choose(automatedBattle);
            const auto result = command.hasValue() ? automatedBattle.execute(command.value()) : automatedBattle.passTurn(actor->id);
            check(result.hasValue());
            continue;
        }

        auto actionExecuted = false;
        for (const auto& ability : loaded.content.combatants.at(actor->definition).abilities)
        {
            const auto targets = automatedBattle.validPrimaryTargets(ability);
            if (targets.empty())
            {
                continue;
            }
            check(automatedBattle.execute({actor->id, ability, targets.front()}).hasValue());
            actionExecuted = true;
            break;
        }
        if (!actionExecuted)
        {
            check(automatedBattle.passTurn(actor->id).hasValue());
        }
    }
    check(automatedBattle.finished());
}
