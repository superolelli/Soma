#include "BattleTestSupport.hpp"

namespace soma::tests
{
    void runBattleStatusEffectTests(const content::BattleContentLoadResult& loaded, const content::RuleContentLoadResult& rules)
    {
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

        auto stackingBuffContent = loaded.content;
        auto& stackingSimon = stackingBuffContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"});
        stackingSimon.attributes.initiative = 100;
        stackingSimon.attributes.criticalHitChance = 0;
        const auto baseDamage = stackingSimon.attributes.damageMinimum;
        stackingBuffContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}).attributes.initiative = 0;
        auto& stackingBuffAbility = stackingBuffContent.abilities.at(soma::gameplay::AbilityId{"ability.simon.0"});
        stackingBuffAbility.targeting = {soma::content::TargetRelation::self, soma::content::TargetScope::single, 1, false};
        stackingBuffAbility.friendlyEffect = {};
        stackingBuffAbility.friendlyEffect.targetBuff.duration = 3;
        stackingBuffAbility.friendlyEffect.targetBuff.attributes.damageMinimum = 2;
        stackingBuffAbility.friendlyEffect.targetBuff.attributes.damageMaximum = 2;
        stackingBuffAbility.hostileEffect = stackingBuffAbility.friendlyEffect;
        stackingBuffAbility.fatigue = 0;
        MinimumRandom stackingBuffRandom;
        soma::simulation::BattleSimulation stackingBuffBattle{
            stackingBuffContent,
            {{soma::gameplay::ContentDefinitionId{"player.simon"}, 0},
             {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}},
            stackingBuffRandom};
        const auto stackingSimonId = entity("player.simon", 0);
        const auto stackingEnemyId = entity("enemy.gesetzloser", 4);
        check(stackingBuffBattle.execute(
                  {stackingSimonId, soma::gameplay::AbilityId{"ability.simon.0"}, stackingSimonId})
                  .hasValue());
        check(stackingBuffBattle.passTurn(stackingEnemyId).hasValue());
        check(stackingBuffBattle.execute(
                  {stackingSimonId, soma::gameplay::AbilityId{"ability.simon.0"}, stackingSimonId})
                  .hasValue());
        const auto* stackedState = stackingBuffBattle.find(stackingSimonId);
        check(stackedState->buffs.size() == 2);
        check(stackedState->attributes.damageMinimum == baseDamage + 4);
        check(stackedState->buffs.front().remainingTurns == 2);
        check(stackedState->buffs.back().remainingTurns == 3);
        check(stackingBuffBattle.passTurn(stackingEnemyId).hasValue());
        check(stackingBuffBattle.passTurn(stackingSimonId).hasValue());
        check(stackingBuffBattle.passTurn(stackingEnemyId).hasValue());
        stackedState = stackingBuffBattle.find(stackingSimonId);
        check(stackedState->buffs.size() == 1);
        check(stackedState->attributes.damageMinimum == baseDamage + 2);

        auto riposteKillContent = loaded.content;
        auto& riposteEnemy = riposteKillContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"});
        riposteEnemy.attributes.initiative = 100;
        riposteEnemy.attributes.damageMinimum = 10;
        riposteEnemy.attributes.damageMaximum = 10;
        riposteEnemy.attributes.dodge = 0;
        riposteEnemy.attributes.criticalHitChance = 0;
        auto& doomedSimon = riposteKillContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"});
        doomedSimon.attributes.initiative = 50;
        doomedSimon.attributes.damageMinimum = 1;
        doomedSimon.attributes.damageMaximum = 1;
        doomedSimon.attributes.precision = 100;
        doomedSimon.attributes.criticalHitChance = 0;
        auto& lethalRiposteSetup = riposteKillContent.abilities.at(soma::gameplay::AbilityId{"ability.enemy.springfield"});
        lethalRiposteSetup.targeting = {soma::content::TargetRelation::self, soma::content::TargetScope::single, 1, false};
        lethalRiposteSetup.friendlyEffect = {};
        lethalRiposteSetup.friendlyEffect.riposteRounds = 3;
        lethalRiposteSetup.hostileEffect = lethalRiposteSetup.friendlyEffect;
        lethalRiposteSetup.fatigue = 0;
        auto& doomedAttack = riposteKillContent.abilities.at(soma::gameplay::AbilityId{"ability.simon.0"});
        doomedAttack.targeting = {soma::content::TargetRelation::opposingTeam, soma::content::TargetScope::single, 1, false};
        doomedAttack.hostileEffect = {};
        doomedAttack.hostileEffect.damageFactor = 1.0;
        doomedAttack.fatigue = 0;
        soma::gameplay::ComposedRules riposteKillRules;
        riposteKillRules.at(0).trigger = soma::gameplay::RuleFragmentId{"rule.trigger.killed_other"};
        riposteKillRules.at(0).subject = soma::gameplay::RuleFragmentId{"rule.subject.this_combatant"};
        riposteKillRules.at(0).effect = soma::gameplay::RuleFragmentId{"rule.effect.rage_two"};
        MinimumRandom riposteKillRandom;
        soma::simulation::BattleSimulation riposteKillBattle{
            riposteKillContent,
            {{soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4},
             {soma::gameplay::ContentDefinitionId{"player.simon"}, 0, 1}},
            riposteKillRandom,
            &rules.content,
            &riposteKillRules};
        const auto riposteKiller = entity("enemy.gesetzloser", 4);
        const auto riposteVictim = entity("player.simon", 0);
        check(riposteKillBattle.execute(
                  {riposteKiller, soma::gameplay::AbilityId{"ability.enemy.springfield"}, riposteKiller})
                  .hasValue());
        const auto lethalRiposte =
            riposteKillBattle.execute({riposteVictim, soma::gameplay::AbilityId{"ability.simon.0"}, riposteKiller});
        check(lethalRiposte.hasValue());
        const auto defeat = std::ranges::find_if(lethalRiposte.value(), [](const auto& event) {
            return std::holds_alternative<soma::gameplay::CombatantDefeated>(event);
        });
        check(defeat != lethalRiposte.value().end());
        check(std::get<soma::gameplay::CombatantDefeated>(*defeat).source == riposteKiller);
        check(riposteKillBattle.find(riposteKiller)->rageTurns == 2);

        auto equipmentKillContent = loaded.content;
        equipmentKillContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"}).attributes.initiative = 100;
        auto& equipmentCarrier = equipmentKillContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.ole"});
        equipmentCarrier.attributes.initiative = 50;
        equipmentCarrier.equipmentEffects = {
            {soma::gameplay::ItemId{"item.test.lone_victory"}, soma::gameplay::LoneVictoryEffect{1000, 10}}};
        equipmentKillContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}).attributes.initiative = 0;
        soma::gameplay::ComposedRules equipmentKillRules;
        equipmentKillRules.at(0).trigger = soma::gameplay::RuleFragmentId{"rule.trigger.killed_other"};
        equipmentKillRules.at(0).subject = soma::gameplay::RuleFragmentId{"rule.subject.this_combatant"};
        equipmentKillRules.at(0).effect = soma::gameplay::RuleFragmentId{"rule.effect.rage_two"};
        MinimumRandom equipmentKillRandom;
        soma::simulation::BattleSimulation equipmentKillBattle{
            equipmentKillContent,
            {{soma::gameplay::ContentDefinitionId{"player.simon"}, 0, 20},
             {soma::gameplay::ContentDefinitionId{"player.ole"}, 1},
             {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4, 5}},
            equipmentKillRandom,
            &rules.content,
            &equipmentKillRules};
        const auto equipmentKiller = entity("player.ole", 1);
        const auto equipmentKillResult = equipmentKillBattle.passTurn(entity("player.simon", 0));
        check(equipmentKillResult.hasValue());
        const auto equipmentDefeat = std::ranges::find_if(equipmentKillResult.value(), [](const auto& event) {
            const auto* defeated = std::get_if<soma::gameplay::CombatantDefeated>(&event);
            return defeated != nullptr && defeated->combatant == entity("enemy.gesetzloser", 4);
        });
        check(equipmentDefeat != equipmentKillResult.value().end());
        check(std::get<soma::gameplay::CombatantDefeated>(*equipmentDefeat).source == equipmentKiller);
        check(equipmentKillBattle.find(equipmentKiller)->rageTurns == 2);

        auto dynamiteContent = loaded.content;
        auto& dynamiteActor = dynamiteContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"});
        dynamiteActor.attributes.initiative = 100;
        dynamiteActor.attributes.precision = 100;
        dynamiteActor.attributes.criticalHitChance = 0;
        auto& dynamiteTarget = dynamiteContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"});
        dynamiteTarget.attributes.initiative = 90;
        dynamiteTarget.attributes.dodge = 0;
        dynamiteContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.ole"}).attributes.initiative = 10;
        auto& dynamiteAbility = dynamiteContent.abilities.at(soma::gameplay::AbilityId{"ability.enemy.springfield"});
        dynamiteAbility.hostileEffect = {};
        dynamiteAbility.hostileEffect.dynamite = true;
        dynamiteAbility.fatigue = 0;
        MaximumRandom dynamiteRandom;
        soma::simulation::BattleSimulation dynamiteBattle{
            dynamiteContent,
            {{soma::gameplay::ContentDefinitionId{"player.simon"}, 0},
             {soma::gameplay::ContentDefinitionId{"player.ole"}, 3},
             {soma::gameplay::ContentDefinitionId{"player.ole"}, 1},
             {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 7}},
            dynamiteRandom};
        const auto dynamiteResult = dynamiteBattle.execute(
            {entity("enemy.gesetzloser", 7), soma::gameplay::AbilityId{"ability.enemy.springfield"}, entity("player.simon", 0)});
        check(dynamiteResult.hasValue());
        check(dynamiteBattle.find(entity("player.ole", 1))->dynamite);
        check(!dynamiteBattle.find(entity("player.ole", 3))->dynamite);
    
        auto skillEffectContent = loaded.content;
        auto& skilledSimon = skillEffectContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"});
        skilledSimon.attributes.damageMinimum = 10;
        skilledSimon.attributes.damageMaximum = 10;
        skilledSimon.attributes.initiative = 50;
        skilledSimon.attributes.criticalHitChance = 0;
        skilledSimon.attributes.precision = 100;
        auto& durableEnemy = skillEffectContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"});
        durableEnemy.attributes.maximumHealth = 100;
        durableEnemy.attributes.armour = 0;
        durableEnemy.attributes.criticalHitChance = 0;
        durableEnemy.attributes.dodge = 0;
        durableEnemy.attributes.initiative = 100;
        auto& markedAttack = skillEffectContent.abilities.at(soma::gameplay::AbilityId{"ability.simon.2"});
        markedAttack.targeting = {soma::content::TargetRelation::opposingTeam, soma::content::TargetScope::single, 1, false};
        markedAttack.friendlyEffect = {};
        markedAttack.hostileEffect = {};
        markedAttack.hostileEffect.damageFactor = 0.5;
        markedAttack.hostileEffect.damageAgainstMarked = 0.4;
        markedAttack.allCombatantsBuff.duration = 4;
        markedAttack.allCombatantsBuff.damageAgainstMarked = 0.2;
        markedAttack.fatigue = 0;
        auto& markSetup = skillEffectContent.abilities.at(soma::gameplay::AbilityId{"ability.enemy.springfield"});
        markSetup.targeting = {soma::content::TargetRelation::self, soma::content::TargetScope::single, 1, false};
        markSetup.friendlyEffect = {};
        markSetup.friendlyEffect.markRounds = 5;
        markSetup.hostileEffect = markSetup.friendlyEffect;
        markSetup.fatigue = 0;
        MinimumRandom skillEffectRandom;
        soma::simulation::BattleSimulation skillEffectBattle{
            skillEffectContent,
            {{soma::gameplay::ContentDefinitionId{"player.simon"}, 0}, {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}},
            skillEffectRandom};
        const auto markedEnemy = entity("enemy.gesetzloser", 4);
        const auto markedSetupResult = skillEffectBattle.execute(
            {markedEnemy, soma::gameplay::AbilityId{"ability.enemy.springfield"}, markedEnemy});
        check(markedSetupResult.hasValue());
        check(skillEffectBattle.find(markedEnemy)->markedTurns == 5);
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
        MaximumRandom wakeResistanceRandom;
        soma::simulation::BattleSimulation wakeResistanceBattle{
            wakeResistanceContent,
            {{soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}, {soma::gameplay::ContentDefinitionId{"player.simon"}, 0}},
            wakeResistanceRandom};
        const auto wakingPlayer = entity("player.simon", 0);
        const auto wakingEnemy = entity("enemy.gesetzloser", 4);
        const auto wakeEvents = wakeResistanceBattle.execute(
            {wakingEnemy, soma::gameplay::AbilityId{"ability.enemy.springfield"}, wakingPlayer});
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
        mechanicsOle.attributes.initiative = 50;
        mechanicsOle.attributes.damageMinimum = 10;
        mechanicsOle.attributes.damageMaximum = 10;
        mechanicsOle.attributes.criticalHitChance = 0;
        mechanicsOle.attributes.precision = 100;
        auto& mechanicsEnemy = oleMechanicsContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"});
        mechanicsEnemy.attributes.initiative = 100;
        mechanicsEnemy.attributes.maximumHealth = 100;
        mechanicsEnemy.attributes.armour = 0;
        mechanicsEnemy.attributes.criticalHitChance = 0;
        mechanicsEnemy.attributes.dodge = 0;
        auto& acceleratedDecay = oleMechanicsContent.abilities.at(soma::gameplay::AbilityId{"ability.ole.0"});
        acceleratedDecay.targeting = {soma::content::TargetRelation::opposingTeam, soma::content::TargetScope::single, 1, false};
        acceleratedDecay.hostileEffect = {};
        acceleratedDecay.hostileEffect.damageFactor = 0.5;
        acceleratedDecay.hostileEffect.inflictAllDecayAndRemove = true;
        acceleratedDecay.fatigue = 0;
        auto& decaySetup = oleMechanicsContent.abilities.at(soma::gameplay::AbilityId{"ability.enemy.springfield"});
        decaySetup.targeting = {soma::content::TargetRelation::self, soma::content::TargetScope::single, 1, false};
        decaySetup.friendlyEffect = {};
        decaySetup.friendlyEffect.decayDamage = 2;
        decaySetup.friendlyEffect.decayRounds = 3;
        decaySetup.hostileEffect = decaySetup.friendlyEffect;
        decaySetup.fatigue = 0;
        soma::simulation::ActiveBuff fatigueConversion;
        fatigueConversion.remainingTurns = 3;
        fatigueConversion.positive = true;
        fatigueConversion.damageFromFatigue = true;
        fatigueConversion.blocksFatigueStageOne = true;
        fatigueConversion.blocksFatigueStageTwo = true;
        MinimumRandom oleMechanicsRandom;
        soma::simulation::BattleSimulation oleMechanicsBattle{
            oleMechanicsContent,
            {{soma::gameplay::ContentDefinitionId{"player.ole"}, 0, std::nullopt, {fatigueConversion}, 27},
             {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}},
            oleMechanicsRandom};
        const auto mechanicsTarget = entity("enemy.gesetzloser", 4);
        const auto decaySetupResult = oleMechanicsBattle.execute(
            {mechanicsTarget, soma::gameplay::AbilityId{"ability.enemy.springfield"}, mechanicsTarget});
        check(decaySetupResult.hasValue());
        check(oleMechanicsBattle.find(mechanicsTarget)->decay.size() == 1);
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
        auto& transfusionSimon = transfusionContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"});
        transfusionSimon.attributes.initiative = 100;
        transfusionSimon.attributes.criticalHitChance = 0;
        transfusionSimon.attributes.precision = 100;
        auto& transfusionOle = transfusionContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.ole"});
        transfusionOle.attributes.initiative = 90;
        transfusionOle.attributes.criticalHitChance = 0;
        transfusionOle.attributes.precision = 100;
        transfusionContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}).attributes.initiative = 0;
        auto& firstTransfusion = transfusionContent.abilities.at(soma::gameplay::AbilityId{"ability.simon.0"});
        firstTransfusion.targeting = {soma::content::TargetRelation::opposingTeam, soma::content::TargetScope::single, 1, false};
        firstTransfusion.hostileEffect = {};
        firstTransfusion.hostileEffect.decayDamage = 1;
        firstTransfusion.hostileEffect.decayRounds = 1;
        firstTransfusion.hostileEffect.transfusionHealing = 1;
        firstTransfusion.fatigue = 0;
        auto& secondTransfusion = transfusionContent.abilities.at(soma::gameplay::AbilityId{"ability.ole.0"});
        secondTransfusion.targeting = {soma::content::TargetRelation::opposingTeam, soma::content::TargetScope::single, 1, false};
        secondTransfusion.hostileEffect = {};
        secondTransfusion.hostileEffect.transfusionHealing = 2;
        secondTransfusion.fatigue = 0;
        MinimumRandom transfusionRandom;
        soma::simulation::BattleSimulation transfusionBattle{
            transfusionContent,
            {{soma::gameplay::ContentDefinitionId{"player.simon"}, 0, 5},
             {soma::gameplay::ContentDefinitionId{"player.ole"}, 1},
             {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}},
            transfusionRandom};
        const auto transfusionRecipient = entity("player.simon", 0);
        const auto transfusionSource = entity("enemy.gesetzloser", 4);
        const auto firstTransfusionResult = transfusionBattle.execute(
            {transfusionRecipient, soma::gameplay::AbilityId{"ability.simon.0"}, transfusionSource});
        check(firstTransfusionResult.hasValue());
        const auto transfusionResult = transfusionBattle.execute(
            {entity("player.ole", 1), soma::gameplay::AbilityId{"ability.ole.0"}, transfusionSource});
        check(transfusionResult.hasValue());
        check(transfusionBattle.find(transfusionRecipient)->health == 8);
        check(transfusionBattle.find(transfusionSource)->decay.empty());
        check(transfusionBattle.find(transfusionSource)->transfusions.empty());
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
        conditionalAnna.attributes.initiative = 90;
        conditionalAnna.attributes.criticalHitChance = 100;
        auto& conditionalSimon = conditionalDecayContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"});
        conditionalSimon.attributes.initiative = 100;
        conditionalSimon.attributes.criticalHitChance = 0;
        conditionalSimon.attributes.precision = 100;
        auto& conditionalEnemy = conditionalDecayContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"});
        conditionalEnemy.attributes.initiative = 0;
        conditionalEnemy.attributes.dodge = 0;
        conditionalEnemy.resistances.sleepResistance = 0;
        conditionalEnemy.resistances.confusionResistance = 0;
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
        auto& conditionalSetup = conditionalDecayContent.abilities.at(soma::gameplay::AbilityId{"ability.simon.0"});
        conditionalSetup.targeting = {soma::content::TargetRelation::opposingTeam, soma::content::TargetScope::single, 1, false};
        conditionalSetup.hostileEffect = {};
        conditionalSetup.hostileEffect.markRounds = 2;
        conditionalSetup.hostileEffect.confusionProbability = 1.0;
        conditionalSetup.hostileEffect.confusionRounds = 2;
        conditionalSetup.hostileEffect.sleepProbability = 1.0;
        conditionalSetup.fatigue = 0;
        MinimumRandom conditionalDecayRandom;
        soma::simulation::BattleSimulation conditionalDecayBattle{
            conditionalDecayContent,
            {{soma::gameplay::ContentDefinitionId{"player.simon"}, 0},
             {soma::gameplay::ContentDefinitionId{"player.anna"}, 2},
             {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}},
            conditionalDecayRandom};
        const auto conditionalTarget = entity("enemy.gesetzloser", 4);
        const auto conditionalSetupResult = conditionalDecayBattle.execute(
            {entity("player.simon", 0), soma::gameplay::AbilityId{"ability.simon.0"}, conditionalTarget});
        check(conditionalSetupResult.hasValue());
        check(conditionalDecayBattle.find(conditionalTarget)->markedTurns == 2);
        check(conditionalDecayBattle.find(conditionalTarget)->confusedTurns == 2);
        check(conditionalDecayBattle.find(conditionalTarget)->sleeping);
        const auto conditionalResult = conditionalDecayBattle.execute(
            {entity("player.anna", 2), soma::gameplay::AbilityId{"ability.anna.2"}, conditionalTarget});
        check(conditionalResult.hasValue());
        const auto* conditionalTargetState = conditionalDecayBattle.find(conditionalTarget);
        check(conditionalTargetState->decay.size() == 1);
        check(conditionalTargetState->decay.front().damage == 3);
        check(conditionalTargetState->decay.front().remainingTurns == 9);
        const auto criticalBuff = std::ranges::find_if(conditionalTargetState->buffs, [](const auto& buff) {
            return buff.attributes.armour == 1;
        });
        check(criticalBuff != conditionalTargetState->buffs.end());
        check(criticalBuff->remainingTurns == 5);
    
        auto decayConversionContent = loaded.content;
        auto& conversionAnna = decayConversionContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.anna"});
        conversionAnna.attributes.initiative = 80;
        auto& conversionSimon = decayConversionContent.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"});
        conversionSimon.attributes.initiative = 100;
        conversionSimon.attributes.criticalHitChance = 0;
        conversionSimon.attributes.precision = 100;
        auto& conversionEnemy = decayConversionContent.combatants.at(soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"});
        conversionEnemy.attributes.initiative = 0;
        conversionEnemy.attributes.dodge = 0;
        conversionEnemy.resistances.decayResistance = 0;
        auto& decayConversion = decayConversionContent.abilities.at(soma::gameplay::AbilityId{"ability.anna.2"});
        decayConversion.hostileEffect = {};
        decayConversion.hostileEffect.removeDecayAndHealParty = true;
        decayConversion.fatigue = 0;
        auto& firstDecaySetup = decayConversionContent.abilities.at(soma::gameplay::AbilityId{"ability.simon.0"});
        firstDecaySetup.targeting = {soma::content::TargetRelation::opposingTeam, soma::content::TargetScope::single, 1, false};
        firstDecaySetup.hostileEffect = {};
        firstDecaySetup.hostileEffect.decayDamage = 3;
        firstDecaySetup.hostileEffect.decayRounds = 1;
        firstDecaySetup.fatigue = 0;
        MinimumRandom decayConversionRandom;
        soma::simulation::BattleSimulation decayConversionBattle{
            decayConversionContent,
            {{soma::gameplay::ContentDefinitionId{"player.anna"}, 2, 1},
             {soma::gameplay::ContentDefinitionId{"player.simon"}, 0, 1},
             {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}, 4}},
            decayConversionRandom};
        const auto conversionTarget = entity("enemy.gesetzloser", 4);
        const auto firstHealingTarget = entity("player.anna", 2);
        const auto secondHealingTarget = entity("player.simon", 0);
        check(decayConversionBattle.execute(
                  {secondHealingTarget, soma::gameplay::AbilityId{"ability.simon.0"}, conversionTarget})
                  .hasValue());
        const auto decayConversionResult = decayConversionBattle.execute(
            {firstHealingTarget, soma::gameplay::AbilityId{"ability.anna.2"}, conversionTarget});
        check(decayConversionResult.hasValue());
        check(decayConversionBattle.find(conversionTarget)->decay.empty());
        check(decayConversionBattle.find(firstHealingTarget)->health + decayConversionBattle.find(secondHealingTarget)->health == 5);
        check(std::abs(decayConversionBattle.find(firstHealingTarget)->health - decayConversionBattle.find(secondHealingTarget)->health) == 1);

    }
} // namespace soma::tests
