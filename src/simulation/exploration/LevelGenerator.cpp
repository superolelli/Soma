#include "simulation/exploration/LevelGenerator.hpp"

#include "content/world/EncounterBudget.hpp"

#include <algorithm>
#include <numeric>
#include <stdexcept>

namespace soma::simulation
{
    namespace
    {
        template <typename Value>
        const Value& choose(const std::vector<Value>& values, gameplay::IRandom& random)
        {
            if (values.empty())
            {
                throw std::invalid_argument("Cannot choose from an empty content list");
            }
            return values.at(static_cast<std::size_t>(random.between(0, static_cast<int>(values.size() - 1))));
        }
    } // namespace

    LevelGenerator::LevelGenerator(const content::WorldContent& content, gameplay::IRandom& random)
        : content_(content), random_(random)
    {
    }

    LevelRun LevelGenerator::generate(const gameplay::LevelId& levelId) const
    {
        const auto& level = content_.levels.at(levelId);
        auto parallax = generateParallax(level);
        const auto encounters = generateEncounters(level);
        std::vector<gameplay::LootableId> availableLootables;
        for (const auto& [id, lootable] : content_.lootables)
        {
            if (std::ranges::find(lootable.campaigns, level.campaign) != lootable.campaigns.end())
            {
                availableLootables.push_back(id);
            }
        }
        std::vector<GeneratedRoom> rooms;
        rooms.reserve(static_cast<std::size_t>(level.roomCount));
        for (auto index = 0; index < level.roomCount - 1; ++index)
        {
            GeneratedRoom room{choose(level.roomBackgrounds, random_)};
            if (!availableLootables.empty() && random_.chance(level.lootableProbability))
            {
                room.lootable = choose(availableLootables, random_);
            }
            rooms.push_back(std::move(room));
        }
        auto roomIndices = std::vector<std::size_t>(rooms.size());
        std::iota(roomIndices.begin(), roomIndices.end(), 0U);
        for (std::size_t index = roomIndices.size(); index > 1; --index)
        {
            const auto other = static_cast<std::size_t>(random_.between(0, static_cast<int>(index - 1)));
            std::swap(roomIndices.at(index - 1), roomIndices.at(other));
        }
        for (std::size_t index = 0; index < encounters.size(); ++index)
        {
            auto& room = rooms.at(roomIndices.at(index));
            room.encounter = encounters.at(index);
            room.encounterPending = true;
        }
        rooms.push_back({level.endBackground, level.bossEncounter, std::nullopt, true, true, false});
        return LevelRun{levelId, std::move(rooms), std::move(parallax)};
    }

    std::vector<gameplay::EncounterId> LevelGenerator::generateEncounters(const content::LevelDefinition& level) const
    {
        const auto validBudgets = content::reachableEncounterBudgets(level, content_);
        if (validBudgets.empty())
        {
            throw std::runtime_error("No encounter budget in the configured range can be filled");
        }
        const auto budget = choose(validBudgets, random_);
        const auto roomLimit = level.roomCount - 1;
        const content::EncounterBudgetReachability reachability{level, content_};
        const auto& reachable = reachability.byRoomCount();

        std::vector<gameplay::EncounterId> result;
        auto remainingBudget = budget;
        auto remainingRooms = roomLimit;
        while (remainingBudget > 0)
        {
            std::vector<gameplay::EncounterId> candidates;
            for (const auto& encounterId : level.encounters)
            {
                const auto cost = content_.encounters.at(encounterId).budgetCost;
                if (cost <= remainingBudget &&
                    reachable.at(static_cast<std::size_t>(remainingRooms - 1)).at(static_cast<std::size_t>(remainingBudget - cost)))
                {
                    candidates.push_back(encounterId);
                }
            }
            if (candidates.empty())
            {
                throw std::runtime_error("Encounter budget cannot be filled");
            }
            const auto selected = choose(candidates, random_);
            result.push_back(selected);
            remainingBudget -= content_.encounters.at(selected).budgetCost;
            --remainingRooms;
        }
        return result;
    }

    GeneratedParallax LevelGenerator::generateParallax(const content::LevelDefinition& level) const
    {
        const auto& presentation = content_.campaignPresentation.at(level.campaign);
        const auto tileCount = static_cast<std::size_t>(level.roomCount) + 2U;
        return {generateLayerTiles(presentation.secondLayers, tileCount),
                generateLayerTiles(presentation.thirdLayers, tileCount),
                presentation.fourthLayer,
                presentation.secondLayerDivisor,
                presentation.thirdLayerDivisor,
                presentation.fourthLayerDivisor};
    }

    std::vector<gameplay::AssetId> LevelGenerator::generateLayerTiles(const std::vector<gameplay::AssetId>& candidates,
                                                                      std::size_t count) const
    {
        std::vector<gameplay::AssetId> result;
        result.reserve(count);
        while (result.size() < count)
        {
            result.push_back(choose(candidates, random_));
        }
        return result;
    }
} // namespace soma::simulation
