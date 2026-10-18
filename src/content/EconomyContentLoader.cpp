#include "content/EconomyContentLoader.hpp"

#include "gameplay/items/ItemRarity.hpp"

#include <algorithm>
#include <fstream>
#include <json.hpp>
#include <set>
#include <stdexcept>

namespace soma::content
{
    namespace
    {
        using json = nlohmann::json;

        json readJson(const std::filesystem::path& path)
        {
            std::ifstream stream(path);
            if (!stream)
            {
                throw std::runtime_error("Could not open " + path.string());
            }
            return json::parse(stream);
        }

        int integer(const json& value, const char* name, int fallback = 0)
        {
            return value.contains(name) ? value.at(name).get<int>() : fallback;
        }

        double number(const json& value, const char* name, double fallback = 0.0)
        {
            return value.contains(name) ? value.at(name).get<double>() : fallback;
        }

        void requireKnownProperties(const json& value, const std::set<std::string>& allowed, const std::string& context)
        {
            for (const auto& [name, property] : value.items())
            {
                static_cast<void>(property);
                if (!allowed.contains(name))
                {
                    throw std::runtime_error("Unknown " + context + " property: " + name);
                }
            }
        }

        Point readPoint(const json& value)
        {
            if (!value.is_array() || value.size() != 2)
            {
                throw std::runtime_error("Expected a two-component point");
            }
            return {value.at(0).get<int>(), value.at(1).get<int>()};
        }

        Rectangle readRectangle(const json& value)
        {
            if (!value.is_array() || value.size() != 4)
            {
                throw std::runtime_error("Expected a four-component rectangle");
            }
            return {value.at(0).get<int>(), value.at(1).get<int>(), value.at(2).get<int>(), value.at(3).get<int>()};
        }

        template <std::size_t Size, typename Reader>
        auto readArray(const json& value, Reader reader)
        {
            using Element = decltype(reader(value.at(0)));
            if (!value.is_array() || value.size() != Size)
            {
                throw std::runtime_error("Presentation array has an unexpected size");
            }
            std::array<Element, Size> result;
            std::ranges::transform(value, result.begin(), reader);
            return result;
        }

        AttributeDelta readAttributeDelta(const json& value)
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

        AbilityEffect readAbilityEffect(const json& value)
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
                requireKnownProperties(
                    transfer, {"shareOfMissingHealthAsDamage", "shareOfMissingHealthAsHeal"}, "health transfer effect");
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

        AbilityUpgrade readUpgrade(const json& value)
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
            for (const auto& effect : value.value("effects", json::array()))
            {
                upgrade.effects.push_back({readSkillEffectTarget(effect.value("target", "both")), readAbilityEffect(effect)});
            }
            return upgrade;
        }

        gameplay::AttributeBonuses readBonuses(const json& value)
        {
            gameplay::AttributeBonuses result;
            result.armour = integer(value, "armour");
            result.maximumHealth = integer(value, "maxHealth");
            result.damageMinimum = integer(value, "damageMin");
            result.damageMaximum = integer(value, "damageMax");
            result.initiative = integer(value, "initiative");
            result.criticalHitChance = integer(value, "criticalHit");
            result.dodge = integer(value, "dodge");
            result.precision = integer(value, "precision");
            result.healing = integer(value, "healing");
            result.sleepResistance = integer(value, "sleepResistance");
            result.confusionResistance = integer(value, "confusionResistance");
            result.debuffResistance = integer(value, "debuffResistance");
            result.decayResistance = integer(value, "decayResistance");
            return result;
        }

        gameplay::ItemColor readItemColor(const json& value)
        {
            if (!value.is_array() || value.size() != 3)
            {
                throw std::runtime_error("Expected a three-component item color");
            }
            for (const auto& component : value)
            {
                if (!component.is_number_integer() || component.get<int>() < 0 || component.get<int>() > 255)
                {
                    throw std::runtime_error("Item color components must be between 0 and 255");
                }
            }
            return {static_cast<std::uint8_t>(value.at(0).get<int>()),
                    static_cast<std::uint8_t>(value.at(1).get<int>()),
                    static_cast<std::uint8_t>(value.at(2).get<int>())};
        }

