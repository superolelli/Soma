#include "content/world/WorldContentValidation.hpp"

#include "content/world/EncounterBudget.hpp"

#include <algorithm>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace soma::content
{
    namespace
    {
        void validateLoot(const LootTableDefinition& loot, const EconomyContent& economy, const std::string& owner)
        {
            if (loot.minimumRolls < 0 || loot.maximumRolls < loot.minimumRolls || loot.additionalRollChance < 0.0 ||
                loot.additionalRollChance > 1.0)
            {
                throw std::runtime_error(owner + " has an invalid loot roll configuration");
            }
            for (const auto& entry : loot.entries)
            {
                if (entry.weight <= 0 || !economy.items.contains(entry.item))
                {
                    throw std::runtime_error(owner + " contains an invalid loot entry");
                }
            }
        }

        bool hasLootForCampaign(const LootTableDefinition& loot, const EconomyContent& economy, std::string_view campaign)
        {
            if (loot.entries.empty())
            {
                return std::ranges::any_of(economy.items, [campaign](const auto& entry) {
                    const auto& item = entry.second;
                    return std::ranges::find(item.campaigns, campaign) != item.campaigns.end();
                });
            }
            return std::ranges::any_of(loot.entries, [&economy, campaign](const auto& entry) {
                const auto& item = economy.items.at(entry.item);
                return std::ranges::find(item.campaigns, campaign) != item.campaigns.end();
            });
        }
    } // namespace

    void detail::validateEncounterBudgetReachability(const LevelDefinition& level, const WorldContent& world)
    {
        if (reachableEncounterBudgets(level, world).empty())
        {
            throw std::runtime_error("No encounter budget in the configured range can be filled");
        }
    }

    void detail::validateWorldContent(const WorldContent& world,
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
            validateLoot(encounter.loot, economy, "Encounter " + encounter.id.value());
            if (encounter.introCue && !world.assets.contains(*encounter.introCue))
            {
                throw std::runtime_error("Encounter references a missing intro video");
            }
        }
        for (const auto& [id, lootable] : world.lootables)
        {
            static_cast<void>(id);
            validateLoot(lootable.loot, economy, "Lootable " + lootable.id.value());
            if (lootable.campaigns.empty() || std::ranges::any_of(lootable.campaigns, [&lootable, &economy](const auto& campaign) {
                    return !hasLootForCampaign(lootable.loot, economy, campaign);
                }))
            {
                throw std::runtime_error("Lootable " + lootable.id.value() + " has no eligible loot for one of its campaigns");
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
                if (!hasLootForCampaign(world.encounters.at(encounter).loot, economy, level.campaign))
                {
                    throw std::runtime_error("Encounter has no eligible loot for level campaign");
                }
            }
            validateEncounterBudgetReachability(level, world);
            if (!world.encounters.contains(level.bossEncounter) || !world.encounters.at(level.bossEncounter).boss)
            {
                throw std::runtime_error("Level references an invalid boss encounter");
            }
            if (!hasLootForCampaign(world.encounters.at(level.bossEncounter).loot, economy, level.campaign))
            {
                throw std::runtime_error("Boss encounter has no eligible loot for level campaign");
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
} // namespace soma::content
