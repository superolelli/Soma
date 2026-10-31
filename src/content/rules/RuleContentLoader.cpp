#include "content/rules/RuleContentLoader.hpp"

#include <fstream>
#include <json.hpp>
#include <stdexcept>

namespace soma::content
{
    namespace
    {
        using json = nlohmann::json;

        Point point(const json& value)
        {
            if (!value.is_array() || value.size() != 2)
            {
                throw std::runtime_error("Expected a two-component point");
            }
            return {value.at(0).get<int>(), value.at(1).get<int>()};
        }

        RuleFragmentType fragmentType(std::string_view value)
        {
            if (value == "trigger")
            {
                return RuleFragmentType::trigger;
            }
            if (value == "subject")
            {
                return RuleFragmentType::subject;
            }
            if (value == "effect")
            {
                return RuleFragmentType::effect;
            }
            if (value == "modifier")
            {
                return RuleFragmentType::modifier;
            }
            throw std::runtime_error("Unknown rule fragment type");
        }

        TriggerKind triggerKind(std::string_view value)
        {
            if (value == "health_lost")
            {
                return TriggerKind::healthLost;
            }
            if (value == "health_healed")
            {
                return TriggerKind::healthHealed;
            }
            if (value == "marked_received")
            {
                return TriggerKind::markedReceived;
            }
            if (value == "decay_received")
            {
                return TriggerKind::decayReceived;
            }
            if (value == "buff_received")
            {
                return TriggerKind::buffReceived;
            }
            if (value == "debuff_received")
            {
                return TriggerKind::debuffReceived;
            }
            if (value == "combatant_died")
            {
                return TriggerKind::combatantDied;
            }
            if (value == "confused")
            {
                return TriggerKind::confused;
            }
            if (value == "high_damage_dealt")
            {
                return TriggerKind::highDamageDealt;
            }
            if (value == "turn_started")
            {
                return TriggerKind::turnStarted;
            }
            if (value == "woke_up")
            {
                return TriggerKind::wokeUp;
            }
            if (value == "marked_other")
            {
                return TriggerKind::markedOther;
            }
            if (value == "decay_applied")
            {
                return TriggerKind::decayApplied;
            }
            if (value == "healed_other")
            {
                return TriggerKind::healedOther;
            }
            if (value == "buffed_other")
            {
                return TriggerKind::buffedOther;
            }
            if (value == "debuffed_other")
            {
                return TriggerKind::debuffedOther;
            }
            if (value == "killed_other")
            {
                return TriggerKind::killedOther;
            }
            if (value == "confused_other")
            {
                return TriggerKind::confusedOther;
            }
            if (value == "sleep_applied")
            {
                return TriggerKind::sleepApplied;
            }
            if (value == "high_damage_received")
            {
                return TriggerKind::highDamageReceived;
            }
            if (value == "critical_hit_dealt")
            {
                return TriggerKind::criticalHitDealt;
            }
            if (value == "fatigue_reached_nine")
            {
                return TriggerKind::fatigueReachedNine;
            }
            if (value == "attack_dodged")
            {
                return TriggerKind::attackDodged;
            }
            if (value == "decay_damage_received")
            {
                return TriggerKind::decayDamageReceived;
            }
            throw std::runtime_error("Unknown trigger mechanic");
        }

        SubjectKind subjectKind(std::string_view value)
        {
            if (value == "this_combatant")
            {
                return SubjectKind::thisCombatant;
            }
            if (value == "all_others")
            {
                return SubjectKind::allOthers;
            }
            if (value == "all_friends")
            {
                return SubjectKind::allFriends;
            }
            if (value == "all_opponents")
            {
                return SubjectKind::allOpponents;
            }
            if (value == "random_combatant")
            {
                return SubjectKind::randomCombatant;
            }
            if (value == "random_opponent")
            {
                return SubjectKind::randomOpponent;
            }
            if (value == "random_friend")
            {
                return SubjectKind::randomFriend;
            }
            if (value == "all_marked")
            {
                return SubjectKind::allMarked;
            }
            if (value == "all_confused")
            {
                return SubjectKind::allConfused;
            }
            if (value == "all_sleeping")
            {
                return SubjectKind::allSleeping;
            }
            if (value == "all_decaying")
            {
                return SubjectKind::allDecaying;
            }
            if (value == "all_full_health")
            {
                return SubjectKind::allFullHealth;
            }
            if (value == "all_buffed")
            {
                return SubjectKind::allBuffed;
            }
            if (value == "all_debuffed")
            {
                return SubjectKind::allDebuffed;
            }
            if (value == "all_below_nine_fatigue")
            {
                return SubjectKind::allBelowNineFatigue;
            }
            if (value == "all_above_eight_fatigue")
            {
                return SubjectKind::allAboveEightFatigue;
            }
            throw std::runtime_error("Unknown subject mechanic");
        }

