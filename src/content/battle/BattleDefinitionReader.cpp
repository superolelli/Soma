#include "content/battle/BattleContentReader.hpp"
#include "content/JsonFileReader.hpp"

#include <json.hpp>
#include <stdexcept>
#include <string_view>

namespace soma::content
{
    using json = nlohmann::json;

    namespace
    {
        int integer(const json& value, const char* name, int fallback = 0)
        {
            return value.contains(name) ? value.at(name).get<int>() : fallback;
        }

        double number(const json& value, const char* name, double fallback = 0.0)
        {
            return value.contains(name) ? value.at(name).get<double>() : fallback;
        }

        bool boolean(const json& value, const char* name, bool fallback = false)
        {
            return value.contains(name) ? value.at(name).get<bool>() : fallback;
        }

        FatigueTarget readFatigueTarget(std::string_view value)
        {
            if (value == "actor") return FatigueTarget::actor;
            if (value == "targets") return FatigueTarget::targets;
            if (value == "all_players") return FatigueTarget::allPlayers;
            if (value == "random_player") return FatigueTarget::randomPlayer;
            throw std::runtime_error("Unknown fatigue target");
        }

        AbilityEffect readEffect(const json& value)
        {
            AbilityEffect effect;
            effect.criticalHitModifier = integer(value, "criticalHit");
            effect.lessTargetsMoreDamage = number(value, "lessTargetsMoreDamage");
            effect.damageFactor = number(value, "damageFactor");
            effect.damageAgainstMarked = number(value, "damageAgainstMarked");
            effect.damageAgainstRevengeMarked = number(value, "damageAgainstRevengeMarked");
            effect.successiveTargetDamageReduction = number(value, "lessDamageForEachSuccessiveTarget");
            if (value.contains("transferHealthToNext"))
            {
                const auto& transfer = value.at("transferHealthToNext");
                effect.transferredMissingHealthDamageFactor = number(transfer, "shareOfMissingHealthAsDamage");
                effect.transferredMissingHealthHealingFactor = number(transfer, "shareOfMissingHealthAsHeal");
            }
            effect.heal = integer(value, "heal");
            effect.healSelf = integer(value, "healSelf");
            effect.targetFatigue = integer(value, "fatigue_target");
            effect.markRounds = integer(value, "mark");
            effect.sleepProbability = number(value, "putToSleepProbability");
            effect.removeBuffs = boolean(value, "removeBuffs");
            effect.removeDebuffs = boolean(value, "removeDebuffs");
            effect.stealBuffs = boolean(value, "stealBuffs");
            effect.addMiss = integer(value, "addMiss");
            effect.addMissSelf = integer(value, "addMissSelf");
            effect.dynamite = boolean(value, "dynamite");
            effect.bountyRounds = integer(value, "bounty");
            effect.riposteRounds = integer(value, "riposte");
            effect.riposteSelfRounds = integer(value, "riposteSelf");
            effect.rageRounds = integer(value, "rage");
            effect.hackedRounds = integer(value, "hacked");
            effect.transfusionHealing = integer(value, "transfusion");
            effect.panicStacks = integer(value, "panic");
            effect.panicAgainstMarked = integer(value, "panicAgainstMarked");
            effect.spreadPanicTargets = integer(value, "spreadPanic");
            effect.damageFromPanic = boolean(value, "damageFromPanic");
            effect.doublePanic = boolean(value, "doublePanic");
            effect.moveTargetsRandomly = boolean(value, "moveRandomly");
            effect.ignoreArmour = boolean(value, "ignoreArmour");
            effect.ignoreDecayResistance = boolean(value, "ignoreDecayResistance");
            effect.inflictAllDecayAndRemove = boolean(value, "inflictAllDecayAndRemove");
            effect.transferDecayOnKill = boolean(value, "transferDecayOnKill");
            effect.prolongNegativeEffects = integer(value, "prolongNegativeEffects");
            effect.increaseSleepProbabilityByFatigue = boolean(value, "increaseSleepProbabilityByFatigue");
            if (value.contains("randomNegativeStatus"))
            {
                effect.randomNegativeStatusProbability = number(value.at("randomNegativeStatus"), "probability");
                effect.randomNegativeStatusDuration = integer(value.at("randomNegativeStatus"), "duration", 2);
            }
            if (value.contains("onFail") && value.at("onFail").contains("decay"))
            {
                effect.sleepFailureDecayDamage = integer(value.at("onFail").at("decay"), "damage");
                effect.sleepFailureDecayRounds = integer(value.at("onFail").at("decay"), "rounds");
            }

            if (value.contains("confusion"))
            {
                effect.confusionProbability = number(value.at("confusion"), "probability");
                effect.confusionRounds = integer(value.at("confusion"), "rounds");
            }
            if (value.contains("decay"))
            {
                effect.decayDamage = integer(value.at("decay"), "damage");
                effect.decayRounds = integer(value.at("decay"), "rounds");
            }
            if (value.contains("againstMarked") && value.at("againstMarked").contains("decay"))
            {
                effect.decayDamageAgainstMarked = integer(value.at("againstMarked").at("decay"), "damage");
            }
            if (value.contains("againstConfused") && value.at("againstConfused").contains("decay"))
            {
                effect.decayRoundsAgainstConfused = integer(value.at("againstConfused").at("decay"), "rounds");
            }
            if (value.contains("againstSleeping") && value.at("againstSleeping").contains("decay"))
            {
                effect.decayRoundsAgainstSleeping = integer(value.at("againstSleeping").at("decay"), "rounds");
            }
            effect.removeDecayAndHealParty = boolean(value, "removeDecayAndHealParty");
            if (value.contains("buff"))
            {
                const auto& buff = value.at("buff");
                auto& definition = boolean(buff, "onSelf") ? effect.actorBuff : effect.targetBuff;
                definition.duration = integer(buff, "duration");
                definition.positive = boolean(buff, "isPositive", true);
                definition.attributes = detail::readAttributeDelta(buff);
                definition.healingReceived = number(buff, "healingReceived");
                definition.damageAgainstMarked = number(buff, "damageAgainstMarked");
                definition.damagePerNegativeStatus = number(buff, "damagePerNegativeStatus");
                definition.damageFromFatigue = boolean(buff, "damageFromFatigue");
                definition.blocksFatigueStageOne = boolean(buff, "blocksFatigueStageOne");
                definition.blocksFatigueStageTwo = boolean(buff, "blocksFatigueStageTwo");
            }
            return effect;
        }

