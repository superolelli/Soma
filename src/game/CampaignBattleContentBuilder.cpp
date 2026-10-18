#include "game/CampaignBattleContentBuilder.hpp"

#include "game/EquipmentBonusCalculator.hpp"

#include <algorithm>
#include <stdexcept>

namespace soma::game
{
    namespace
    {
        void add(content::AttributeDelta& destination, const content::AttributeDelta& source)
        {
            destination.armour += source.armour;
            destination.damageMinimum += source.damageMinimum;
            destination.damageMaximum += source.damageMaximum;
            destination.initiative += source.initiative;
            destination.maximumHealth += source.maximumHealth;
            destination.criticalHitChance += source.criticalHitChance;
            destination.precision += source.precision;
            destination.dodge += source.dodge;
            destination.healing += source.healing;
            destination.sleepResistance += source.sleepResistance;
            destination.confusionResistance += source.confusionResistance;
            destination.debuffResistance += source.debuffResistance;
            destination.decayResistance += source.decayResistance;
        }

        bool hasBuffPayload(const content::BuffDefinition& buff)
        {
            return buff.attributes != content::AttributeDelta{} || buff.healingReceived != 0.0 || buff.damageAgainstMarked != 0.0 ||
                   buff.damagePerNegativeStatus != 0.0 || buff.damageFromFatigue || buff.blocksFatigueStageOne ||
                   buff.blocksFatigueStageTwo;
        }

        void mergeBuff(content::BuffDefinition& destination, const content::BuffDefinition& source)
        {
            const auto destinationWasDefined = destination.duration > 0 || hasBuffPayload(destination);
            const auto sourceHasPayload = hasBuffPayload(source);
            if (!destinationWasDefined)
            {
                destination.positive = source.positive;
            }

            add(destination.attributes, source.attributes);
            destination.healingReceived += source.healingReceived;
            destination.damageAgainstMarked += source.damageAgainstMarked;
            destination.damagePerNegativeStatus += source.damagePerNegativeStatus;
            destination.damageFromFatigue = destination.damageFromFatigue || source.damageFromFatigue;
            destination.blocksFatigueStageOne = destination.blocksFatigueStageOne || source.blocksFatigueStageOne;
            destination.blocksFatigueStageTwo = destination.blocksFatigueStageTwo || source.blocksFatigueStageTwo;

            if (source.duration <= 0)
            {
                return;
            }
            if (sourceHasPayload)
            {
                destination.duration = std::max(destination.duration, source.duration);
            }
            else
            {
                destination.duration += source.duration;
            }
        }

        void applyEffect(content::AbilityEffect& destination, const content::AbilityEffect& source)
        {
            destination.criticalHitModifier += source.criticalHitModifier;
            destination.lessTargetsMoreDamage += source.lessTargetsMoreDamage;
            destination.damageFactor += source.damageFactor;
            destination.damageAgainstMarked += source.damageAgainstMarked;
            destination.damageAgainstRevengeMarked += source.damageAgainstRevengeMarked;
            destination.successiveTargetDamageReduction += source.successiveTargetDamageReduction;
            destination.transferredMissingHealthDamageFactor += source.transferredMissingHealthDamageFactor;
            destination.transferredMissingHealthHealingFactor += source.transferredMissingHealthHealingFactor;
            destination.heal += source.heal;
            destination.healSelf += source.healSelf;
            destination.targetFatigue += source.targetFatigue;
            destination.confusionProbability += source.confusionProbability;
            destination.confusionRounds += source.confusionRounds;
            destination.decayDamage += source.decayDamage;
            destination.decayRounds += source.decayRounds;
            destination.decayDamageAgainstMarked += source.decayDamageAgainstMarked;
            destination.decayRoundsAgainstConfused += source.decayRoundsAgainstConfused;
            destination.decayRoundsAgainstSleeping += source.decayRoundsAgainstSleeping;
            destination.removeDecayAndHealParty = destination.removeDecayAndHealParty || source.removeDecayAndHealParty;
            destination.markRounds += source.markRounds;
            destination.sleepProbability += source.sleepProbability;
            destination.removeBuffs = destination.removeBuffs || source.removeBuffs;
            destination.removeDebuffs = destination.removeDebuffs || source.removeDebuffs;
            destination.stealBuffs = destination.stealBuffs || source.stealBuffs;
            destination.addMiss += source.addMiss;
            destination.addMissSelf += source.addMissSelf;
            destination.dynamite = destination.dynamite || source.dynamite;
            destination.bountyRounds += source.bountyRounds;
            destination.riposteRounds += source.riposteRounds;
            destination.riposteSelfRounds += source.riposteSelfRounds;
            destination.rageRounds += source.rageRounds;
            destination.hackedRounds += source.hackedRounds;
            destination.transfusionHealing += source.transfusionHealing;
            destination.panicStacks += source.panicStacks;
            destination.panicAgainstMarked += source.panicAgainstMarked;
            destination.spreadPanicTargets += source.spreadPanicTargets;
            destination.damageFromPanic = destination.damageFromPanic || source.damageFromPanic;
            destination.doublePanic = destination.doublePanic || source.doublePanic;
            destination.moveTargetsRandomly = destination.moveTargetsRandomly || source.moveTargetsRandomly;
            destination.ignoreArmour = destination.ignoreArmour || source.ignoreArmour;
            destination.ignoreDecayResistance = destination.ignoreDecayResistance || source.ignoreDecayResistance;
            destination.inflictAllDecayAndRemove = destination.inflictAllDecayAndRemove || source.inflictAllDecayAndRemove;
            destination.transferDecayOnKill = destination.transferDecayOnKill || source.transferDecayOnKill;
            destination.randomNegativeStatusProbability += source.randomNegativeStatusProbability;
            destination.randomNegativeStatusDuration = std::max(destination.randomNegativeStatusDuration, source.randomNegativeStatusDuration);
            destination.prolongNegativeEffects += source.prolongNegativeEffects;
            destination.increaseSleepProbabilityByFatigue =
                destination.increaseSleepProbabilityByFatigue || source.increaseSleepProbabilityByFatigue;
            destination.sleepFailureDecayDamage += source.sleepFailureDecayDamage;
            destination.sleepFailureDecayRounds += source.sleepFailureDecayRounds;
            mergeBuff(destination.targetBuff, source.targetBuff);
            mergeBuff(destination.actorBuff, source.actorBuff);
        }