        EffectKind effectKind(std::string_view value)
        {
            if (value == "damage")
            {
                return EffectKind::damage;
            }
            if (value == "healing")
            {
                return EffectKind::healing;
            }
            if (value == "fatigue")
            {
                return EffectKind::fatigue;
            }
            if (value == "apply_status")
            {
                return EffectKind::applyStatus;
            }
            if (value == "apply_attribute")
            {
                return EffectKind::applyAttribute;
            }
            if (value == "dynamite")
            {
                return EffectKind::dynamite;
            }
            throw std::runtime_error("Unknown effect mechanic");
        }

        ModifierKind modifierKind(std::string_view value)
        {
            if (value == "damage_received")
            {
                return ModifierKind::damageReceived;
            }
            if (value == "damage_dealt")
            {
                return ModifierKind::damageDealt;
            }
            if (value == "armour")
            {
                return ModifierKind::armour;
            }
            if (value == "dodge")
            {
                return ModifierKind::dodge;
            }
            if (value == "critical_hit_chance")
            {
                return ModifierKind::criticalHitChance;
            }
            if (value == "precision")
            {
                return ModifierKind::precision;
            }
            if (value == "initiative")
            {
                return ModifierKind::initiative;
            }
            if (value == "noface_buff")
            {
                return ModifierKind::nofaceBuff;
            }
            throw std::runtime_error("Unknown modifier mechanic");
        }

        RuleFragmentPayload payload(RuleFragmentType type, const json& value)
        {
            const auto mechanic = value.at("mechanic").get<std::string>();
            switch (type)
            {
            case RuleFragmentType::trigger:
                return TriggerFragment{triggerKind(mechanic)};
            case RuleFragmentType::subject:
                return SubjectFragment{subjectKind(mechanic), value.value("plural", false)};
            case RuleFragmentType::effect: {
                EffectFragment result{effectKind(mechanic), value.value("amount", 0)};
                if (value.contains("status"))
                {
                    result.status = gameplay::StatusId{value.at("status").get<std::string>()};
                }
                result.duration = value.value("duration", 0);
                if (value.contains("attribute"))
                {
                    result.attribute = modifierKind(value.at("attribute").get<std::string>());
                }
                return result;
            }
            case RuleFragmentType::modifier:
                return ModifierFragment{modifierKind(mechanic), value.at("value").get<int>()};
            }
            throw std::runtime_error("Unsupported rule fragment type");
        }

        RuleFragmentSource fragmentSource(const json& acquisition, std::string_view name)
        {
            const auto key = std::string{name};
            if (!acquisition.contains(key))
            {
                return {};
            }
            const auto& source = acquisition.at(key);
            RuleFragmentSource result;
            result.enabled = true;
            result.campaigns = source.value("campaigns", std::vector<std::string>{});
            for (const auto& level : source.value("levels", std::vector<std::string>{}))
            {
                result.levels.emplace_back(level);
            }
            for (const auto& enemy : source.value("enemies", std::vector<std::string>{}))
            {
                result.enemies.emplace_back(enemy);
            }
            return result;
        }

        RuleFragmentAcquisition acquisition(const json& source)
        {
            if (!source.contains("acquisition"))
            {
                return {};
            }
            const auto& value = source.at("acquisition");
            return {fragmentSource(value, "lootables"), fragmentSource(value, "battles")};
        }
    } // namespace

    bool RuleContentLoadResult::valid() const noexcept
    {
        return diagnostics.empty();
    }

