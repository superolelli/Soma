#include "content/WorldContentLoader.hpp"

#include <algorithm>
#include <fstream>
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

        template <typename Registry, typename Definition>
        void insertUnique(Registry& registry, Definition definition)
        {
            if (!registry.emplace(definition.id, std::move(definition)).second)
            {
                throw std::runtime_error("Duplicate world content ID");
            }
        }

        bool canFillEncounterBudget(const LevelDefinition& level, const WorldContent& world, int budget)
        {
            std::vector<std::vector<bool>> reachable(static_cast<std::size_t>(level.roomCount),
                                                     std::vector<bool>(static_cast<std::size_t>(budget + 1), false));
            reachable.at(0).at(0) = true;
            for (int rooms = 1; rooms < level.roomCount; ++rooms)
            {
                reachable.at(static_cast<std::size_t>(rooms)).at(0) = true;
                for (int total = 1; total <= budget; ++total)
                {
                    reachable.at(static_cast<std::size_t>(rooms)).at(static_cast<std::size_t>(total)) =
                        std::ranges::any_of(level.encounters, [&world, &reachable, rooms, total](const auto& encounterId) {
                            const auto cost = world.encounters.at(encounterId).budgetCost;
                            return cost <= total && reachable.at(static_cast<std::size_t>(rooms - 1)).at(static_cast<std::size_t>(total - cost));
                        });
                }
            }
            return std::ranges::any_of(reachable, [budget](const auto& totals) {
                return totals.at(static_cast<std::size_t>(budget));
            });
        }

        void validateReferences(const WorldContent& world,
                                const BattleContent& battle,
                                const EconomyContent& economy,
                                const std::filesystem::path& repositoryRoot)
        {
            for (const auto& [id, path] : world.assets)
            {
                static_cast<void>(id);
                if (!std::filesystem::exists(repositoryRoot / path))
                {
                    throw std::runtime_error("World content references missing asset " + path.string());
                }
            }
            for (const auto& [id, encounter] : world.encounters)
            {
                static_cast<void>(id);
                if (encounter.combatants.empty() || encounter.combatants.size() > 4)
                {
                    throw std::runtime_error("Encounter formations must contain one to four combatants");
                }
                for (const auto& combatant : encounter.combatants)
                {
                    if (!battle.combatants.contains(combatant))
                    {
                        throw std::runtime_error("Encounter references missing combatant " + combatant.value());
                    }
                }
                if (!world.lootTables.contains(encounter.rewardTable))
                {
                    throw std::runtime_error("Encounter references a missing reward table");
                }
                if (encounter.introCue && !world.assets.contains(*encounter.introCue))
                {
                    throw std::runtime_error("Encounter references a missing intro video");
                }
            }
            for (const auto& [id, table] : world.lootTables)
            {
                static_cast<void>(id);
                if (table.entries.empty() || table.minimumRolls < 0 || table.maximumRolls < table.minimumRolls || table.additionalRollChance < 0.0 ||
                    table.additionalRollChance > 1.0)
                {
                    throw std::runtime_error("Invalid loot table roll configuration");
                }
                for (const auto& entry : table.entries)
                {
                    if (entry.weight <= 0 || !economy.items.contains(entry.item))
                    {
                        throw std::runtime_error("Loot table contains an invalid item entry");
                    }
                }
            }
            for (const auto& [id, lootable] : world.lootables)
            {
                static_cast<void>(id);
                if (!world.lootTables.contains(lootable.lootTable))
                {
                    throw std::runtime_error("Lootable references a missing loot table");
                }
                if (!world.assets.contains(lootable.sprite) || !world.assets.contains(lootable.sound))
                {
                    throw std::runtime_error("Lootable references a missing presentation asset");
                }
            }
            for (const auto& [campaign, presentation] : world.campaignPresentation)
            {
                static_cast<void>(campaign);
                if (presentation.secondLayers.empty() || presentation.thirdLayers.empty() || presentation.musicTracks.empty() ||
                    presentation.secondLayerDivisor <= 0 || presentation.thirdLayerDivisor <= 0 || presentation.fourthLayerDivisor <= 0 ||
                    presentation.musicVolume < 0.0 || presentation.battleMusicVolume < 0.0 || presentation.battleCrossfadeSeconds <= 0.0 ||
                    !world.assets.contains(presentation.fourthLayer))
                {
                    throw std::runtime_error("Invalid campaign parallax presentation");
                }
                for (const auto& asset : presentation.secondLayers)
                {
                    if (!world.assets.contains(asset))
                    {
                        throw std::runtime_error("Campaign presentation references a missing second-layer asset");
                    }
                }
                for (const auto& asset : presentation.thirdLayers)
                {
                    if (!world.assets.contains(asset))
                    {
                        throw std::runtime_error("Campaign presentation references a missing third-layer asset");
                    }
                }
                for (const auto& asset : presentation.musicTracks)
                {
                    if (!world.assets.contains(asset))
                    {
                        throw std::runtime_error("Campaign presentation references a missing music asset");
                    }
                }
                for (const auto& emitter : presentation.particles)
                {
                    const auto validRange = [](const ParticleRange& range) {
                        return range.minimum <= range.maximum;
                    };
                    if (!world.assets.contains(emitter.texture) || emitter.atlasColumns <= 0 || emitter.atlasRows <= 0 ||
                        emitter.maximumParticles <= 0 || emitter.emissionRate < 0.0 || emitter.lifetimeSeconds.minimum <= 0.0 ||
                        !validRange(emitter.lifetimeSeconds) || !validRange(emitter.velocityX) || !validRange(emitter.velocityY) ||
                        emitter.scale.minimum <= 0.0 || !validRange(emitter.scale) || !validRange(emitter.angularVelocity) ||
                        emitter.oscillationFrequency < 0.0 || emitter.opacity < 0 || emitter.opacity > 255 || emitter.spawnArea.width <= 0 ||
                        emitter.spawnArea.height <= 0)
                    {
                        throw std::runtime_error("Invalid campaign particle emitter");
                    }
                }
            }
            for (const auto& [id, level] : world.levels)
            {
                static_cast<void>(id);
                if (!world.campaignPresentation.contains(level.campaign))
                {
                    throw std::runtime_error("Level references missing campaign presentation");
                }
                if (level.roomCount < 2 || level.roomBackgrounds.empty() || level.minimumEncounterBudget <= 0 ||
                    level.maximumEncounterBudget < level.minimumEncounterBudget || level.lootableProbability < 0.0 || level.lootableProbability > 1.0)
                {
                    throw std::runtime_error("Invalid level generation parameters");
                }
                for (const auto& encounter : level.encounters)
                {
                    if (!world.encounters.contains(encounter) || world.encounters.at(encounter).boss ||
                        world.encounters.at(encounter).budgetCost <= 0)
                    {
                        throw std::runtime_error("Level references an invalid regular encounter");
                    }
                }
                for (int budget = level.minimumEncounterBudget; budget <= level.maximumEncounterBudget; ++budget)
                {
                    if (!canFillEncounterBudget(level, world, budget))
                    {
                        throw std::runtime_error("Level encounter budget cannot be filled exactly: " + std::to_string(budget));
                    }
                }
                if (!world.encounters.contains(level.bossEncounter) || !world.encounters.at(level.bossEncounter).boss)
                {
                    throw std::runtime_error("Level references an invalid boss encounter");
                }
                if (!world.assets.contains(level.endBackground))
                {
                    throw std::runtime_error("Level references a missing end-room background");
                }
                for (const auto& background : level.roomBackgrounds)
                {
                    if (!world.assets.contains(background))
                    {
                        throw std::runtime_error("Level references a missing room background");
                    }
                }
            }
            const auto& speech = world.presentation.speechBubbles;
            if (!world.assets.contains(speech.texture) || speech.lines.empty() || speech.checkIntervalSeconds <= 0.0 ||
                speech.appearanceChance < 0.0 || speech.appearanceChance > 1.0 || speech.durationSeconds <= 0.0 || speech.maximumTextWidth <= 0 ||
                speech.characterSize <= 0)
            {
                throw std::runtime_error("Invalid speech-bubble presentation");
            }
            for (const auto& [speaker, lines] : speech.lines)
            {
                if (!battle.combatants.contains(speaker) || lines.empty())
                {
                    throw std::runtime_error("Speech bubbles reference an invalid speaker or empty line set");
                }
            }
            const auto& party = world.presentation;
            const auto& status = party.partyStatus;
            if (party.movementSpeed <= 0.0 || party.partyScale <= 0.0 || party.idleAnimation.empty() || party.walkAnimation.empty() ||
                party.idleAnimationSpeed <= 0.0 || party.walkAnimationSpeed <= 0.0 || status.verticalOffset < 0 || status.statusGap < 0 ||
                !world.assets.contains(status.healthBar) || !world.assets.contains(status.healthBarFrame) ||
                !world.assets.contains(status.fatigueBar) || !world.assets.contains(status.fatigueBarFrame) ||
                !world.assets.contains(status.fatigueTired) || !world.assets.contains(status.fatigueStupid) || !world.assets.contains(status.buff))
            {
                throw std::runtime_error("Invalid exploration party presentation");
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
            const auto document = readJson(path);
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
                                               value.contains("intro_cue")
                                                   ? std::optional<gameplay::AssetId>{gameplay::AssetId{value.at("intro_cue").get<std::string>()}}
                                                   : std::nullopt,
                                               gameplay::LootTableId{value.at("reward_table").get<std::string>()}};
                insertUnique(result.content.encounters, std::move(definition));
            }
            for (const auto& value : document.at("loot_tables"))
            {
                LootTableDefinition definition{gameplay::LootTableId{value.at("id").get<std::string>()}};
                definition.minimumRolls = value.value("minimum_rolls", 1);
                definition.maximumRolls = value.value("maximum_rolls", 1);
                definition.additionalRollChance = value.value("additional_roll_chance", 0.0);
                for (const auto& entry : value.at("entries"))
                {
                    definition.entries.push_back({gameplay::ItemId{entry.at("item").get<std::string>()}, entry.value("weight", 1)});
                }
                insertUnique(result.content.lootTables, std::move(definition));
            }
            for (const auto& value : document.at("lootables"))
            {
                LootableDefinition definition{gameplay::LootableId{value.at("id").get<std::string>()},
                                              value.at("name").get<std::string>(),
                                              strings(value.at("campaigns")),
                                              gameplay::LootTableId{value.at("loot_table").get<std::string>()},
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
            validateReferences(result.content, battle, economy, contentRoot.parent_path().parent_path().parent_path());
        }
        catch (const std::exception& error)
        {
            result.diagnostics.push_back({gameplay::DiagnosticSeverity::error, "world-content", path, {}, error.what()});
        }
        return result;
    }
} // namespace soma::content