        void readTargetedEffects(const json& value, AbilityDefinition& ability)
        {
            if (!value.is_array())
            {
                ability.friendlyEffect = readEffect(value);
                ability.hostileEffect = ability.friendlyEffect;
                return;
            }

            for (const auto& targeted : value)
            {
                if (targeted.value("target", std::string{}) == "friend")
                {
                    ability.friendlyEffect = readEffect(targeted);
                }
                else
                {
                    ability.hostileEffect = readEffect(targeted);
                }
            }
        }

        void readEffectAnimations(const json& value, AbilityDefinition& ability)
        {
            if (value.is_string())
            {
                ability.friendlyEffectAnimation = value.get<std::string>();
                ability.hostileEffectAnimation = value.get<std::string>();
                return;
            }
            for (const auto& targeted : value)
            {
                if (targeted.value("target", std::string{}) == "friend")
                {
                    ability.friendlyEffectAnimation = targeted.at("value").get<std::string>();
                }
                else
                {
                    ability.hostileEffectAnimation = targeted.at("value").get<std::string>();
                }
            }
        }

        TargetRelation readTargetRelation(const std::string& value)
        {
            if (value == "self") return TargetRelation::self;
            if (value == "own_team") return TargetRelation::ownTeam;
            if (value == "opposing_team") return TargetRelation::opposingTeam;
            if (value == "either_team") return TargetRelation::eitherTeam;
            if (value == "everyone_else") return TargetRelation::everyoneElse;
            throw std::runtime_error("Unknown ability target relation: " + value);
        }

        TargetScope readTargetScope(const std::string& value)
        {
            if (value == "single") return TargetScope::single;
            if (value == "single_random") return TargetScope::randomSingle;
            if (value == "count") return TargetScope::count;
            if (value == "team") return TargetScope::team;
            throw std::runtime_error("Unknown ability target scope: " + value);
        }