    RuleContentLoadResult RuleContentLoader::load(const std::filesystem::path& contentRoot) const
    {
        RuleContentLoadResult result;
        const auto file = contentRoot / "rules/content.json";
        try
        {
            std::ifstream stream{file};
            if (!stream)
            {
                throw std::runtime_error("file could not be opened");
            }
            json document;
            stream >> document;
            if (document.at("schema_version").get<int>() != 1 || document.at("content_kind") != "rules")
            {
                throw std::runtime_error("unsupported schema or content kind");
            }
            const auto repositoryRoot = contentRoot.parent_path().parent_path().parent_path();
            const auto checkedPath = [&](const json& value) {
                auto path = std::filesystem::path{value.get<std::string>()};
                if (!std::filesystem::is_regular_file(repositoryRoot / path))
                {
                    throw std::runtime_error("missing asset: " + path.string());
                }
                return path;
            };
            const auto& presentation = document.at("presentation");
            result.content.panel = checkedPath(presentation.at("panel"));
            result.content.fragmentBackground = checkedPath(presentation.at("fragment_background"));
            result.content.slotLock = checkedPath(presentation.at("slot_lock"));
            result.content.closeButton = checkedPath(presentation.at("close_button"));
            result.content.confirmationDialog = checkedPath(presentation.at("confirmation_dialog"));
            result.content.confirmationButton = checkedPath(presentation.at("confirmation_button"));
            result.content.activeRulesToggle = checkedPath(presentation.at("active_rules_toggle"));
            const auto& layout = presentation.at("layout");
            result.content.presentation = {layout.at("panel_x").get<int>(),
                                           layout.at("panel_y").get<int>(),
                                           layout.at("fragment_list_x").get<int>(),
                                           layout.at("fragment_list_y").get<int>(),
                                           layout.at("fragment_list_width").get<int>(),
                                           layout.at("fragment_row_height").get<int>(),
                                           layout.at("visible_fragment_rows").get<int>(),
                                           layout.at("rule_x").get<int>(),
                                           layout.at("rule_y").get<int>(),
                                           layout.at("rule_row_spacing").get<int>(),
                                           layout.at("part_width").get<int>(),
                                           layout.at("part_height").get<int>(),
                                           layout.at("part_spacing").get<int>(),
                                           layout.at("rule_content_width").get<int>(),
                                           layout.at("sigil_offset_x").get<int>(),
                                           layout.at("sigil_offset_y").get<int>(),
                                           layout.at("fragment_text_offset_x").get<int>(),
                                           layout.at("fragment_text_offset_y").get<int>(),
                                           layout.at("fragment_count_offset_x").get<int>(),
                                           layout.at("fragment_count_offset_y").get<int>(),
                                           layout.at("lock_x").get<int>(),
                                           layout.at("lock_y").get<int>(),
                                           layout.at("lock_price_x").get<int>(),
                                           layout.at("lock_price_y").get<int>(),
                                           layout.at("lock_price_width").get<int>(),
                                           layout.at("lock_price_height").get<int>(),
                                           layout.at("close_x").get<int>(),
                                           layout.at("close_y").get<int>(),
                                           layout.at("scrollbar_x").get<int>(),
                                           layout.at("scrollbar_y").get<int>(),
                                           layout.at("scrollbar_width").get<int>(),
                                           layout.at("scrollbar_height").get<int>(),
                                           layout.at("sentence_offset_y").get<int>(),
                                           layout.at("confirmation_x").get<int>(),
                                           layout.at("confirmation_y").get<int>(),
                                           layout.at("confirmation_message_y").get<int>(),
                                           layout.at("confirmation_yes_x").get<int>(),
                                           layout.at("confirmation_yes_y").get<int>(),
                                           layout.at("confirmation_no_x").get<int>(),
                                           layout.at("confirmation_no_y").get<int>(),
                                           layout.at("title_size").get<int>(),
                                           layout.at("text_size").get<int>(),
                                           layout.at("reference_width").get<int>(),
                                           layout.at("reference_height").get<int>(),
                                           layout.at("confirmation_message_width").get<int>(),
                                           layout.at("confirmation_text_size").get<int>(),
                                           layout.at("confirmation_button_text_size").get<int>()};
            const auto& activeRules = presentation.at("active_rules");
            result.content.activeRulesPresentation = {point(activeRules.at("toggle_position")),
                                                       point(activeRules.at("list_position")),
                                                       point(activeRules.at("text_inset")),
                                                       activeRules.at("text_width").get<int>(),
                                                       activeRules.at("text_size").get<int>(),
                                                       activeRules.at("row_spacing").get<int>(),
                                                       activeRules.at("empty_text").get<std::string>()};
            for (const auto type : {RuleFragmentType::trigger, RuleFragmentType::subject, RuleFragmentType::effect, RuleFragmentType::modifier})
            {
                static constexpr std::array names{"trigger", "subject", "effect", "modifier"};
                result.content.fragmentSprites.emplace(type, checkedPath(presentation.at("fragment_sprites").at(names.at(static_cast<int>(type)))));
                result.content.fragmentSigils.emplace(type, checkedPath(presentation.at("fragment_sigils").at(names.at(static_cast<int>(type)))));
            }
            result.content.lootChance = document.at("loot_chance").get<double>();
            result.content.shopPrice = document.at("shop_price").get<int>();
            if (result.content.lootChance < 0.0 || result.content.lootChance > 1.0)
            {
                throw std::runtime_error("loot_chance must be within 0..1");
            }
            for (std::size_t index = 0; index < 3; ++index)
            {
                result.content.slotUnlockCosts.at(index) = document.at("slot_unlock_costs").at(index).get<int>();
            }
            for (const auto& source : document.at("fragments"))
            {
                const auto type = fragmentType(source.at("type").get<std::string>());
                RuleModeCompatibility modes;
                for (const auto& mode : source.at("allowed_modes"))
                {
                    const auto name = mode.get<std::string>();
                    if (name == "triggered")
                    {
                        modes.triggered = true;
                    }
                    else if (name == "standing")
                    {
                        modes.standing = true;
                    }
                    else
                    {
                        throw std::runtime_error("Unknown allowed rule mode");
                    }
                }
                RuleFragmentDefinition definition{gameplay::RuleFragmentId{source.at("id").get<std::string>()},
                                                  source.at("name").get<std::string>(),
                                                  source.at("text").get<std::string>(),
                                                  source.at("sentence_template").get<std::string>(),
                                                  source.value("plural_sentence_template", source.at("sentence_template").get<std::string>()),
                                                  type,
                                                  modes,
                                                  payload(type, source),
                                                  acquisition(source)};
                if (!result.content.fragments.emplace(definition.id, std::move(definition)).second)
                {
                    throw std::runtime_error("duplicate rule fragment ID");
                }
            }
        }
        catch (const std::exception& error)
        {
            result.diagnostics.push_back({gameplay::DiagnosticSeverity::error, "invalid-rule-content", file, {}, error.what()});
        }
        return result;
    }
} // namespace soma::content
