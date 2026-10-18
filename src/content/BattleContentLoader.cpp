#include "content/BattleContentLoader.hpp"

#include <algorithm>
#include <fstream>
#include <functional>
#include <json.hpp>
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

        AttributeDelta readAttributes(const json& value)
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
                definition.attributes = readAttributes(buff);
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

        void loadDefinitions(const json& document, BattleContent& content)
        {
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

        void loadPresentation(const json& document, BattleContent& content)
        {
            for (const auto& [id, path] : document.at("assets").items())
            {
                content.assets.emplace(id, path.get<std::string>());
            }
            for (const auto& [id, path] : document.at("sounds").items())
            {
                content.sounds.emplace(id, path.get<std::string>());
            }
            content.layout.referenceWidth = document.at("reference_resolution").at("width");
            content.layout.referenceHeight = document.at("reference_resolution").at("height");
            const auto& layout = document.at("layout");
            content.layout.normalScale = layout.at("normal_scale");
            content.layout.abilityScale = layout.at("ability_scale");
            content.layout.groundY = layout.at("ground_y");
            content.layout.playerStartX = layout.at("player_start_x");
            content.layout.playerSpacing = layout.at("player_spacing");
            content.layout.enemyStartFromViewCenter = layout.at("enemy_start_x_from_view_center");
            content.layout.enemySpacing = layout.at("enemy_spacing");
            content.layout.abilityStageY = layout.at("ability_stage_y");
            content.layout.abilityAttackerXPlayer = layout.at("ability_attacker_x_player");
            content.layout.abilityTargetXPlayer = layout.at("ability_target_x_player");
            content.layout.abilityGroupPadding = layout.at("ability_group_padding");
            const auto& abilityPanel = layout.at("ability_panel");
            content.layout.abilityPanelX = abilityPanel.at("x");
            content.layout.abilityPanelY = abilityPanel.at("y");
            const auto& abilityButtons = layout.at("ability_buttons");
            content.layout.abilityButtonStartX = abilityButtons.at("start_x");
            content.layout.abilityButtonY = abilityButtons.at("y");
            content.layout.abilityButtonSpacing = abilityButtons.at("spacing");
            const auto& informationPanel = layout.at("combatant_information_panel");
            content.layout.informationPanelX = informationPanel.at("x");
            content.layout.informationPanelY = informationPanel.at("y");
            const auto& announcement = layout.at("enemy_announcement");
            content.layout.announcementX = announcement.at("x");
            content.layout.announcementY = announcement.at("y");
            content.layout.announcementTextX = announcement.at("text_x");
            content.layout.announcementTextY = announcement.at("text_y");
            const auto& battleResult = layout.at("battle_result");
            content.layout.battleResultX = battleResult.at("x");
            content.layout.battleResultY = battleResult.at("y");
            const auto& turnOrderNumbers = layout.at("turn_order_numbers");
            content.layout.turnOrderNumberOffsetY = turnOrderNumbers.at("offset_y");
            content.layout.turnOrderNumberCharacterSize = turnOrderNumbers.at("character_size");
            const auto& expectedAttack = layout.at("expected_attack");
            content.layout.expectedAttackCenterX = expectedAttack.at("center_x");
            content.layout.expectedAttackSingleY = expectedAttack.at("single_line_y");
            content.layout.expectedAttackMultipleY = expectedAttack.at("multiple_lines_y");
            content.layout.expectedAttackLineSpacing = expectedAttack.at("line_spacing");
            content.layout.expectedAttackTextSize = expectedAttack.at("character_size");
            const auto& timing = layout.at("timing");
            content.layout.announcementDuration = timing.at("announcement_seconds");
            content.layout.executionTimeout = timing.at("execution_timeout_seconds");
            content.layout.reactionDuration = timing.at("reaction_seconds");
            content.layout.resultDuration = timing.at("result_seconds");
            content.layout.wakeResultDuration = timing.at("wake_result_seconds");
            content.layout.defeatDuration = timing.at("defeat_seconds");
            const auto& itemActivation = layout.at("item_activation");
            content.layout.itemSpeechDuration = itemActivation.at("speech_duration_seconds");
            content.layout.itemSpeechMaximumWidth = itemActivation.at("maximum_text_width");
            content.layout.itemSpeechCharacterSize = itemActivation.at("character_size");
            content.layout.itemSpeechPaddingX = itemActivation.at("padding").at(0);
            content.layout.itemSpeechPaddingY = itemActivation.at("padding").at(1);
            content.layout.itemSpeechSpeakerOffsetX = itemActivation.at("speaker_offset").at(0);
            content.layout.itemSpeechSpeakerOffsetY = itemActivation.at("speaker_offset").at(1);
            content.layout.itemSpeechTextVerticalOffset = itemActivation.at("text_vertical_offset");

            for (const auto& value : document.at("item_activations"))
            {
                const gameplay::ItemId item{value.at("item").get<std::string>()};
                const ItemActivationPresentation presentation{value.at("speech").get<std::string>(),
                                                                value.at("notification").get<std::string>()};
                if (presentation.speech.empty() || presentation.notification.empty() ||
                    !content.itemActivations.emplace(item, presentation).second)
                {
                    throw std::runtime_error("Invalid or duplicate item activation presentation: " + item.value());
                }
            }

            for (const auto& value : document.at("combatants"))
            {
                CombatantPresentationDefinition definition;
                definition.id = gameplay::ContentDefinitionId{value.at("id").get<std::string>()};
                definition.preferredSlot = value.at("slot");
                definition.model = value.at("model").get<std::string>();
                definition.entity = value.at("entity").get<std::string>();
                definition.scaleAdjustment = value.at("scale_adjustment");
                if (value.contains("ability_icons"))
                {
                    for (const auto& [ability, icon] : value.at("ability_icons").items())
                    {
                        definition.abilityIcons.emplace(gameplay::AbilityId{ability}, icon.get<std::string>());
                    }
                }
                content.presentation.emplace(definition.id, std::move(definition));
            }
        }

        void validateReferences(const BattleContent& content, const std::filesystem::path& contentRoot)
        {
            if (content.layout.turnOrderNumberCharacterSize <= 0 || content.layout.announcementDuration <= 0.0 ||
                content.layout.executionTimeout <= 0.0 || content.layout.reactionDuration <= 0.0 || content.layout.resultDuration <= 0.0 ||
                content.layout.wakeResultDuration <= 0.0 || content.layout.defeatDuration <= 0.0)
            {
                throw std::runtime_error("Invalid battle timing or turn-order presentation layout");
            }
            if (content.layout.itemSpeechDuration <= 0.0 || content.layout.itemSpeechMaximumWidth <= 0 ||
                content.layout.itemSpeechCharacterSize <= 0 || content.layout.itemSpeechPaddingX < 0 || content.layout.itemSpeechPaddingY < 0)
            {
                throw std::runtime_error("Invalid item activation presentation layout");
            }
            for (const auto& [id, combatant] : content.combatants)
            {
                if (combatant.team == gameplay::Team::enemies && combatant.encounterCost <= 0)
                {
                    throw std::runtime_error("Enemy has no positive encounter cost: " + id.value());
                }
                if (!content.presentation.contains(id))
                {
                    throw std::runtime_error("Combatant has no presentation definition: " + id.value());
                }
                for (const auto& ability : combatant.abilities)
                {
                    if (!content.abilities.contains(ability))
                    {
                        throw std::runtime_error("Combatant references unknown ability: " + ability.value());
                    }
                }
            }
            for (const auto& [id, ability] : content.abilities)
            {
                if (ability.summon.has_value() && !content.combatants.contains(ability.summon->combatant))
                {
                    throw std::runtime_error("Ability references unknown summon: " + id.value());
                }
            }
            for (const auto& [id, encounter] : content.encounters)
            {
                std::vector<int> occupiedSlots;
                for (const auto& [combatant, slot] : encounter.combatants)
                {
                    if (!content.combatants.contains(combatant))
                    {
                        throw std::runtime_error("Encounter references unknown combatant: " + id.value());
                    }
                    if (slot < 0 || slot >= 8 || std::ranges::find(occupiedSlots, slot) != occupiedSlots.end())
                    {
                        throw std::runtime_error("Encounter contains an invalid or duplicate slot: " + id.value());
                    }
                    occupiedSlots.push_back(slot);
                }
            }

            const auto repositoryRoot = contentRoot.parent_path().parent_path().parent_path();
            const auto validateAsset = [&repositoryRoot](const std::string& id, const std::filesystem::path& path) {
                if (!std::filesystem::exists(repositoryRoot / path))
                {
                    throw std::runtime_error("Missing asset '" + id + "': " + path.string());
                }
            };
            for (const auto& [id, path] : content.assets)
            {
                validateAsset(id, path);
            }
            for (const auto& [id, path] : content.sounds)
            {
                validateAsset(id, path);
            }
            for (const auto& [id, presentation] : content.presentation)
            {
                validateAsset(id.value() + ".model", presentation.model);
                for (const auto& [ability, icon] : presentation.abilityIcons)
                {
                    if (!content.abilities.contains(ability))
                        throw std::runtime_error("Presentation references an unknown ability icon: " + ability.value());
                    validateAsset(id.value() + ".ability_icon." + ability.value(), icon);
                }
            }
            for (const auto& [id, encounter] : content.encounters)
            {
                validateAsset(id.value() + ".background", encounter.background);
                validateAsset(id.value() + ".music", encounter.music);
            }
        }
    } // namespace

    bool BattleContentLoadResult::valid() const
    {
        return diagnostics.empty();
    }

    BattleContentLoadResult BattleContentLoader::load(const std::filesystem::path& contentRoot) const
    {
        BattleContentLoadResult result;
        try
        {
            const auto document = readJson(contentRoot / "battle" / "content.json");
            if (document.value("schema_version", 0) != 1 || document.value("content_kind", std::string{}) != "battle")
            {
                throw std::runtime_error("Expected battle content schema version 1");
            }
            loadDefinitions(document, result.content);
            loadPresentation(readJson(contentRoot / "battle" / "presentation.json"), result.content);
            validateReferences(result.content, contentRoot);
        }
        catch (const std::exception& exception)
        {
            result.diagnostics.push_back({gameplay::DiagnosticSeverity::error, "battle-content", contentRoot, {}, exception.what()});
        }
        return result;
    }
} // namespace soma::content
