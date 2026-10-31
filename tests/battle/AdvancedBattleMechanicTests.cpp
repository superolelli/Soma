#include "BattleTestSupport.hpp"

namespace soma::tests
{
    void runAdvancedBattleMechanicTests(const content::BattleContentLoadResult& loaded)
    {
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
        panicEnemy.attributes.maximumHealth = 100;
        panicEnemy.attributes.initiative = 0;
        panicEnemy.attributes.armour = 0;
        panicEnemy.attributes.dodge = 0;
        panicEnemy.attributes.damageMinimum = 5;
        panicEnemy.attributes.damageMaximum = 5;
        panicEnemy.attributes.precision = 100;
        panicEnemy.attributes.criticalHitChance = 0;
        panicEnemy.resistances.debuffResistance = 0;
        panicEnemy.resistances.confusionResistance = 0;
        auto& panicSetupActor = panicContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.ole"});
        panicSetupActor.attributes.initiative = 110;
        panicSetupActor.attributes.criticalHitChance = 0;
        panicSetupActor.attributes.precision = 100;
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
        auto& panicSetupAbility = panicContent.abilities.at(soma::gameplay::AbilityId{"ability.ole.0"});
        panicSetupAbility.targeting = {soma::content::TargetRelation::opposingTeam, soma::content::TargetScope::single, 1, false};
        panicSetupAbility.hostileEffect = {};
        panicSetupAbility.hostileEffect.markRounds = 2;
        panicSetupAbility.hostileEffect.confusionProbability = 1.0;
        panicSetupAbility.hostileEffect.confusionRounds = 2;
        panicSetupAbility.fatigue = 0;
        auto& panicOnMarkus = panicContent.abilities.at(soma::gameplay::AbilityId{"ability.enemy.springfield"});
        panicOnMarkus.hostileEffect = {};
        panicOnMarkus.hostileEffect.damageFactor = 1.0;
        panicOnMarkus.hostileEffect.ignoreArmour = true;
        panicOnMarkus.hostileEffect.panicStacks = 2;
        panicOnMarkus.fatigue = 0;
        soma::gameplay::SeededRandom panicRandom{1012};
        soma::simulation::BattleSimulation panicBattle{
            panicContent,
            {{soma::gameplay::ContentDefinitionId{"player.ole"}, 1},
             {soma::gameplay::ContentDefinitionId{"player.markus"}, 3},
             {soma::gameplay::ContentDefinitionId{"player.simon"}, 0},
             {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4},
             {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 5}},
            panicRandom};
        const auto panicTarget = entity("enemy.gesetzloser", 4);
        const auto panicSetupResult = panicBattle.execute(
            {entity("player.ole", 1), soma::gameplay::AbilityId{"ability.ole.0"}, panicTarget});
        check(panicSetupResult.hasValue());
        check(panicBattle.find(panicTarget)->markedTurns == 2);
        check(panicBattle.find(panicTarget)->confusedTurns == 2);
        const auto basePrecision = panicBattle.find(panicTarget)->attributes.precision;
        const auto panicResult = panicBattle.execute(
            {entity("player.markus", 3), soma::gameplay::AbilityId{"ability.markus.3.alternative"}, panicTarget});
        check(panicResult.hasValue());
        check(panicBattle.find(panicTarget)->panicStacks == 3);
        check(panicBattle.effectiveAttributes(*panicBattle.find(panicTarget)).precision == basePrecision - 15);
        check(panicBattle.find(panicTarget)->debuffs.size() == 1);
        check(panicBattle.find(panicTarget)->debuffs.front().negativeStatusCountAtApplication == 3);
        check(panicBattle.modifiedDamage(*panicBattle.find(panicTarget), *panicBattle.find(entity("player.markus", 3)), 100) == 85);
        const auto panicReactionResult = panicBattle.execute(
            {entity("player.simon", 0), soma::gameplay::AbilityId{"ability.simon.0"}, panicTarget});
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
        check(panicBattle.find(panicTarget)->panicStacks == 3);
    
        const auto panickedMarkus = entity("player.markus", 3);
        check(panicBattle.passTurn(panicTarget).hasValue());
        const auto panicOnMarkusResult = panicBattle.execute(
            {entity("enemy.gesetzloser", 5), soma::gameplay::AbilityId{"ability.enemy.springfield"}, panickedMarkus});
        check(panicOnMarkusResult.hasValue());
        check(panicBattle.find(panickedMarkus)->panicStacks == 2);
        soma::gameplay::ConsumableItem panicHealing;
        panicHealing.healing = 5;
        check(panicBattle.useConsumable(panickedMarkus, panicHealing).hasValue());
        check(panicBattle.find(panickedMarkus)->panicStacks == 1);
        soma::gameplay::ConsumableItem panicBuff;
        panicBuff.duration = 1;
        panicBuff.temporaryBonuses.armour = 1;
        check(panicBattle.useConsumable(panickedMarkus, panicBuff).hasValue());
        check(panicBattle.find(panickedMarkus)->panicStacks == 0);
    
        auto panicConversionContent = loaded.content;
        auto& panicConversionMarkus =
            panicConversionContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.markus"});
        panicConversionMarkus.attributes.initiative = 100;
        panicConversionMarkus.attributes.criticalHitChance = 0;
        panicConversionMarkus.abilities.push_back(soma::gameplay::AbilityId{"ability.markus.3.alternative"});
        auto& panicConversionSimon = panicConversionContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"});
        panicConversionSimon.attributes.initiative = 110;
        panicConversionSimon.attributes.criticalHitChance = 0;
        panicConversionSimon.attributes.precision = 100;
        panicConversionContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.ole"}).attributes.initiative = 50;
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
        auto& panicConversionSetup = panicConversionContent.abilities.at(soma::gameplay::AbilityId{"ability.simon.0"});
        panicConversionSetup.targeting = {soma::content::TargetRelation::opposingTeam, soma::content::TargetScope::single, 1, false};
        panicConversionSetup.hostileEffect = {};
        panicConversionSetup.hostileEffect.panicStacks = 12;
        panicConversionSetup.fatigue = 0;
        soma::gameplay::SeededRandom panicConversionRandom{1013};
        soma::simulation::BattleSimulation panicConversionBattle{
            panicConversionContent,
            {{soma::gameplay::ContentDefinitionId{"player.markus"}, 3},
             {soma::gameplay::ContentDefinitionId{"player.simon"}, 0},
             {soma::gameplay::ContentDefinitionId{"player.ole"}, 1},
             {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}},
            panicConversionRandom};
        const auto panicConversionTarget = entity("enemy.gesetzloser", 4);
        const auto panicConversionSetupResult = panicConversionBattle.execute(
            {entity("player.simon", 0), soma::gameplay::AbilityId{"ability.simon.0"}, panicConversionTarget});
        check(panicConversionSetupResult.hasValue());
        check(panicConversionBattle.find(panicConversionTarget)->panicStacks == 12);
        const auto panicConversionResult = panicConversionBattle.execute(
            {entity("player.markus", 3), soma::gameplay::AbilityId{"ability.markus.3.alternative"}, panicConversionTarget});
        check(panicConversionResult.hasValue());
        check(panicConversionBattle.find(panicConversionTarget)->health == 88);
        check(panicConversionBattle.find(panicConversionTarget)->panicStacks == 15);
    
        auto lodgeContent = loaded.content;
        lodgeContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"})
            .equipmentEffects.push_back({soma::gameplay::ItemId{"item.kutschfahrt.lodge_ring"}, soma::gameplay::LoneVictoryEffect{10, 10}});
        const auto enemyHealthBeforeLodge =
            lodgeContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}).attributes.maximumHealth;
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
        check(lodgeBattle.find(entity("enemy.gesetzloser", 4))->health == std::max(0, enemyHealthBeforeLodge - 10));
    
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
} // namespace soma::tests