        gameplay::ItemCategory readCategory(const std::string& value)
        {
            if (value == "currency")
            {
                return gameplay::ItemCategory::currency;
            }
            if (value == "equipment")
            {
                return gameplay::ItemCategory::equipment;
            }
            if (value == "consumable")
            {
                return gameplay::ItemCategory::consumable;
            }
            throw std::runtime_error("Unknown item category: " + value);
        }

        std::optional<gameplay::EquipmentBattleEffect> readEquipmentBattleEffect(const json& value)
        {
            if (!value.contains("battle_effect"))
            {
                return std::nullopt;
            }
            const auto& effect = value.at("battle_effect");
            const auto type = effect.at("type").get<std::string>();
            if (type == "lone_victory")
            {
                const auto threshold = effect.at("other_party_health_threshold").get<int>();
                const auto damage = effect.at("damage").get<int>();
                if (threshold < 0 || damage <= 0)
                {
                    throw std::runtime_error("Invalid lone-victory equipment effect");
                }
                return gameplay::LoneVictoryEffect{threshold, damage};
            }
            throw std::runtime_error("Unknown equipment battle effect: " + type);
        }

        gameplay::ItemProperties readProperties(const json& value, gameplay::ItemCategory category)
        {
            if (category == gameplay::ItemCategory::currency)
            {
                const auto currency = value.at("currency").get<std::string>();
                if (currency == "cards")
                {
                    return gameplay::CurrencyItem{gameplay::Currency::cards};
                }
                if (currency == "dice")
                {
                    return gameplay::CurrencyItem{gameplay::Currency::dice};
                }
                throw std::runtime_error("Unknown currency: " + currency);
            }
            if (category == gameplay::ItemCategory::equipment)
            {
                return gameplay::EquipmentItem{readBonuses(value.at("bonuses")),
                                               value.value("miss_on_high_damage", false),
                                               value.value("heal_on_pass", false),
                                               readEquipmentBattleEffect(value)};
            }
            return gameplay::ConsumableItem{integer(value, "healing"), readBonuses(value.at("bonuses")), integer(value, "duration")};
        }

        void loadItems(const json& document, EconomyContent& content, const std::filesystem::path& contentRoot)
        {
            const auto repositoryRoot = contentRoot.parent_path().parent_path().parent_path();
            for (const auto& value : document.at("diamond_bonuses"))
            {
                std::optional<gameplay::ItemColor> requiredConnectionColor;
                if (value.contains("requires_connection_color"))
                {
                    requiredConnectionColor = readItemColor(value.at("requires_connection_color"));
                }
                content.diamondBonuses.push_back(
                    {readItemColor(value.at("color")), requiredConnectionColor, readBonuses(value.at("bonuses"))});
            }
            for (const auto& value : document.at("items"))
            {
                const gameplay::ItemId id{value.at("id").get<std::string>()};
                const gameplay::AssetId icon{id.value() + ".icon"};
                const auto iconPath = std::filesystem::path{value.at("icon").get<std::string>()};
                const auto category = readCategory(value.at("category").get<std::string>());
                gameplay::ItemDefinition definition{id,
                                                    value.at("name").get<std::string>(),
                                                    category,
                                                    integer(value, "level", 1),
                                                    integer(value, "price"),
                                                    readProperties(value, category),
                                                    icon,
                                                    value.at("campaigns").get<std::vector<std::string>>(),
                                                    value.value("description", std::string{})};
                if (definition.name.empty() || definition.level < gameplay::minimumItemRarityLevel ||
                    definition.level > gameplay::maximumItemRarityLevel || definition.price < 0 || definition.campaigns.empty())
                {
                    throw std::runtime_error("Invalid item values: " + id.value());
                }
                for (const auto& campaign : definition.campaigns)
                {
                    if (campaign != "bang" && campaign != "kutschfahrt")
                    {
                        throw std::runtime_error("Unknown campaign '" + campaign + "' on item " + id.value());
                    }
                }
                if (!std::filesystem::exists(repositoryRoot / iconPath))
                {
                    throw std::runtime_error("Missing item icon: " + iconPath.string());
                }
                if (!content.items.emplace(id, std::move(definition)).second)
                {
                    throw std::runtime_error("Duplicate item ID: " + id.value());
                }
                content.assets.emplace(icon, iconPath);
            }
        }

