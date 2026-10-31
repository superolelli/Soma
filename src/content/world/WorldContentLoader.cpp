#include "content/world/WorldContentLoader.hpp"
#include "content/JsonFileReader.hpp"
#include "content/world/WorldContentValidation.hpp"

#include <algorithm>
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

        Rectangle rectangle(const json& value)
        {
            if (!value.is_array() || value.size() != 4)
            {
                throw std::runtime_error("Expected a four-component rectangle");
            }
            return {value.at(0).get<int>(), value.at(1).get<int>(), value.at(2).get<int>(), value.at(3).get<int>()};
        }

        ParticleRange particleRange(const json& value)
        {
            if (!value.is_array() || value.size() != 2)
            {
                throw std::runtime_error("Expected a two-component particle range");
            }
            return {value.at(0).get<double>(), value.at(1).get<double>()};
        }

        ParticleLayer particleLayer(const std::string& value)
        {
            if (value == "behind_characters")
            {
                return ParticleLayer::behindCharacters;
            }
            if (value == "foreground")
            {
                return ParticleLayer::foreground;
            }
            throw std::runtime_error("Unknown particle layer: " + value);
        }

        template <typename Id>
        std::vector<Id> ids(const json& values)
        {
            std::vector<Id> result;
            for (const auto& value : values)
            {
                result.emplace_back(value.get<std::string>());
            }
            return result;
        }

        std::vector<std::string> strings(const json& value)
        {
            if (value.is_string())
            {
                return {value.get<std::string>()};
            }
            return value.get<std::vector<std::string>>();
        }

        LootTableDefinition readLootDefinition(const json& value, LootTableDefinition defaults = {})
        {
            defaults.minimumRolls = value.value("minimum_rolls", defaults.minimumRolls);
            defaults.maximumRolls = value.value("maximum_rolls", defaults.maximumRolls);
            defaults.additionalRollChance = value.value("additional_roll_chance", defaults.additionalRollChance);
            if (value.contains("entries"))
            {
                defaults.entries.clear();
                for (const auto& entry : value.at("entries"))
                {
                    defaults.entries.push_back(
                        {gameplay::ItemId{entry.at("item").get<std::string>()}, entry.value("weight", 1)});
                }
            }
            return defaults;
        }

        template <typename Registry, typename Definition>
        void insertUnique(Registry& registry, Definition definition)
        {
            if (!registry.emplace(definition.id, std::move(definition)).second)
            {
                throw std::runtime_error("Duplicate world content ID");
            }
        }

    } // namespace

    bool WorldContentLoadResult::valid() const noexcept
    {
        return diagnostics.empty();
    }

    WorldContentLoadResult WorldContentLoader::load(const std::filesystem::path& contentRoot,
                                                    const BattleContent& battle,
                                                    const EconomyContent& economy) const
    {
        WorldContentLoadResult result;
        const auto path = contentRoot / "world" / "content.json";
        try
        {
            const auto document = detail::readJsonFile(path);
            if (document.value("schema_version", 0) != 1 || document.value("content_kind", std::string{}) != "world_content")
            {
                throw std::runtime_error("Expected world content schema version 1");
            }
            for (const auto& value : document.at("assets"))
            {
                result.content.assets.emplace(gameplay::AssetId{value.at("id").get<std::string>()}, value.at("path").get<std::string>());
            }
            for (const auto& value : document.at("encounters"))
            {
                const auto combatants = ids<gameplay::ContentDefinitionId>(value.at("combatants"));
                auto budgetCost = 0;
                for (const auto& combatant : combatants)
                {
                    budgetCost += battle.combatants.at(combatant).encounterCost;
                }
                EncounterDefinition definition{gameplay::EncounterId{value.at("id").get<std::string>()},
                                               combatants,
                                               budgetCost,
                                               value.value("boss", false),
                                               value.contains("boss_combatant")
                                                   ? std::optional<gameplay::ContentDefinitionId>{
                                                         gameplay::ContentDefinitionId{value.at("boss_combatant").get<std::string>()}}
                                                   : std::nullopt,
                                               value.contains("intro_cue")
                                                   ? std::optional<gameplay::AssetId>{gameplay::AssetId{value.at("intro_cue").get<std::string>()}}
                                                   : std::nullopt,
                                               readLootDefinition(value.at("loot"))};
                if (definition.bossCombatant &&
                    std::ranges::find(definition.combatants, *definition.bossCombatant) == definition.combatants.end())
                {
                    throw std::runtime_error("Boss combatant is not part of encounter: " + definition.id.value());
                }
                if (definition.bossCombatant && !definition.boss)
                {
                    throw std::runtime_error("A featured boss combatant requires a boss encounter: " + definition.id.value());
                }
                if (definition.introCue.has_value() != definition.bossCombatant.has_value())
                {
                    throw std::runtime_error("An encounter intro and boss combatant must be specified together: " + definition.id.value());
                }
                insertUnique(result.content.encounters, std::move(definition));
            }
            for (const auto& value : document.at("lootables"))
            {
                LootableDefinition definition{gameplay::LootableId{value.at("id").get<std::string>()},
                                              value.at("name").get<std::string>(),
                                              strings(value.at("campaigns")),
                                              value.contains("loot")
                                                  ? readLootDefinition(value.at("loot"), LootTableDefinition{{}, 1, 9, 0.25})
                                                  : LootTableDefinition{{}, 1, 9, 0.25},
                                              gameplay::AssetId{value.at("sprite").get<std::string>()},
                                              gameplay::AssetId{value.at("sound").get<std::string>()},
                                              rectangle(value.at("hitbox")),
                                              point(value.at("position"))};
                insertUnique(result.content.lootables, std::move(definition));
            }
            for (const auto& [campaign, value] : document.at("campaign_presentation").items())
            {
                WorldContent::CampaignPresentation definition;
                definition.secondLayers = ids<gameplay::AssetId>(value.at("second_layers"));
                definition.thirdLayers = ids<gameplay::AssetId>(value.at("third_layers"));
                definition.fourthLayer = gameplay::AssetId{value.at("fourth_layer").get<std::string>()};
                definition.musicTracks = ids<gameplay::AssetId>(value.at("music_tracks"));
                definition.secondLayerDivisor = value.at("second_layer_divisor").get<int>();
                definition.thirdLayerDivisor = value.at("third_layer_divisor").get<int>();
                definition.fourthLayerDivisor = value.at("fourth_layer_divisor").get<int>();
                definition.musicVolume = value.at("music_volume").get<double>();
                definition.battleMusicVolume = value.at("battle_music_volume").get<double>();
                definition.battleCrossfadeSeconds = value.at("battle_crossfade_seconds").get<double>();
                for (const auto& particle : value.value("particles", json::array()))
                {
                    const auto acceleration = particleRange(particle.at("acceleration"));
                    const auto atlas = point(particle.at("atlas"));
                    definition.particles.push_back({gameplay::AssetId{particle.at("texture").get<std::string>()},
                                                    particleLayer(particle.value("layer", "behind_characters")),
                                                    atlas.x,
                                                    atlas.y,
                                                    particle.at("maximum_particles").get<int>(),
                                                    particle.at("emission_rate").get<double>(),
                                                    particleRange(particle.at("lifetime_seconds")),
                                                    rectangle(particle.at("spawn_area")),
                                                    particleRange(particle.at("velocity_x")),
                                                    particleRange(particle.at("velocity_y")),
                                                    acceleration.minimum,
                                                    acceleration.maximum,
                                                    particleRange(particle.at("scale")),
                                                    particleRange(particle.at("angular_velocity")),
                                                    particle.value("oscillation_amplitude", 0.0),
                                                    particle.value("oscillation_frequency", 1.0),
                                                    particle.value("opacity", 255)});
                }
                result.content.campaignPresentation.emplace(campaign, std::move(definition));
            }
            for (const auto& value : document.at("levels"))
            {
                LevelDefinition definition{gameplay::LevelId{value.at("id").get<std::string>()},
                                           value.at("campaign").get<std::string>(),
                                           value.at("difficulty").get<int>(),
                                           value.at("room_count").get<int>(),
                                           value.at("encounter_budget").at("minimum").get<int>(),
                                           value.at("encounter_budget").at("maximum").get<int>(),
                                           value.at("lootable_probability").get<double>(),
                                           ids<gameplay::AssetId>(value.at("room_backgrounds")),
                                           gameplay::AssetId{value.at("end_background").get<std::string>()},
                                           ids<gameplay::EncounterId>(value.at("encounters")),
                                           gameplay::EncounterId{value.at("boss_encounter").get<std::string>()}};
                insertUnique(result.content.levels, std::move(definition));
            }
            const auto& presentation = document.at("presentation");
            result.content.presentation.referenceSize = point(presentation.at("reference_size"));
            for (std::size_t index = 0; index < result.content.presentation.partyPositions.size(); ++index)
            {
                result.content.presentation.partyPositions.at(index) = point(presentation.at("party_positions").at(index));
            }
            result.content.presentation.movementSpeed = presentation.at("movement_speed").get<double>();
            result.content.presentation.partyScale = presentation.at("party_scale").get<double>();
            result.content.presentation.idleAnimation = presentation.at("idle_animation").get<std::string>();
            result.content.presentation.walkAnimation = presentation.at("walk_animation").get<std::string>();
            result.content.presentation.idleAnimationSpeed = presentation.at("idle_animation_speed").get<double>();
            result.content.presentation.walkAnimationSpeed = presentation.at("walk_animation_speed").get<double>();
            const auto& partyStatus = presentation.at("party_status");
            result.content.presentation.partyStatus.healthBar = gameplay::AssetId{partyStatus.at("health_bar").get<std::string>()};
            result.content.presentation.partyStatus.healthBarFrame = gameplay::AssetId{partyStatus.at("health_bar_frame").get<std::string>()};
            result.content.presentation.partyStatus.fatigueBar = gameplay::AssetId{partyStatus.at("fatigue_bar").get<std::string>()};
            result.content.presentation.partyStatus.fatigueBarFrame = gameplay::AssetId{partyStatus.at("fatigue_bar_frame").get<std::string>()};
            result.content.presentation.partyStatus.fatigueTired = gameplay::AssetId{partyStatus.at("fatigue_tired").get<std::string>()};
            result.content.presentation.partyStatus.fatigueStupid = gameplay::AssetId{partyStatus.at("fatigue_stupid").get<std::string>()};
            result.content.presentation.partyStatus.buff = gameplay::AssetId{partyStatus.at("buff").get<std::string>()};
            result.content.presentation.partyStatus.verticalOffset = partyStatus.at("vertical_offset").get<int>();
            result.content.presentation.partyStatus.statusGap = partyStatus.at("status_gap").get<int>();
            const auto& lootDialog = presentation.at("loot_dialog");
            result.content.presentation.lootDialog.position = point(lootDialog.at("position"));
            result.content.presentation.lootDialog.titleCenter = point(lootDialog.at("title_center"));
            result.content.presentation.lootDialog.firstItem = point(lootDialog.at("first_item"));
            result.content.presentation.lootDialog.itemSpacing = point(lootDialog.at("item_spacing"));
            result.content.presentation.lootDialog.closeButton = point(lootDialog.at("close_button"));
            result.content.presentation.lootDialog.takeAllButton = point(lootDialog.at("take_all_button"));
            result.content.presentation.lootDialog.itemSize = point(lootDialog.at("item_size"));
            result.content.presentation.lootDialog.quantityOffset = point(lootDialog.at("quantity_offset"));
            result.content.presentation.lootDialog.errorCenter = point(lootDialog.at("error_center"));
            result.content.presentation.lootDialog.columns = lootDialog.at("columns").get<int>();
            const auto& levelResult = presentation.at("level_result_dialog");
            result.content.presentation.levelResultDialog.title = point(levelResult.at("title"));
            result.content.presentation.levelResultDialog.rewardDescription = point(levelResult.at("reward_description"));
            result.content.presentation.levelResultDialog.rewardCards = point(levelResult.at("reward_cards"));
            result.content.presentation.levelResultDialog.rewardDice = point(levelResult.at("reward_dice"));
            result.content.presentation.levelResultDialog.totalCards = point(levelResult.at("total_cards"));
            result.content.presentation.levelResultDialog.totalDice = point(levelResult.at("total_dice"));
            result.content.presentation.levelResultDialog.continueButton = point(levelResult.at("continue_button"));
            result.content.presentation.levelResultDialog.unlockTitle = point(levelResult.at("unlock_title"));
            result.content.presentation.levelResultDialog.firstUnlockedItem = point(levelResult.at("first_unlocked_item"));
            result.content.presentation.levelResultDialog.unlockedItemSpacing = point(levelResult.at("unlocked_item_spacing"));
            const auto& transition = presentation.at("battle_transition");
            result.content.presentation.battleTransition.model = gameplay::AssetId{transition.at("model").get<std::string>()};
            result.content.presentation.battleTransition.entity = transition.at("entity").get<std::string>();
            result.content.presentation.battleTransition.animation = transition.at("animation").get<std::string>();
            result.content.presentation.battleTransition.position = point(transition.at("position"));
            result.content.presentation.battleTransition.scale = transition.at("scale").get<double>();
            result.content.presentation.battleTransition.delayAfterSeconds = transition.at("delay_after_seconds").get<double>();
            const auto& speechBubbles = presentation.at("speech_bubbles");
            auto& speech = result.content.presentation.speechBubbles;
            speech.texture = gameplay::AssetId{speechBubbles.at("texture").get<std::string>()};
            speech.checkIntervalSeconds = speechBubbles.at("check_interval_seconds").get<double>();
            speech.appearanceChance = speechBubbles.at("appearance_chance").get<double>();
            speech.durationSeconds = speechBubbles.at("duration_seconds").get<double>();
            speech.maximumTextWidth = speechBubbles.at("maximum_text_width").get<int>();
            speech.characterSize = speechBubbles.at("character_size").get<int>();
            speech.padding = point(speechBubbles.at("padding"));
            speech.speakerOffset = point(speechBubbles.at("speaker_offset"));
            speech.textVerticalOffset = speechBubbles.at("text_vertical_offset").get<int>();
            for (const auto& value : speechBubbles.at("speakers"))
            {
                const auto speaker = gameplay::ContentDefinitionId{value.at("id").get<std::string>()};
                if (!speech.lines.emplace(speaker, value.at("lines").get<std::vector<std::string>>()).second)
                {
                    throw std::runtime_error("Duplicate speech-bubble speaker");
                }
            }
            detail::validateWorldContent(result.content, battle, economy, contentRoot.parent_path().parent_path().parent_path());
        }
        catch (const std::exception& error)
        {
            result.diagnostics.push_back({gameplay::DiagnosticSeverity::error, "world-content", path, {}, error.what()});
        }
        return result;
    }
} // namespace soma::content
