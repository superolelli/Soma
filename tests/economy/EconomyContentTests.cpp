#include "content/BattleContentLoader.hpp"
#include "content/EconomyContentLoader.hpp"
#include "content/RuleContentLoader.hpp"
#include "game/CampaignBattleContentBuilder.hpp"
#include "game/EconomyQueries.hpp"
#include "game/EconomyService.hpp"
#include "game/EquipmentBonusCalculator.hpp"
#include "game/ShopStockGenerator.hpp"
#include "game/RuleBuilderService.hpp"
#include "gameplay/random/SeededRandom.hpp"

#include <array>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <source_location>
#include <string>
#include <random>
#include <vector>

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
} // namespace

int main(int argc, char** argv)
{
    check(argc == 2);
    const auto result = soma::content::EconomyContentLoader{}.load(std::filesystem::absolute(argv[1]));
    const auto rules = soma::content::RuleContentLoader{}.load(std::filesystem::absolute(argv[1]));
    check(result.valid());
    check(rules.valid());
    check(result.content.items.size() == 36);
    const auto& lodgeRing = result.content.items.at(soma::gameplay::ItemId{"item.kutschfahrt.lodge_ring"});
    check(lodgeRing.description == "???");
    check(std::get<soma::gameplay::EquipmentItem>(lodgeRing.properties).battleEffect.has_value());
    check(result.content.skills.size() == 160);
    check(result.content.abilitySets.size() == 16);
    check(result.content.assets.size() == result.content.items.size() + result.content.skills.size() + 81);
    check(result.content.assets.contains(soma::gameplay::AssetId{"font.price"}));
    check(result.content.presentation.resourceCounter.background == soma::gameplay::AssetId{"status_bar.background"});
    check(result.content.presentation.referenceSize == soma::content::Point{1920, 1080});
    check(result.content.presentation.resourceCounter.textSize == 18);
    check(result.content.presentation.inventory.origin == soma::content::Point{143, 69});
    check(result.content.presentation.inventory.equipmentFields[1] == soma::content::Point{365, 0});
    check(result.content.presentation.inventory.itemsScrollbar.width == 35);
    check(result.content.presentation.shop.dicePriceLock == soma::content::Point{685, 255});
    check(result.content.presentation.shop.confirmationYesButton == soma::content::Point{40, 132});
    check(result.content.presentation.skills.skillIcons[4] == soma::content::Point{106, 453});
    check(result.content.presentation.skills.playerNameBounds.width == 162);
    check(result.content.presentation.skills.abilityBridges[1] == soma::content::Point{525, 328});
    check(result.content.presentation.skills.selectedSkillTitleBounds.width == 555);
    check(result.content.presentation.consumableDrawer.openPosition == soma::content::Point{1200, 600});
    check(result.content.presentation.explorationConsumables.position == soma::content::Point{1200, 855});
    check(result.content.presentation.skills.connectionTypes[3] == 2);

    const auto& firstSkill = result.content.skills.at(soma::gameplay::SkillId{"skill.simon.0.1"});
    check(firstSkill.name == "Verbesserte Heilung I");
    check(firstSkill.description.find("Heilt Simon") != std::string::npos);
    check(firstSkill.cost == 1);
    check(firstSkill.upgrade.effects.size() == 1);
    const auto& secondSkill = result.content.skills.at(soma::gameplay::SkillId{"skill.simon.0.2"});
    check(secondSkill.prerequisites.empty());
    const auto& thirdSkill = result.content.skills.at(soma::gameplay::SkillId{"skill.simon.0.3"});
    check(thirdSkill.prerequisites == std::vector{soma::gameplay::SkillId{"skill.simon.0.1"}});
    const auto& markedDamageSkill = result.content.skills.at(soma::gameplay::SkillId{"skill.simon.2.alternative.2"});
    check(markedDamageSkill.upgrade.effects.front().effect.damageAgainstMarked == 0.2);
    const auto& healingReceivedSkill = result.content.skills.at(soma::gameplay::SkillId{"skill.simon.3.2"});
    check(healingReceivedSkill.upgrade.effects.front().effect.targetBuff.healingReceived == 0.25);
    const auto& globalMarkedDamageSkill = result.content.skills.at(soma::gameplay::SkillId{"skill.simon.3.alternative.5"});
    check(globalMarkedDamageSkill.upgrade.effects.front().target == soma::content::SkillEffectTarget::allCombatants);
    const auto& focusedMockerySkill = result.content.skills.at(soma::gameplay::SkillId{"skill.simon.1.5"});
    check(focusedMockerySkill.upgrade.effects.front().effect.actorBuff.duration == 3);
    check(focusedMockerySkill.upgrade.effects.front().effect.actorBuff.attributes.precision == 20);
    check(focusedMockerySkill.upgrade.effects.front().effect.targetBuff.duration == 0);
    const auto& fewerTargetsSkill = result.content.skills.at(soma::gameplay::SkillId{"skill.ole.0.5"});
    check(fewerTargetsSkill.upgrade.effects.front().effect.lessTargetsMoreDamage == 0.1);
    const auto& preciseKickSkill = result.content.skills.at(soma::gameplay::SkillId{"skill.ole.1.2"});
    check(preciseKickSkill.upgrade.precisionModifier == 20);
    const auto& focusedPoetrySkill = result.content.skills.at(soma::gameplay::SkillId{"skill.ole.3.5"});
    check(focusedPoetrySkill.upgrade.effects.front().effect.actorBuff.duration > 0);
    const auto& acceleratedDecaySkill = result.content.skills.at(soma::gameplay::SkillId{"skill.ole.0.alternative.5"});
    check(acceleratedDecaySkill.upgrade.effects.front().effect.inflictAllDecayAndRemove);
    const auto& unavoidableDecaySkill = result.content.skills.at(soma::gameplay::SkillId{"skill.ole.1.5"});
    check(unavoidableDecaySkill.upgrade.effects.front().effect.ignoreDecayResistance);
    const auto& transferredDecaySkill = result.content.skills.at(soma::gameplay::SkillId{"skill.ole.1.alternative.5"});
    check(transferredDecaySkill.upgrade.effects.front().effect.transferDecayOnKill);
    const auto& randomStatusSkill = result.content.skills.at(soma::gameplay::SkillId{"skill.ole.2.3"});
    check(randomStatusSkill.upgrade.effects.front().effect.randomNegativeStatusProbability == 0.15);
    check(randomStatusSkill.upgrade.effects.front().effect.randomNegativeStatusDuration == 2);
    const auto& fatigueDamageSkill = result.content.skills.at(soma::gameplay::SkillId{"skill.ole.2.alternative.1"});
    check(fatigueDamageSkill.upgrade.effects.front().effect.targetBuff.damageFromFatigue);
    check(fatigueDamageSkill.upgrade.effects.front().effect.targetBuff.duration == 2);
    const auto& firstFatigueBlockSkill = result.content.skills.at(soma::gameplay::SkillId{"skill.ole.2.alternative.4"});
    check(firstFatigueBlockSkill.upgrade.effects.front().effect.targetBuff.blocksFatigueStageOne);
    const auto& painfulWordsSkill = result.content.skills.at(soma::gameplay::SkillId{"skill.ole.3.2"});
    check(painfulWordsSkill.upgrade.effects.front().effect.damageFactor == 0.2);
    const auto& adrenalineSkill = result.content.skills.at(soma::gameplay::SkillId{"skill.ole.3.alternative.1"});
    check(adrenalineSkill.upgrade.fatigue == -6);
    const auto& failedSleepSkill = result.content.skills.at(soma::gameplay::SkillId{"skill.ole.3.alternative.4"});
    check(failedSleepSkill.upgrade.effects.front().effect.sleepFailureDecayDamage == 1);
    check(failedSleepSkill.upgrade.effects.front().effect.sleepFailureDecayRounds == 4);
    const auto& largerAudienceSkill = result.content.skills.at(soma::gameplay::SkillId{"skill.ole.3.alternative.5"});
    check(largerAudienceSkill.upgrade.additionalTargets == 1);
    const auto& markedDecaySkill = result.content.skills.at(soma::gameplay::SkillId{"skill.anna.2.3"});
    check(markedDecaySkill.upgrade.effects.front().effect.decayDamageAgainstMarked == 1);
    const auto& weaknessSkill = result.content.skills.at(soma::gameplay::SkillId{"skill.anna.2.4"});
    check(weaknessSkill.upgrade.effects.front().effect.decayRoundsAgainstConfused == 2);
    check(weaknessSkill.upgrade.effects.front().effect.decayRoundsAgainstSleeping == 2);
    const auto& decayConversionSkill = result.content.skills.at(soma::gameplay::SkillId{"skill.anna.2.5"});
    check(decayConversionSkill.upgrade.effects.front().effect.removeDecayAndHealParty);
    const auto& transfusionSkill = result.content.skills.at(soma::gameplay::SkillId{"skill.anna.3.alternative.2"});
    check(transfusionSkill.upgrade.effects.front().effect.transfusionHealing == 1);
    const auto& transferredHealthSkill = result.content.skills.at(soma::gameplay::SkillId{"skill.markus.1.1"});
    check(transferredHealthSkill.upgrade.effects.front().effect.transferredMissingHealthDamageFactor == 0.25);
    check(transferredHealthSkill.upgrade.effects.front().effect.transferredMissingHealthHealingFactor == 0.25);
    const auto& randomMovementSkill = result.content.skills.at(soma::gameplay::SkillId{"skill.markus.1.alternative.4"});
    check(randomMovementSkill.upgrade.effects.front().effect.moveTargetsRandomly);
    const auto& reducedFalloffSkill = result.content.skills.at(soma::gameplay::SkillId{"skill.markus.1.alternative.2"});
    check(reducedFalloffSkill.upgrade.effects.front().effect.successiveTargetDamageReduction == -0.25);
    const auto& panicDamageSkill = result.content.skills.at(soma::gameplay::SkillId{"skill.markus.3.3"});
    check(panicDamageSkill.upgrade.effects.front().effect.damageFromPanic);
    const auto& spreadingPanicSkill = result.content.skills.at(soma::gameplay::SkillId{"skill.markus.3.4"});
    check(spreadingPanicSkill.upgrade.effects.front().effect.spreadPanicTargets == 1);
    const auto& negativeStatusDamageSkill =
        result.content.skills.at(soma::gameplay::SkillId{"skill.markus.3.alternative.4"});
    check(negativeStatusDamageSkill.upgrade.effects.front().effect.targetBuff.damagePerNegativeStatus == -0.05);

    const auto& ironPlate = result.content.items.at(soma::gameplay::ItemId{"item.bang.iron_plate"});
    check(ironPlate.name == "Eisenplatte");
    check(ironPlate.price == 80);
    check(std::get<soma::gameplay::EquipmentItem>(ironPlate.properties).bonuses.armour == 10);

    const auto& brokenMirror = result.content.items.at(soma::gameplay::ItemId{"item.kutschfahrt.broken_mirror"});
    check(std::get<soma::gameplay::EquipmentItem>(brokenMirror.properties).healsOnPass);

    const auto& flashPowder = result.content.items.at(soma::gameplay::ItemId{"item.kutschfahrt.flash_powder"});
    const auto& consumable = std::get<soma::gameplay::ConsumableItem>(flashPowder.properties);
    check(consumable.duration == 10);
    check(consumable.temporaryBonuses.damageMinimum == 2);
    check(consumable.temporaryBonuses.damageMaximum == 2);

    const auto& dice = result.content.items.at(soma::gameplay::ItemId{"currency.dice"});
    check(std::get<soma::gameplay::CurrencyItem>(dice.properties).currency == soma::gameplay::Currency::dice);

    soma::game::CampaignState state;
    soma::game::EconomyService economy{state};
    check(economy.grant(ironPlate, 1, {20, 30, 40}).hasValue());
    const soma::game::EconomyQueries queries{state, result.content};
    const auto inventory = queries.inventory(soma::game::ItemSort::name);
    check(inventory.size() == 1);
    check(inventory.front().name == "Eisenplatte");
    check(inventory.front().salePrice == 16);
    check(inventory.front().item.color == soma::gameplay::ItemColor{});

    const std::vector stock{soma::gameplay::ItemInstance{ironPlate.id, {}}, soma::gameplay::ItemInstance{flashPowder.id, {}}};
    const auto offers = queries.shopOffers(stock);
    check(offers.size() == 2);
    check(offers[0].available && offers[0].affordable);
    check(!offers[1].available);

    const auto skills = queries.skills(soma::gameplay::ContentDefinitionId{"player.simon"}, 0, 0);
    check(skills.size() == 5);
    check(skills[0].availability == soma::game::SkillAvailability::available);
    check(skills[1].availability == soma::game::SkillAvailability::available);
    check(skills[2].availability == soma::game::SkillAvailability::locked);
    const auto alternativeSkills = queries.skills(soma::gameplay::ContentDefinitionId{"player.simon"}, 0, 1);
    check(alternativeSkills.size() == 5);
    check(alternativeSkills.front().skill == soma::gameplay::SkillId{"skill.simon.0.alternative.1"});

    check(result.content.diamondBonuses.size() == 18);
    soma::game::CampaignState colorlessState;
    soma::game::EconomyService colorlessEconomy{colorlessState};
    check(colorlessEconomy.grant(ironPlate, 2, {255, 255, 255}).hasValue());
    const std::array colorlessSlots{"north", "east"};
    for (const auto slot : colorlessSlots)
    {
        check(colorlessEconomy.equip({soma::gameplay::ContentDefinitionId{"player.simon"},
                                      soma::gameplay::EquipmentSlotId{"equipment." + std::string{slot}}},
                                     colorlessState.inventory().front())
                  .hasValue());
    }
    const auto colorlessSummary = soma::game::EquipmentBonusCalculator{result.content}.calculate(
        colorlessState, soma::gameplay::ContentDefinitionId{"player.simon"});
    check(std::ranges::none_of(colorlessSummary.connections, &soma::game::EquipmentConnection::active));
    check(colorlessSummary.diamondColor == soma::gameplay::ItemColor{});

    soma::game::CampaignState bonusState;
    soma::game::EconomyService bonusEconomy{bonusState};
    check(bonusEconomy.grant(ironPlate, 4, {128, 0, 0}).hasValue());
    const std::array slotNames{"north", "east", "south", "west"};
    for (const auto slot : slotNames)
    {
        const auto item = bonusState.inventory().front();
        check(bonusEconomy.equip({soma::gameplay::ContentDefinitionId{"player.simon"},
                                  soma::gameplay::EquipmentSlotId{"equipment." + std::string{slot}}},
                                 item)
                  .hasValue());
    }
    const auto equipmentSummary = soma::game::EquipmentBonusCalculator{result.content}.calculate(
        bonusState, soma::gameplay::ContentDefinitionId{"player.simon"});
    check(equipmentSummary.diamondColor == soma::gameplay::ItemColor{255, 0, 0});
    check(equipmentSummary.diamondBonuses.damageMinimum == 16);
    check(equipmentSummary.combinedBonuses.armour == 40);

    const auto battleResult = soma::content::BattleContentLoader{}.load(std::filesystem::absolute(argv[1]));
    check(battleResult.valid());
    check(bonusEconomy.grant(dice, 1).hasValue());
    check(bonusEconomy.acquireSkill(firstSkill.id, firstSkill.cost, firstSkill.prerequisites).hasValue());
    const auto campaignBattle =
        soma::game::CampaignBattleContentBuilder{}.build(battleResult.content, result.content, bonusState);
    const auto& baseSimon = battleResult.content.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"});
    const auto& equippedSimon = campaignBattle.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"});
    check(equippedSimon.attributes.armour == baseSimon.attributes.armour + 40);
    const auto& baseFlash = battleResult.content.abilities.at(soma::gameplay::AbilityId{"ability.simon.0"});
    const auto& skilledFlash = campaignBattle.abilities.at(soma::gameplay::AbilityId{"ability.simon.0"});
    check(skilledFlash.friendlyEffect.heal == baseFlash.friendlyEffect.heal + 4);
    check(skilledFlash.hostileEffect.heal == baseFlash.hostileEffect.heal + 4);

    soma::game::CampaignState alternativeState;
    soma::game::EconomyService alternativeEconomy{alternativeState};
    check(alternativeEconomy.grant(dice, 1).hasValue());
    const auto& alternativeSkill = result.content.skills.at(soma::gameplay::SkillId{"skill.simon.0.alternative.1"});
    check(alternativeEconomy.acquireSkill(alternativeSkill.id, alternativeSkill.cost, alternativeSkill.prerequisites).hasValue());
    check(alternativeEconomy.selectAbilityVersion(soma::gameplay::ContentDefinitionId{"player.simon"}, 0, 1).hasValue());
    const auto alternativeBattle =
        soma::game::CampaignBattleContentBuilder{}.build(battleResult.content, result.content, alternativeState);
    check(alternativeBattle.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"}).abilities.front() ==
          soma::gameplay::AbilityId{"ability.simon.0.alternative"});
    const auto& alternativeFlash = alternativeBattle.abilities.at(soma::gameplay::AbilityId{"ability.simon.0.alternative"});
    const auto& baseAlternativeFlash = battleResult.content.abilities.at(soma::gameplay::AbilityId{"ability.simon.0.alternative"});
    check(alternativeFlash.friendlyEffect.actorBuff.duration == baseAlternativeFlash.friendlyEffect.actorBuff.duration + 2);

    soma::game::CampaignState oleTooltipState;
    soma::game::EconomyService oleTooltipEconomy{oleTooltipState};
    check(oleTooltipEconomy.grant(dice, 20).hasValue());
    for (const auto* skillId : {"skill.ole.0.1", "skill.ole.0.2", "skill.ole.0.3", "skill.ole.0.4", "skill.ole.0.5"})
    {
        const auto& skill = result.content.skills.at(soma::gameplay::SkillId{skillId});
        check(oleTooltipEconomy.acquireSkill(skill.id, skill.cost, skill.prerequisites).hasValue());
    }
    check(oleTooltipEconomy.acquireSkill(preciseKickSkill.id, preciseKickSkill.cost, preciseKickSkill.prerequisites).hasValue());
    const auto oleTooltipBattle =
        soma::game::CampaignBattleContentBuilder{}.build(battleResult.content, result.content, oleTooltipState);
    const auto& skilledGummiArm = oleTooltipBattle.abilities.at(soma::gameplay::AbilityId{"ability.ole.0"});
    check(skilledGummiArm.hostileEffect.lessTargetsMoreDamage == 0.1);
    check(skilledGummiArm.targeting.targetCount == 4);
    const auto& skilledKick = oleTooltipBattle.abilities.at(soma::gameplay::AbilityId{"ability.ole.1"});
    check(skilledKick.precisionModifier == 20);

    soma::game::CampaignState combinedBuffState;
    soma::game::EconomyService combinedBuffEconomy{combinedBuffState};
    check(combinedBuffEconomy.grant(dice, 20).hasValue());
    for (const auto* skillId : {"skill.simon.3.1", "skill.simon.3.2", "skill.simon.3.3"})
    {
        const auto& skill = result.content.skills.at(soma::gameplay::SkillId{skillId});
        check(combinedBuffEconomy.acquireSkill(skill.id, skill.cost, skill.prerequisites).hasValue());
    }
    const auto combinedBuffBattle =
        soma::game::CampaignBattleContentBuilder{}.build(battleResult.content, result.content, combinedBuffState);
    const auto& combinedBuffAbility = combinedBuffBattle.abilities.at(soma::gameplay::AbilityId{"ability.simon.3"});
    check(combinedBuffAbility.friendlyEffect.targetBuff.duration == 4);
    check(combinedBuffAbility.friendlyEffect.targetBuff.attributes.armour == 40);
    check(combinedBuffAbility.friendlyEffect.targetBuff.healingReceived == 0.25);

    soma::game::CampaignState globalBuffState;
    soma::game::EconomyService globalBuffEconomy{globalBuffState};
    check(globalBuffEconomy.grant(dice, 20).hasValue());
    for (const auto* skillId : {"skill.simon.3.alternative.1",
                                "skill.simon.3.alternative.2",
                                "skill.simon.3.alternative.3",
                                "skill.simon.3.alternative.4",
                                "skill.simon.3.alternative.5"})
    {
        const auto& skill = result.content.skills.at(soma::gameplay::SkillId{skillId});
        check(globalBuffEconomy.acquireSkill(skill.id, skill.cost, skill.prerequisites).hasValue());
    }
    check(globalBuffEconomy.selectAbilityVersion(soma::gameplay::ContentDefinitionId{"player.simon"}, 3, 1).hasValue());
    const auto globalBuffBattle = soma::game::CampaignBattleContentBuilder{}.build(battleResult.content, result.content, globalBuffState);
    const auto& globalBuffAbility = globalBuffBattle.abilities.at(soma::gameplay::AbilityId{"ability.simon.3.alternative"});
    check(globalBuffAbility.hostileEffect.targetBuff.duration == 4);
    check(globalBuffAbility.hostileEffect.targetBuff.attributes.armour == -20);
    check(globalBuffAbility.hostileEffect.targetBuff.attributes.dodge == -20);
    check(globalBuffAbility.allCombatantsBuff.duration == 4);
    check(globalBuffAbility.allCombatantsBuff.damageAgainstMarked == 0.2);

    soma::game::CampaignState panicSkillState;
    soma::game::EconomyService panicSkillEconomy{panicSkillState};
    check(panicSkillEconomy.grant(dice, 20).hasValue());
    for (const auto* skillId : {"skill.markus.3.alternative.1",
                                "skill.markus.3.alternative.2",
                                "skill.markus.3.alternative.3",
                                "skill.markus.3.alternative.4",
                                "skill.markus.3.alternative.5"})
    {
        const auto& skill = result.content.skills.at(soma::gameplay::SkillId{skillId});
        check(panicSkillEconomy.acquireSkill(skill.id, skill.cost, skill.prerequisites).hasValue());
    }
    check(panicSkillEconomy.selectAbilityVersion(soma::gameplay::ContentDefinitionId{"player.markus"}, 3, 1).hasValue());
    const auto panicSkillBattle =
        soma::game::CampaignBattleContentBuilder{}.build(battleResult.content, result.content, panicSkillState);
    const auto& panicSkillAbility = panicSkillBattle.abilities.at(soma::gameplay::AbilityId{"ability.markus.3.alternative"});
    check(panicSkillAbility.hostileEffect.panicStacks == 2);
    check(panicSkillAbility.hostileEffect.panicAgainstMarked == 3);
    check(panicSkillAbility.hostileEffect.targetBuff.duration == 4);
    check(panicSkillAbility.hostileEffect.targetBuff.attributes.healing == -40);
    check(panicSkillAbility.hostileEffect.targetBuff.damagePerNegativeStatus == -0.1);

    soma::game::CampaignState collisionSkillState;
    soma::game::EconomyService collisionSkillEconomy{collisionSkillState};
    check(collisionSkillEconomy.grant(dice, 20).hasValue());
    for (const auto* skillId : {"skill.markus.1.alternative.1",
                                "skill.markus.1.alternative.2",
                                "skill.markus.1.alternative.3",
                                "skill.markus.1.alternative.4",
                                "skill.markus.1.alternative.5"})
    {
        const auto& skill = result.content.skills.at(soma::gameplay::SkillId{skillId});
        check(collisionSkillEconomy.acquireSkill(skill.id, skill.cost, skill.prerequisites).hasValue());
    }
    check(collisionSkillEconomy.selectAbilityVersion(soma::gameplay::ContentDefinitionId{"player.markus"}, 1, 1).hasValue());
    const auto collisionSkillBattle =
        soma::game::CampaignBattleContentBuilder{}.build(battleResult.content, result.content, collisionSkillState);
    const auto& collisionSkillAbility =
        collisionSkillBattle.abilities.at(soma::gameplay::AbilityId{"ability.markus.1.alternative"});
    check(collisionSkillAbility.hostileEffect.successiveTargetDamageReduction == 0.25);
    check(collisionSkillAbility.hostileEffect.moveTargetsRandomly);

    soma::game::CampaignState passiveState;
    soma::game::EconomyService passiveEconomy{passiveState};
    const auto& sombrero = result.content.items.at(soma::gameplay::ItemId{"item.bang.sombrero"});
    check(passiveEconomy.grant(sombrero).hasValue());
    check(passiveEconomy.equip({soma::gameplay::ContentDefinitionId{"player.simon"},
                                soma::gameplay::EquipmentSlotId{"equipment.north"}},
                               passiveState.inventory().front())
              .hasValue());
    const auto passiveBattle =
        soma::game::CampaignBattleContentBuilder{}.build(battleResult.content, result.content, passiveState);
    check(passiveBattle.combatants.at(soma::gameplay::ContentDefinitionId{"player.simon"}).addsMissOnHighDamage);

    soma::game::CampaignState lodgeState;
    soma::game::EconomyService lodgeEconomy{lodgeState};
    check(lodgeEconomy.grant(lodgeRing).hasValue());
    check(lodgeEconomy.equip({soma::gameplay::ContentDefinitionId{"player.anna"},
                              soma::gameplay::EquipmentSlotId{"equipment.north"}},
                             lodgeState.inventory().front())
              .hasValue());
    const auto lodgeBattle = soma::game::CampaignBattleContentBuilder{}.build(battleResult.content, result.content, lodgeState);
    check(lodgeBattle.combatants.at(soma::gameplay::ContentDefinitionId{"player.anna"}).equipmentEffects.size() == 1);

    std::mt19937 random{42};
    const auto generatedStock = soma::game::ShopStockGenerator{random}.generate(result.content, rules.content, passiveState, "bang", 5);
    check(generatedStock.size() == 5);
    for (const auto& offer : generatedStock)
    {
        check(offer.item.has_value());
        const auto& item = *offer.item;
        check(result.content.items.at(item.definition).category == soma::gameplay::ItemCategory::equipment);
        const auto validColor = item.color == soma::gameplay::ItemColor{} || item.color == soma::gameplay::ItemColor{128, 0, 0} ||
                                item.color == soma::gameplay::ItemColor{0, 128, 0} || item.color == soma::gameplay::ItemColor{0, 0, 128} ||
                                item.color == soma::gameplay::ItemColor{128, 128, 0};
        check(validColor);
    }

    std::mt19937 rarityRandom{4242};
    soma::game::ShopStockGenerator rarityStockGenerator{rarityRandom};
    soma::content::EconomyContent rarityEconomy;
    for (auto level = 1; level <= 3; ++level)
    {
        auto definition = sombrero;
        definition.id = soma::gameplay::ItemId{"item.test.rarity." + std::to_string(level)};
        definition.level = level;
        definition.campaigns = {"rarity-test"};
        rarityEconomy.items.emplace(definition.id, std::move(definition));
    }
    std::array<int, 4> rarityCounts{};
    for (auto iteration = 0; iteration < 5'000; ++iteration)
    {
        const auto generatedRarityStock =
            rarityStockGenerator.generate(rarityEconomy, rules.content, passiveState, "rarity-test", 1);
        check(generatedRarityStock.size() == 1);
        check(generatedRarityStock.front().item.has_value());
        ++rarityCounts.at(
            static_cast<std::size_t>(rarityEconomy.items.at(generatedRarityStock.front().item->definition).level));
    }
    check(rarityCounts.at(1) > 3'250 && rarityCounts.at(1) < 3'750);
    check(rarityCounts.at(2) > 1'050 && rarityCounts.at(2) < 1'450);
    check(rarityCounts.at(3) > 150 && rarityCounts.at(3) < 350);

    soma::game::RuleBuilderService ruleService{passiveState, rules.content};
    check(ruleService.grant(soma::gameplay::RuleFragmentId{"rule.trigger.health_lost"}).hasValue());
    check(ruleService.grant(soma::gameplay::RuleFragmentId{"rule.subject.this_combatant"}).hasValue());
    std::mt19937 mixedRandom{43};
    const auto mixedStock =
        soma::game::ShopStockGenerator{mixedRandom}.generate(result.content, rules.content, passiveState, "bang", 5);
    check(std::ranges::count_if(mixedStock, [](const auto& offer) { return offer.fragment.has_value(); }) == 2);

}