        Targeting readTargeting(const json& data)
        {
            const auto& source = data.at("targeting");
            Targeting result;
            result.relation = readTargetRelation(source.at("relation").get<std::string>());
            result.scope = readTargetScope(source.value("scope", std::string{"single"}));
            result.targetCount = source.value("count", 1);
            result.excludesActor = source.value("exclude_actor", false);
            if (result.targetCount < 1 || (result.scope == TargetScope::count && result.targetCount < 2))
            {
                throw std::runtime_error("Ability target count does not match its scope");
            }
            return result;
        }
    } // namespace

    void detail::loadBattleDefinitions(const json& document, BattleContent& content)
    {
        content.bossHealthAndDamageMultiplier = document.value("boss_health_and_damage_multiplier", 1.0);
        if (content.bossHealthAndDamageMultiplier <= 0.0)
        {
            throw std::runtime_error("Boss attribute multiplier must be positive");
        }
        for (const auto& value : document.at("combatants"))
        {
            CombatantDefinition combatant;
            combatant.id = gameplay::ContentDefinitionId{value.at("id").get<std::string>()};
            combatant.name = value.at("name").get<std::string>();
            combatant.team = value.at("team").get<std::string>() == "players" ? gameplay::Team::players : gameplay::Team::enemies;
            const auto& attributes = value.at("attributes");
            combatant.attributes = {attributes.value("maximum_health", 0),
                                    attributes.value("damage_minimum", 0),
                                    attributes.value("damage_maximum", 0),
                                    attributes.value("armour", 0),
                                    attributes.value("critical_hit_chance", 0),
                                    attributes.value("dodge", 0),
                                    attributes.value("initiative", 0),
                                    attributes.value("precision", 0)};
            const auto& resistances = value.at("resistances");
            combatant.resistances.sleepResistance = resistances.value("sleep", 0);
            combatant.resistances.confusionResistance = resistances.value("confusion", 0);
            combatant.resistances.debuffResistance = resistances.value("debuff", 0);
            combatant.resistances.decayResistance = resistances.value("decay", 0);
            combatant.marksAttackersForRevenge = value.value("marks_attackers_for_revenge", false);
            combatant.aiPolicy = value.value("ai_policy", std::string{});
            combatant.encounterCost = value.value("encounter_cost", 0);
            for (const auto& ability : value.at("abilities"))
            {
                combatant.abilities.emplace_back(ability.get<std::string>());
            }
            content.combatants.emplace(combatant.id, std::move(combatant));
        }

        for (const auto& value : document.at("abilities"))
        {
            const auto& data = value.at("data");
            AbilityDefinition ability;
            ability.id = gameplay::AbilityId{value.at("id").get<std::string>()};
            ability.name = data.at("name").get<std::string>();
            ability.animation = data.value("animation", std::string{"idle"});
            ability.targeting = readTargeting(data);
            ability.precisionModifier = integer(data, "precision");
            ability.fatigue = integer(data, "fatigue");
            if (ability.fatigue != 0)
            {
                ability.fatigueTarget = readFatigueTarget(data.at("fatigue_target").get<std::string>());
            }
            ability.targeting.requiresRevengeMarker = data.value("requires_revenge_marker", false);
            if (data.contains("summon"))
            {
                const auto& summon = data.at("summon");
                SummonDefinition definition;
                definition.combatant = gameplay::ContentDefinitionId{summon.at("combatant").get<std::string>()};
                definition.count = summon.at("count").get<int>();
                definition.allowedSlots = summon.at("allowed_slots").get<std::vector<int>>();
                ability.summon = std::move(definition);
            }
            if (data.contains("effect"))
            {
                readTargetedEffects(data.at("effect"), ability);
            }
            if (data.contains("effectAnimation"))
            {
                readEffectAnimations(data.at("effectAnimation"), ability);
            }
            content.abilities.emplace(ability.id, std::move(ability));
        }

        for (const auto& value : document.at("encounters"))
        {
            BattleContent::Encounter encounter;
            encounter.id = gameplay::ContentDefinitionId{value.at("id").get<std::string>()};
            encounter.background = value.at("background").get<std::string>();
            encounter.music = value.at("music").get<std::string>();
            for (const auto& spawn : value.at("combatants"))
            {
                encounter.combatants.emplace_back(gameplay::ContentDefinitionId{spawn.at("id").get<std::string>()}, spawn.at("slot").get<int>());
            }
            content.encounters.emplace(encounter.id, std::move(encounter));
        }
    }
} // namespace soma::content