        void loadSkills(const json& document, EconomyContent& content, const std::filesystem::path& contentRoot)
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
                            throw std::runtime_error("Ability version references an unknown skill: " +
                                                     version.skills.at(skillIndex).value());
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
                    for (const auto& version : set.versions)
                        referenced += static_cast<int>(std::ranges::count(version.skills, id));
                if (referenced != 1)
                {
                    throw std::runtime_error("A skill must belong to exactly one ability version: " + id.value());
                }
            }
        }

        void loadPresentation(const json& document, EconomyContent& content, const std::filesystem::path& contentRoot)
        {
            const auto repositoryRoot = contentRoot.parent_path().parent_path().parent_path();
            for (const auto& [idValue, pathValue] : document.at("assets").items())
            {
                const gameplay::AssetId id{idValue};
                const std::filesystem::path path{pathValue.get<std::string>()};
                if (!std::filesystem::exists(repositoryRoot / path))
                {
                    throw std::runtime_error("Missing economy presentation asset: " + path.string());
                }
                if (!content.assets.emplace(id, path).second)
                {
                    throw std::runtime_error("Duplicate economy asset ID: " + id.value());
                }
            }

            auto& presentation = content.presentation;
            presentation.referenceSize = readPoint(document.at("reference_size"));
            const auto& resources = document.at("resource_counter");
            presentation.resourceCounter = {readPoint(resources.at("dice_icon")),
                                            readPoint(resources.at("dice_text_center")),
                                            readPoint(resources.at("cards_icon")),
                                            readPoint(resources.at("cards_text_center")),
                                            resources.at("dice_scale").get<double>(),
                                            resources.at("cards_scale").get<double>(),
                                            integer(resources, "text_size")};
            const auto& inventory = document.at("inventory");
            presentation.inventory = {readPoint(inventory.at("origin")),
                                      readPoint(inventory.at("items_panel")),
                                      readPoint(inventory.at("equipment_panel")),
                                      readPoint(inventory.at("attributes")),
                                      readPoint(inventory.at("previous_button")),
                                      readPoint(inventory.at("next_button")),
                                      readPoint(inventory.at("close_button")),
                                      readArray<4>(inventory.at("equipment_fields"), readPoint),
                                      readPoint(inventory.at("equipment_item_inset")),
                                      readPoint(inventory.at("equipment_background")),
                                      readPoint(inventory.at("equipment_diamond")),
                                      readArray<4>(inventory.at("equipment_connections"), readPoint),
                                      readRectangle(inventory.at("items_scrollbar"))};

            const auto& shop = document.at("shop");
            presentation.shop = {readPoint(shop.at("vending_origin")),
                                 readPoint(shop.at("inventory_panel")),
                                 readPoint(shop.at("consumables_row")),
                                 readPoint(shop.at("shop_panel")),
                                 readPoint(shop.at("shop_item_origin")),
                                 readPoint(shop.at("shop_item_spacing")),
                                 readPoint(shop.at("consumable_origin")),
                                 readPoint(shop.at("consumable_spacing")),
                                 readPoint(shop.at("dice")),
                                 readPoint(shop.at("dice_price_lock")),
                                 readPoint(shop.at("buy_button")),
                                 readPoint(shop.at("close_button")),
                                 readPoint(shop.at("title")),
                                 readPoint(shop.at("offer_title")),
                                 readPoint(shop.at("selection_inset")),
                                 readPoint(shop.at("price_sign_offset")),
                                 readPoint(shop.at("slot_lock_offset")),
                                 readRectangle(shop.at("slot_unlock_price")),
                                 readPoint(shop.at("reduced_price_sign_offset")),
                                 readRectangle(shop.at("dice_unlock_price")),
                                 readPoint(shop.at("action_price")),
                                 readPoint(shop.at("action_cards")),
                                 readPoint(shop.at("confirmation_message")),
                                 integer(shop, "confirmation_message_width"),
                                 readPoint(shop.at("confirmation_yes_button")),
                                 readPoint(shop.at("confirmation_no_button")),
                                 readPoint(shop.at("sell_slider")),
                                 readPoint(shop.at("sell_confirm_button")),
                                 readPoint(shop.at("sell_cancel_button")),
                                 readRectangle(shop.at("sell_quantity_bounds")),
                                 readPoint(shop.at("owned_item_inset")),
                                 readPoint(shop.at("owned_item_spacing")),
                                 readPoint(shop.at("item_size")),
                                 readPoint(shop.at("quantity_offset"))};

            const auto& skills = document.at("skills");
            presentation.skills = {readPoint(skills.at("origin")),
                                   readRectangle(skills.at("player_name_bounds")),
                                   readRectangle(skills.at("panel_title_bounds")),
                                   readArray<4>(skills.at("ability_hitboxes"), readRectangle),
                                   readRectangle(skills.at("ability_name_bounds")),
                                   readRectangle(skills.at("ability_icon_bounds")),
                                   readArray<4>(skills.at("ability_bridges"), readPoint),
                                   readArray<2>(skills.at("tree_origins"), readPoint),
                                   readPoint(skills.at("version_ability_icon")),
                                   readArray<2>(skills.at("version_selectors"), readPoint),
                                   readArray<5>(skills.at("skill_icons"), readPoint),
                                   readArray<4>(skills.at("connection_offsets"), readPoint),
                                   readArray<4>(skills.at("connection_types"), [](const json& value) { return value.get<int>(); }),
                                   readPoint(skills.at("acquired_inset")),
                                   readPoint(skills.at("selection_inset")),
                                   readPoint(skills.at("dice")),
                                   readPoint(skills.at("buy_button")),
                                   readPoint(skills.at("previous_button")),
                                   readPoint(skills.at("next_button")),
                                   readPoint(skills.at("close_button")),
                                   readRectangle(skills.at("selected_skill_title_bounds")),
                                   readPoint(skills.at("dice_cost_offset"))};
            const auto& drawer = document.at("consumable_drawer");
            presentation.consumableDrawer = {readPoint(drawer.at("open_position")),
                                              integer(drawer, "closed_x"),
                                              readPoint(drawer.at("toggle_size")),
                                              readPoint(drawer.at("toggle_inset")),
                                              readPoint(drawer.at("row_inset")),
                                              readPoint(drawer.at("first_item_inset")),
                                              integer(drawer, "item_spacing")};
            const auto& explorationConsumables = document.at("exploration_consumables");
            presentation.explorationConsumables = {readPoint(explorationConsumables.at("position")),
                                                    readPoint(explorationConsumables.at("row_inset")),
                                                    readPoint(explorationConsumables.at("first_item_inset")),
                                                    readPoint(explorationConsumables.at("quantity_offset")),
                                                    integer(explorationConsumables, "item_spacing"),
                                                    integer(explorationConsumables, "item_size")};
            if (presentation.referenceSize.x <= 0 || presentation.referenceSize.y <= 0)
            {
                throw std::runtime_error("Economy presentation reference size must be positive");
            }
        }
    } // namespace

    bool EconomyContentLoadResult::valid() const
    {
        return diagnostics.empty();
    }

    EconomyContentLoadResult EconomyContentLoader::load(const std::filesystem::path& contentRoot) const
    {
        EconomyContentLoadResult result;
        try
        {
            const auto itemDocument = readJson(contentRoot / "economy" / "items.json");
            if (itemDocument.value("schema_version", 0) != 1 || itemDocument.value("content_kind", std::string{}) != "economy_items")
            {
                throw std::runtime_error("Expected economy item schema version 1");
            }
            loadItems(itemDocument, result.content, contentRoot);
            const auto skillDocument = readJson(contentRoot / "economy" / "skills.json");
            if (skillDocument.value("schema_version", 0) != 1 || skillDocument.value("content_kind", std::string{}) != "economy_skills")
            {
                throw std::runtime_error("Expected economy skill schema version 1");
            }
            loadSkills(skillDocument, result.content, contentRoot);
            const auto presentationDocument = readJson(contentRoot / "economy" / "presentation.json");
            if (presentationDocument.value("schema_version", 0) != 1 ||
                presentationDocument.value("content_kind", std::string{}) != "economy_presentation")
            {
                throw std::runtime_error("Expected economy presentation schema version 1");
            }
            loadPresentation(presentationDocument, result.content, contentRoot);
        }
        catch (const std::exception& exception)
        {
            result.diagnostics.push_back({gameplay::DiagnosticSeverity::error, "economy-content", contentRoot, {}, exception.what()});
        }
        return result;
    }
} // namespace soma::content