        void applyUpgrade(content::AbilityDefinition& ability, const content::AbilityUpgrade& upgrade)
        {
            ability.targeting.targetCount += upgrade.additionalTargets;
            if (ability.targeting.targetCount > 1 && ability.targeting.scope == content::TargetScope::single)
            {
                ability.targeting.scope = content::TargetScope::count;
            }
            ability.precisionModifier += upgrade.precisionModifier;
            ability.fatigue += upgrade.fatigue;
            for (const auto& modifier : upgrade.effects)
            {
                auto effect = modifier.effect;
                if (modifier.target == content::SkillEffectTarget::allCombatants)
                {
                    mergeBuff(ability.allCombatantsBuff, effect.targetBuff);
                    continue;
                }
                if (ability.targeting.relation == content::TargetRelation::self)
                {
                    mergeBuff(effect.actorBuff, effect.targetBuff);
                    effect.targetBuff = {};
                }
                if (modifier.target != content::SkillEffectTarget::hostile)
                {
                    applyEffect(ability.friendlyEffect, effect);
                }
                if (modifier.target != content::SkillEffectTarget::friendly)
                {
                    applyEffect(ability.hostileEffect, effect);
                }
            }
        }

        void applyEquipment(content::CombatantDefinition& combatant, const EquipmentBonusSummary& summary)
        {
            const auto& bonus = summary.combinedBonuses;
            combatant.attributes.armour += bonus.armour;
            combatant.attributes.maximumHealth += bonus.maximumHealth;
            combatant.attributes.damageMinimum += bonus.damageMinimum;
            combatant.attributes.damageMaximum += bonus.damageMaximum;
            combatant.attributes.initiative += bonus.initiative;
            combatant.attributes.criticalHitChance += bonus.criticalHitChance;
            combatant.attributes.dodge += bonus.dodge;
            combatant.attributes.precision += bonus.precision;
            combatant.resistances.healing += bonus.healing;
            combatant.resistances.sleepResistance += bonus.sleepResistance;
            combatant.resistances.confusionResistance += bonus.confusionResistance;
            combatant.resistances.debuffResistance += bonus.debuffResistance;
            combatant.resistances.decayResistance += bonus.decayResistance;
            combatant.addsMissOnHighDamage = summary.addsMissOnHighDamage;
            combatant.healsOnPass = summary.healsOnPass;
            combatant.equipmentEffects = summary.battleEffects;
        }
    } // namespace

    content::BattleContent CampaignBattleContentBuilder::build(const content::BattleContent& battleContent,
                                                                const content::EconomyContent& economyContent,
                                                                const CampaignState& state) const
    {
        auto result = battleContent;
        for (const auto& set : economyContent.abilitySets)
        {
            const auto selectedVersion = state.activeAbilityVersion(set.player, set.slot);
            const auto& version = set.versions.at(selectedVersion);
            auto& combatant = result.combatants.at(set.player);
            if (set.slot >= combatant.abilities.size())
            {
                throw std::runtime_error("Ability set slot is not present on its combatant");
            }
            combatant.abilities.at(set.slot) = version.ability;
            auto& ability = result.abilities.at(version.ability);
            for (const auto& skillId : version.skills)
            {
                if (state.skillAcquired(skillId)) applyUpgrade(ability, economyContent.skills.at(skillId).upgrade);
            }
        }
        const EquipmentBonusCalculator calculator{economyContent};
        for (auto& [id, combatant] : result.combatants)
        {
            if (combatant.team == gameplay::Team::players)
            {
                applyEquipment(combatant, calculator.calculate(state, id));
            }
        }
        return result;
    }
} // namespace soma::game
