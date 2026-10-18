#include "content/EconomyContentReader.hpp"

#include <algorithm>
#include <stdexcept>

namespace soma::content::detail
{
    AttributeDelta readAttributeDelta(const Json& value)
    {
        AttributeDelta result;
        result.armour = integer(value, "armour");
        const auto damage = integer(value, "damage");
        result.damageMinimum = integer(value, "damageMin", damage);
        result.damageMaximum = integer(value, "damageMax", damage);
        result.initiative = integer(value, "initiative");
        result.maximumHealth = integer(value, "maxHealth");
        result.criticalHitChance = integer(value, "criticalHit");
        result.precision = integer(value, "precision");
        result.dodge = integer(value, "dodge");
        result.healing = integer(value, "healing");
        result.sleepResistance = integer(value, "sleepResistance");
        result.confusionResistance = integer(value, "confusionResistance");
        result.debuffResistance = integer(value, "debuffResistance");
        result.decayResistance = integer(value, "decayResistance");
        return result;
    }

    AbilityEffect readAbilityEffect(const Json& value)
    {
        static const std::set<std::string> effectProperties{"target",
                                                            "criticalHit",
                                                            "lessTargetsMoreDamage",
                                                            "damageFactor",
                                                            "damageAgainstMarked",
                                                            "damageAgainstRevengeMarked",
                                                            "lessDamageForEachSuccessiveTarget",
                                                            "transferHealthToNext",
                                                            "heal",
                                                            "healSelf",
                                                            "fatigue_target",
                                                            "mark",
                                                            "putToSleepProbability",
                                                            "riposte",
                                                            "riposteSelf",
                                                            "rage",
                                                            "hacked",
                                                            "transfusion",
                                                            "panic",
                                                            "panicAgainstMarked",
                                                            "spreadPanic",
                                                            "damageFromPanic",
                                                            "doublePanic",
                                                            "moveRandomly",
                                                            "ignoreArmour",
                                                            "ignoreDecayResistance",
                                                            "inflictAllDecayAndRemove",
                                                            "transferDecayOnKill",
                                                            "randomNegativeStatus",
                                                            "prolongNegativeEffects",
                                                            "increaseSleepProbabilityByFatigue",
                                                            "onFail",
                                                            "removeBuffs",
                                                            "removeDebuffs",
                                                            "stealBuffs",
                                                            "addMiss",
                                                            "addMissSelf",
                                                            "dynamite",
                                                            "bounty",
                                                            "confusion",
                                                            "decay",
                                                            "againstMarked",
                                                            "againstConfused",
                                                            "againstSleeping",
                                                            "removeDecayAndHealParty",
                                                            "buff"};
        requireKnownProperties(value, effectProperties, "skill effect");

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
            requireKnownProperties(transfer, {"shareOfMissingHealthAsDamage", "shareOfMissingHealthAsHeal"}, "health transfer effect");
            effect.transferredMissingHealthDamageFactor = number(transfer, "shareOfMissingHealthAsDamage");
            effect.transferredMissingHealthHealingFactor = number(transfer, "shareOfMissingHealthAsHeal");
        }
        effect.heal = integer(value, "heal");
        effect.healSelf = integer(value, "healSelf");
        effect.targetFatigue = integer(value, "fatigue_target");
        effect.markRounds = integer(value, "mark");
        effect.sleepProbability = number(value, "putToSleepProbability");
        effect.riposteRounds = integer(value, "riposte");
        effect.riposteSelfRounds = integer(value, "riposteSelf");
        effect.rageRounds = integer(value, "rage");
        effect.hackedRounds = integer(value, "hacked");
        effect.transfusionHealing = integer(value, "transfusion");
        effect.panicStacks = integer(value, "panic");
        effect.panicAgainstMarked = integer(value, "panicAgainstMarked");
        effect.spreadPanicTargets = integer(value, "spreadPanic");
        effect.damageFromPanic = value.value("damageFromPanic", false);
        effect.doublePanic = value.value("doublePanic", false);
        effect.moveTargetsRandomly = value.value("moveRandomly", false);
        effect.ignoreArmour = value.value("ignoreArmour", false);
        effect.ignoreDecayResistance = value.value("ignoreDecayResistance", false);
        effect.inflictAllDecayAndRemove = value.value("inflictAllDecayAndRemove", false);
        effect.transferDecayOnKill = value.value("transferDecayOnKill", false);
        effect.prolongNegativeEffects = integer(value, "prolongNegativeEffects");
        effect.increaseSleepProbabilityByFatigue = value.value("increaseSleepProbabilityByFatigue", false);
        if (value.contains("randomNegativeStatus"))
        {
            const auto& randomStatus = value.at("randomNegativeStatus");
            requireKnownProperties(randomStatus, {"probability", "duration"}, "random negative status effect");
            effect.randomNegativeStatusProbability = number(randomStatus, "probability");
            effect.randomNegativeStatusDuration = integer(randomStatus, "duration", 2);
        }
        if (value.contains("onFail"))
        {
            const auto& onFail = value.at("onFail");
            requireKnownProperties(onFail, {"decay"}, "failed status effect");
            if (onFail.contains("decay"))
            {
                requireKnownProperties(onFail.at("decay"), {"damage", "rounds"}, "failed sleep decay effect");
                effect.sleepFailureDecayDamage = integer(onFail.at("decay"), "damage");
                effect.sleepFailureDecayRounds = integer(onFail.at("decay"), "rounds");
            }
        }
        effect.removeBuffs = value.value("removeBuffs", false);
        effect.removeDebuffs = value.value("removeDebuffs", false);
        effect.stealBuffs = value.value("stealBuffs", false);
        effect.addMiss = integer(value, "addMiss");
        effect.addMissSelf = integer(value, "addMissSelf");
        effect.dynamite = value.value("dynamite", false);
        effect.bountyRounds = integer(value, "bounty");
        if (value.contains("confusion"))
        {
            requireKnownProperties(value.at("confusion"), {"probability", "rounds"}, "confusion effect");
            effect.confusionProbability = number(value.at("confusion"), "probability");
            effect.confusionRounds = integer(value.at("confusion"), "rounds");
        }
        if (value.contains("decay"))
        {
            requireKnownProperties(value.at("decay"), {"damage", "rounds"}, "decay effect");
            effect.decayDamage = integer(value.at("decay"), "damage");
            effect.decayRounds = integer(value.at("decay"), "rounds");
        }
        const auto readConditionalDecay = [&value](const char* property, const char* context, int& damage, int& rounds) {
            if (!value.contains(property))
            {
                return;
            }
            requireKnownProperties(value.at(property), {"decay"}, context);
            if (value.at(property).contains("decay"))
            {
                requireKnownProperties(value.at(property).at("decay"), {"damage", "rounds"}, context);
                damage = integer(value.at(property).at("decay"), "damage");
                rounds = integer(value.at(property).at("decay"), "rounds");
            }
        };
        int unusedDamage{};
        int unusedRounds{};
        readConditionalDecay("againstMarked", "marked target effect", effect.decayDamageAgainstMarked, unusedRounds);
        readConditionalDecay("againstConfused", "confused target effect", unusedDamage, effect.decayRoundsAgainstConfused);
        readConditionalDecay("againstSleeping", "sleeping target effect", unusedDamage, effect.decayRoundsAgainstSleeping);
        effect.removeDecayAndHealParty = value.value("removeDecayAndHealParty", false);
        if (value.contains("buff"))
        {
            const auto& buff = value.at("buff");
            static const std::set<std::string> buffProperties{"duration",
                                                              "isPositive",
                                                              "onSelf",
                                                              "armour",
                                                              "damage",
                                                              "damageMin",
                                                              "damageMax",
                                                              "initiative",
                                                              "maxHealth",
                                                              "criticalHit",
                                                              "precision",
                                                              "dodge",
                                                              "healing",
                                                              "sleepResistance",
                                                              "confusionResistance",
                                                              "debuffResistance",
                                                              "decayResistance",
                                                              "healingReceived",
                                                              "damageAgainstMarked",
                                                              "damagePerNegativeStatus",
                                                              "damageFromFatigue",
                                                              "blocksFatigueStageOne",
                                                              "blocksFatigueStageTwo"};
            requireKnownProperties(buff, buffProperties, "skill buff");
            auto& definition = buff.value("onSelf", false) ? effect.actorBuff : effect.targetBuff;
            definition.duration = integer(buff, "duration");
            definition.positive = buff.value("isPositive", true);
            definition.attributes = readAttributeDelta(buff);
            definition.healingReceived = number(buff, "healingReceived");
            definition.damageAgainstMarked = number(buff, "damageAgainstMarked");
            definition.damagePerNegativeStatus = number(buff, "damagePerNegativeStatus");
            definition.damageFromFatigue = buff.value("damageFromFatigue", false);
            definition.blocksFatigueStageOne = buff.value("blocksFatigueStageOne", false);
            definition.blocksFatigueStageTwo = buff.value("blocksFatigueStageTwo", false);
        }
        return effect;
    }

    SkillEffectTarget readSkillEffectTarget(const std::string& value)
    {
        if (value == "friend")
        {
            return SkillEffectTarget::friendly;
        }
        if (value == "enemy")
        {
            return SkillEffectTarget::hostile;
        }
        if (value == "both")
        {
            return SkillEffectTarget::both;
        }
        if (value == "all")
        {
            return SkillEffectTarget::allCombatants;
        }
        throw std::runtime_error("Unknown skill effect target: " + value);
    }

    AbilityUpgrade readUpgrade(const Json& value)
    {
        requireKnownProperties(value, {"additional_targets", "precision", "fatigue", "decay", "effects"}, "skill modifier");
        AbilityUpgrade upgrade;
        upgrade.additionalTargets = integer(value, "additional_targets");
        upgrade.precisionModifier = integer(value, "precision");
        upgrade.fatigue = integer(value, "fatigue");
        if (value.contains("decay"))
        {
            requireKnownProperties(value.at("decay"), {"damage", "rounds"}, "decay modifier");
            AbilityEffect effect;
            effect.decayDamage = integer(value.at("decay"), "damage");
            effect.decayRounds = integer(value.at("decay"), "rounds");
            upgrade.effects.push_back({SkillEffectTarget::both, effect});
        }
        for (const auto& effect : value.value("effects", Json::array()))
        {
            upgrade.effects.push_back({readSkillEffectTarget(effect.value("target", "both")), readAbilityEffect(effect)});
        }
        return upgrade;
    }

    void loadSkills(const Json& document, EconomyContent& content, const std::filesystem::path& contentRoot)
    {
        const auto repositoryRoot = contentRoot.parent_path().parent_path().parent_path();
        for (const auto& value : document.at("skills"))
        {
            const gameplay::SkillId id{value.at("id").get<std::string>()};
            const gameplay::AssetId icon{id.value() + ".icon"};
            const auto iconPath = std::filesystem::path{value.at("icon").get<std::string>()};
            SkillDefinition definition{id,
                                       gameplay::ContentDefinitionId{value.at("player").get<std::string>()},
                                       gameplay::AbilityId{value.at("ability").get<std::string>()},
                                       integer(value, "slot"),
                                       value.at("name").get<std::string>(),
                                       value.at("description").get<std::string>(),
                                       integer(value, "cost"),
                                       {},
                                       icon,
                                       readUpgrade(value.at("modifier"))};
            for (const auto& prerequisite : value.at("prerequisites"))
            {
                definition.prerequisites.emplace_back(prerequisite.get<std::string>());
            }
            if (definition.slot < 1 || definition.slot > 8 || definition.name.empty() || definition.cost < 0)
            {
                throw std::runtime_error("Invalid skill values: " + id.value());
            }
            if (!std::filesystem::exists(repositoryRoot / iconPath))
            {
                throw std::runtime_error("Missing skill icon: " + iconPath.string());
            }
            if (!content.skills.emplace(id, std::move(definition)).second)
            {
                throw std::runtime_error("Duplicate skill ID: " + id.value());
            }
            content.assets.emplace(icon, iconPath);
        }

        for (const auto& value : document.at("ability_sets"))
        {
            AbilitySetDefinition set;
            set.player = gameplay::ContentDefinitionId{value.at("player").get<std::string>()};
            set.slot = value.at("slot").get<std::size_t>();
            if (set.slot >= 4 || !value.at("versions").is_array() || value.at("versions").size() != set.versions.size())
            {
                throw std::runtime_error("An ability set must contain two versions and use a slot from 0 through 3");
            }
            for (std::size_t versionIndex = 0; versionIndex < set.versions.size(); ++versionIndex)
            {
                const auto& source = value.at("versions").at(versionIndex);
                auto& version = set.versions.at(versionIndex);
                version.ability = gameplay::AbilityId{source.at("ability").get<std::string>()};
                if (!source.at("skills").is_array() || source.at("skills").size() != version.skills.size())
                {
                    throw std::runtime_error("Every ability version must reference exactly five skills");
                }
                for (std::size_t skillIndex = 0; skillIndex < version.skills.size(); ++skillIndex)
                {
                    version.skills.at(skillIndex) = gameplay::SkillId{source.at("skills").at(skillIndex).get<std::string>()};
                    const auto skill = content.skills.find(version.skills.at(skillIndex));
                    if (skill == content.skills.end())
                    {
                        throw std::runtime_error("Ability version references an unknown skill: " + version.skills.at(skillIndex).value());
                    }
                    if (skill->second.player != set.player || skill->second.ability != version.ability ||
                        skill->second.slot != static_cast<int>(skillIndex + 1))
                    {
                        throw std::runtime_error("Ability version contains a skill assigned to another player, ability, or slot: " +
                                                 version.skills.at(skillIndex).value());
                    }
                }
            }
            const auto duplicate = std::ranges::find_if(content.abilitySets, [&set](const auto& existing) {
                return existing.player == set.player && existing.slot == set.slot;
            });
            if (duplicate != content.abilitySets.end())
            {
                throw std::runtime_error("Duplicate player ability set");
            }
            content.abilitySets.push_back(std::move(set));
        }
        for (const auto& [id, skill] : content.skills)
        {
            for (const auto& prerequisite : skill.prerequisites)
            {
                const auto found = content.skills.find(prerequisite);
                if (found == content.skills.end() || found->second.ability != skill.ability || found->second.slot >= skill.slot)
                {
                    throw std::runtime_error("Invalid prerequisite " + prerequisite.value() + " on skill " + id.value());
                }
            }
        }
        for (const auto& entry : content.skills)
        {
            const auto& id = entry.first;
            auto referenced = 0;
            for (const auto& set : content.abilitySets)
            {
                for (const auto& version : set.versions)
                {
                    referenced += static_cast<int>(std::ranges::count(version.skills, id));
                }
            }
            if (referenced != 1)
            {
                throw std::runtime_error("A skill must belong to exactly one ability version: " + id.value());
            }
        }
    }

} // namespace soma::content::detail
