#include "BattleTestSupport.hpp"

namespace soma::tests
{
    void runBattleCoreTests(const content::BattleContentLoadResult& loaded, const content::RuleContentLoadResult& rules)
    {
        const auto gregId = soma::gameplay::ContentDefinitionId{"enemy.greg"};
        const auto& gregDefinition = loaded.content.combatants.at(gregId);
        soma::gameplay::SeededRandom bossRandom{0};
        soma::simulation::BattleSimulation bossBattle{
            loaded.content,
            {{soma::gameplay::ContentDefinitionId{"player.simon"}, 0},
             {gregId, 4, std::nullopt, {}, 0, loaded.content.bossHealthAndDamageMultiplier}},
            bossRandom};
        const auto* scaledBoss = bossBattle.find(entity("enemy.greg", 4));
        check(scaledBoss != nullptr);
        const auto scaled = [&loaded](int value) {
            return static_cast<int>(std::round(static_cast<double>(value) * loaded.content.bossHealthAndDamageMultiplier));
        };
        check(scaledBoss->attributes.maximumHealth == scaled(gregDefinition.attributes.maximumHealth));
        check(scaledBoss->attributes.damageMinimum == scaled(gregDefinition.attributes.damageMinimum));
        check(scaledBoss->attributes.damageMaximum == scaled(gregDefinition.attributes.damageMaximum));
        check(scaledBoss->attributes.armour == gregDefinition.attributes.armour);
        check(scaledBoss->attributes.criticalHitChance == gregDefinition.attributes.criticalHitChance);
        check(scaledBoss->attributes.dodge == gregDefinition.attributes.dodge);
        check(scaledBoss->attributes.initiative == gregDefinition.attributes.initiative);
        check(scaledBoss->attributes.precision == gregDefinition.attributes.precision);
        check(scaledBoss->secondaryAttributes == gregDefinition.resistances);
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

        auto initiativeContent = loaded.content;
        initiativeContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.ole"}).attributes.initiative = 12;
        initiativeContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"}).attributes.initiative = 10;
        initiativeContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}).attributes.initiative = 8;
        soma::gameplay::SeededRandom initiativeRandom{3};
        soma::simulation::BattleSimulation initiativeBattle{
            initiativeContent,
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
    
        auto initiativeDebuffContent = initiativeContent;
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

        auto lethalRiposteContent = loaded.content;
        auto& riposteActorDefinition = lethalRiposteContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.bill_noface"});
        riposteActorDefinition.attributes.initiative = 50;
        riposteActorDefinition.attributes.precision = 100;
        auto& riposteTargetDefinition = lethalRiposteContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"});
        riposteTargetDefinition.attributes.initiative = 100;
        riposteTargetDefinition.attributes.damageMinimum = 100;
        riposteTargetDefinition.attributes.damageMaximum = 100;
        lethalRiposteContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.markus"}).attributes.initiative = 0;
        auto& riposteSetup = lethalRiposteContent.abilities.at(soma::gameplay::AbilityId{"ability.simon.0"});
        riposteSetup.targeting = {soma::content::TargetRelation::self, soma::content::TargetScope::single, 1, false};
        riposteSetup.friendlyEffect = {};
        riposteSetup.friendlyEffect.riposteSelfRounds = 2;
        riposteSetup.friendlyEffect.addMissSelf = 1;
        riposteSetup.hostileEffect = riposteSetup.friendlyEffect;
        riposteSetup.fatigue = 0;
        soma::gameplay::SeededRandom lethalRiposteRandom{778};
        soma::simulation::BattleSimulation lethalRiposteBattle{
            lethalRiposteContent,
            {{soma::gameplay::ContentDefinitionId{"enemy.bill_noface"}, 4, 1},
             {soma::gameplay::ContentDefinitionId{"player.simon"}, 0},
             {soma::gameplay::ContentDefinitionId{"player.markus"}, 1}},
            lethalRiposteRandom};
        const auto riposteActor = entity("enemy.bill_noface", 4);
        const auto riposteTarget = entity("player.simon", 0);
        const auto untouchedTarget = entity("player.markus", 1);
        const auto riposteSetupResult = lethalRiposteBattle.execute(
            {riposteTarget, soma::gameplay::AbilityId{"ability.simon.0"}, riposteTarget});
        check(riposteSetupResult.hasValue());
        check(lethalRiposteBattle.find(riposteTarget)->riposteTurns > 0);
        check(lethalRiposteBattle.find(riposteTarget)->misses == 1);
        const auto untouchedHealth = lethalRiposteBattle.find(untouchedTarget)->health;
        const auto lethalRiposteResult = lethalRiposteBattle.execute(
            {riposteActor, soma::gameplay::AbilityId{"ability.enemy.shotgun"}, riposteTarget});
        check(lethalRiposteResult.hasValue());
        check(!lethalRiposteBattle.find(riposteActor)->alive());
        check(lethalRiposteBattle.find(untouchedTarget)->health == untouchedHealth);
    
        auto confusionContent = loaded.content;
        auto& confusingSimon = confusionContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"});
        confusingSimon.attributes.initiative = 100;
        confusingSimon.attributes.precision = 100;
        auto& confusedTequila = confusionContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.tequila_joe"});
        confusedTequila.attributes.initiative = 50;
        confusedTequila.attributes.dodge = 0;
        confusedTequila.resistances.confusionResistance = 0;
        auto& confusionSetup = confusionContent.abilities.at(soma::gameplay::AbilityId{"ability.simon.0"});
        confusionSetup.targeting = {soma::content::TargetRelation::opposingTeam, soma::content::TargetScope::single, 1, false};
        confusionSetup.hostileEffect = {};
        confusionSetup.hostileEffect.confusionProbability = 1.0;
        confusionSetup.hostileEffect.confusionRounds = 2;
        confusionSetup.fatigue = 0;
        MinimumRandom confusionRandom;
        soma::simulation::BattleSimulation confusedBattle{
            confusionContent,
            {{soma::gameplay::ContentDefinitionId{"enemy.tequila_joe"}, 4}, {soma::gameplay::ContentDefinitionId{"player.simon"}, 0, 10}},
            confusionRandom};
        const auto tequilaJoe = entity("enemy.tequila_joe", 4);
        const auto confusedTarget = entity("player.simon", 0);
        const auto simonBaseArmour = confusedBattle.find(confusedTarget)->attributes.armour;
        const auto tequilaJoeBaseArmour = confusedBattle.find(tequilaJoe)->attributes.armour;
        const auto confusionApplied = confusedBattle.execute(
            {confusedTarget, soma::gameplay::AbilityId{"ability.simon.0"}, tequilaJoe});
        check(confusionApplied.hasValue());
        check(confusedBattle.currentActor() == tequilaJoe);
        check(confusedBattle.find(tequilaJoe)->confusedThisTurn);
        const auto confusedBuff = confusedBattle.execute({tequilaJoe, soma::gameplay::AbilityId{"ability.enemy.beer"}, tequilaJoe});
        check(confusedBuff.hasValue());
        check(std::ranges::any_of(confusedBuff.value(), [&confusedTarget](const auto& event) {
            const auto* redirected = std::get_if<soma::gameplay::TargetRedirected>(&event);
            return redirected != nullptr && redirected->resolvedTarget == confusedTarget;
        }));
        check(confusedBattle.find(confusedTarget)->attributes.armour == simonBaseArmour + 5);
        check(confusedBattle.find(tequilaJoe)->attributes.armour == tequilaJoeBaseArmour);
    
        auto enemyFatigueContent = loaded.content;
        auto& targetedFatigueAbility = enemyFatigueContent.abilities.at(soma::gameplay::AbilityId{"ability.enemy.gravedigger_gaze"});
        targetedFatigueAbility.targeting = {
            soma::content::TargetRelation::opposingTeam, soma::content::TargetScope::single, 1, false};
        targetedFatigueAbility.friendlyEffect = {};
        targetedFatigueAbility.hostileEffect = {};
        targetedFatigueAbility.hostileEffect.targetFatigue = 2;
        targetedFatigueAbility.fatigue = 0;
        soma::gameplay::SeededRandom enemyFatigueRandom{894};
        soma::simulation::BattleSimulation enemyFatigueBattle{enemyFatigueContent,
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

        auto summoningContent = loaded.content;
        auto& summonerDefinition = summoningContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.apache_kid"});
        summonerDefinition.attributes.initiative = 100;
        auto& summoningAbility = summoningContent.abilities.at(soma::gameplay::AbilityId{"ability.enemy.indians"});
        summoningAbility.targeting = {soma::content::TargetRelation::opposingTeam, soma::content::TargetScope::single, 1, false};
        summoningAbility.hostileEffect.damageFactor = 1.0;
        soma::gameplay::SeededRandom summoningRandom{900};
        soma::simulation::BattleSimulation summoningRuleBattle{
            summoningContent,
            {{soma::gameplay::ContentDefinitionId{"enemy.apache_kid"}, 4},
             {soma::gameplay::ContentDefinitionId{"player.simon"}, 0}},
            summoningRandom,
            &rules.content,
            &triggeredRules};
        const auto summoningResult = summoningRuleBattle.execute(
            {entity("enemy.apache_kid", 4), soma::gameplay::AbilityId{"ability.enemy.indians"}, entity("player.simon", 0)});
        check(summoningResult.hasValue());
        check(std::ranges::count_if(summoningResult.value(), [](const auto& event) {
                  return std::holds_alternative<soma::gameplay::CombatantSummoned>(event);
              }) == 3);
        check(std::ranges::any_of(summoningResult.value(), [](const auto& event) {
            return std::holds_alternative<soma::gameplay::RuleActivated>(event);
        }));
    
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

        auto confusedSkipContent = loaded.content;
        auto& skipEnemy = confusedSkipContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"});
        skipEnemy.attributes.initiative = 100;
        skipEnemy.attributes.precision = 100;
        auto& skipSimon = confusedSkipContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"});
        skipSimon.attributes.initiative = 50;
        skipSimon.attributes.dodge = 0;
        skipSimon.resistances.confusionResistance = 0;
        auto& skipSetup = confusedSkipContent.abilities.at(soma::gameplay::AbilityId{"ability.enemy.springfield"});
        skipSetup.hostileEffect = {};
        skipSetup.hostileEffect.confusionProbability = 1.0;
        skipSetup.hostileEffect.confusionRounds = 2;
        MinimumRandom confusedSkipRandom;
        soma::simulation::BattleSimulation confusedSkipBattle{
            confusedSkipContent,
            {{soma::gameplay::ContentDefinitionId{"player.simon"}, 0},
             {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}},
            confusedSkipRandom,
            &rules.content,
            &turnRules};
        const auto confusedActor = entity("player.simon", 0);
        const auto nextActor = entity("enemy.gesetzloser", 4);
        const auto confusionSetupResult = confusedSkipBattle.execute(
            {nextActor, soma::gameplay::AbilityId{"ability.enemy.springfield"}, confusedActor});
        check(confusionSetupResult.hasValue());
        check(confusedSkipBattle.currentActor() == confusedActor);
        check(confusedSkipBattle.find(confusedActor)->confusedThisTurn);
        const auto nextActorHealth = confusedSkipBattle.find(nextActor)->health;
        const auto confusedSkipResult = confusedSkipBattle.execute(
            {confusedActor, soma::gameplay::AbilityId{"ability.simon.1"}, nextActor});
        check(confusedSkipResult.hasValue());
        check(confusedSkipBattle.currentActor() == nextActor);
        check(confusedSkipBattle.find(nextActor)->health == nextActorHealth - 1);
        check(std::ranges::any_of(confusedSkipResult.value(), [](const auto& event) {
            return std::holds_alternative<soma::gameplay::RuleActivated>(event);
        }));
    
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
        auto& decayingSimon = decayTriggerContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"});
        decayingSimon.attributes.initiative = 0;
        decayingSimon.attributes.dodge = 0;
        decayingSimon.resistances.decayResistance = 0;
        decayTriggerContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}).attributes.criticalHitChance = 0;
        auto& decaySetup = decayTriggerContent.abilities.at(soma::gameplay::AbilityId{"ability.enemy.springfield"});
        decaySetup.hostileEffect = {};
        decaySetup.hostileEffect.decayDamage = 1;
        decaySetup.hostileEffect.decayRounds = 2;
        MinimumRandom decayDamageRandom;
        soma::simulation::BattleSimulation decayDamageBattle{
            decayTriggerContent,
            {{soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}, {soma::gameplay::ContentDefinitionId{"player.simon"}, 0}},
            decayDamageRandom,
            &rules.content,
            &decayDamageRules};
        const auto decayingPlayer = entity("player.simon", 0);
        const auto decayDamageResult = decayDamageBattle.execute(
            {entity("enemy.gesetzloser", 4), soma::gameplay::AbilityId{"ability.enemy.springfield"}, decayingPlayer});
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

    }
} // namespace soma::tests
