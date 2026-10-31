#include "content/battle/BattleContentLoader.hpp"
#include "content/economy/EconomyContentLoader.hpp"
#include "content/flow/GameFlowContentLoader.hpp"
#include "content/rules/RuleContentLoader.hpp"
#include "content/world/WorldContentLoader.hpp"
#include "game/EconomyService.hpp"
#include "game/CampaignProgression.hpp"
#include "persistence/SaveRepository.hpp"
#include "persistence/SaveSlotService.hpp"
#include "simulation/exploration/LevelGenerator.hpp"
#include "gameplay/random/SeededRandom.hpp"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>

namespace
{
    void check(bool condition)
    {
        if (!condition)
        {
            std::abort();
        }
    }
}

int main(int argc, char** argv)
{
    check(argc == 4);
    const auto contentRoot = std::filesystem::absolute(argv[1]);
    const auto battle = soma::content::BattleContentLoader{}.load(contentRoot);
    const auto economy = soma::content::EconomyContentLoader{}.load(contentRoot);
    const auto world = soma::content::WorldContentLoader{}.load(contentRoot, battle.content, economy.content);
    const auto flow = soma::content::GameFlowContentLoader{}.load(contentRoot);
    const auto rules = soma::content::RuleContentLoader{}.load(contentRoot);
    check(battle.valid());
    check(economy.valid());
    check(world.valid());
    check(flow.valid());

    soma::persistence::SaveRepository repository{economy.content, world.content, rules.content};
    soma::persistence::SaveGame original;
    original.playerName = "Käpt'n Test";
    original.completedLevels.emplace("level.bang.1");
    soma::game::EconomyService service{original.campaign};
    check(service.grant(economy.content.items.at(soma::gameplay::ItemId{"currency.dice"}), 7).hasValue());
    const auto stockedItem = economy.content.items.begin()->first;
    const auto stockedFragment = rules.content.fragments.begin()->first;
    std::vector<soma::game::ShopStockEntry> shopStock;
    shopStock.push_back({soma::gameplay::ItemInstance{stockedItem, {12, 34, 56}}, std::nullopt});
    shopStock.push_back({std::nullopt, stockedFragment});
    service.replaceShopStock(std::move(shopStock));
    soma::gameplay::SeededRandom levelRandom{1234};
    auto activeRun =
        soma::simulation::LevelGenerator{world.content, levelRandom}.generate(soma::gameplay::LevelId{"level.bang.1"});
    check(activeRun.restoreProgress(1));
    original.activeLevel = soma::game::ActiveLevelState{std::move(activeRun),
                                                        {.enemiesDefeated = 4,
                                                         .usedPlayerAbilities = {soma::gameplay::AbilityId{"ability.simon.1.primary"}},
                                                         .playerHealth = {{soma::gameplay::ContentDefinitionId{"player.simon"}, 42}},
                                                         .playerFatigue = {{soma::gameplay::ContentDefinitionId{"player.simon"}, 23}},
                                                         .playerBuffs = {{soma::gameplay::ContentDefinitionId{"player.simon"},
                                                                          {{{.armour = 2, .precision = 5}, 3, true}}}}},
                                                        1550.0F,
                                                        original.campaign.data()};

    const auto saveDirectory = std::filesystem::absolute(argv[2]);
    std::error_code cleanupError;
    std::filesystem::remove_all(saveDirectory, cleanupError);
    const auto savePath = saveDirectory / "round-trip.json";
    check(repository.save(savePath, original).hasValue());
    const auto loaded = repository.load(savePath);
    check(loaded.hasValue());
    check(loaded.value().playerName == original.playerName);
    check(loaded.value().campaign.currency(soma::gameplay::Currency::cards) == 1000);
    check(loaded.value().campaign.currency(soma::gameplay::Currency::dice) == 7);
    check(loaded.value().campaign.shopStock() == original.campaign.shopStock());
    check(loaded.value().completedLevels == original.completedLevels);
    check(loaded.value().activeLevel.has_value());
    check(loaded.value().activeLevel->run.currentRoom() == 1);
    check(loaded.value().activeLevel->progress.playerFatigue.at(soma::gameplay::ContentDefinitionId{"player.simon"}) == 23);
    check(loaded.value().activeLevel->viewLeft == 1550.0F);
    check(loaded.value().activeLevel->progress.playerHealth.at(soma::gameplay::ContentDefinitionId{"player.simon"}) == 42);
    check(loaded.value().activeLevel->progress.enemiesDefeated == 4);
    check(loaded.value().activeLevel->progress.usedPlayerAbilities.contains(soma::gameplay::AbilityId{"ability.simon.1.primary"}));
    const auto& loadedBuff = loaded.value().activeLevel->progress.playerBuffs.at(soma::gameplay::ContentDefinitionId{"player.simon"}).front();
    check(loadedBuff.attributes.armour == 2);
    check(loadedBuff.attributes.precision == 5);
    check(loadedBuff.remainingTurns == 3);
    check(loaded.value().activeLevel->baseline.currencies.at(soma::gameplay::Currency::dice) == 7);

    const auto developerSave = repository.load(std::filesystem::absolute(argv[3]));
    check(developerSave.hasValue());
    check(developerSave.value().campaign.inventory().size() >= 30U);
    check(developerSave.value().campaign.data().ruleFragments.size() == rules.content.fragments.size());
    check(developerSave.value().campaign.ruleSlotsUnlocked() == 4);
    check(developerSave.value().campaign.currency(soma::gameplay::Currency::cards) >= 10'000);

    soma::game::CampaignProgression progression{world.content, original.completedLevels};
    check(progression.nextLevel("bang") == soma::gameplay::LevelId{"level.bang.2"});
    check(progression.nextLevel("kutschfahrt") == soma::gameplay::LevelId{"level.kutschfahrt.1"});
    check(!progression.completed("bang"));

    soma::persistence::SaveGame maximum;
    maximum.playerName = "Vollständig";
    auto maximumData = maximum.campaign.data();
    maximumData.currencies[soma::gameplay::Currency::cards] = 1'000'000;
    maximumData.currencies[soma::gameplay::Currency::dice] = 1'000'000;
    maximumData.shopSlotsUnlocked = 5;
    maximumData.shopDicePrice = 50;
    for (const auto& [id, item] : economy.content.items)
    {
        if (item.category == soma::gameplay::ItemCategory::equipment)
        {
            maximumData.inventory.push_back({id, {255, 255, 255}});
            maximumData.unlockedItems.insert(id);
        }
        else if (item.category == soma::gameplay::ItemCategory::consumable && maximumData.consumables.size() < 5U)
        {
            maximumData.consumables.emplace(id, 999);
            maximumData.unlockedItems.insert(id);
        }
    }
    for (const auto& [id, skill] : economy.content.skills)
    {
        static_cast<void>(skill);
        maximumData.acquiredSkills.insert(id);
    }
    for (const auto& [id, level] : world.content.levels)
    {
        static_cast<void>(level);
        maximum.completedLevels.insert(id);
    }
    maximum.campaign = soma::game::CampaignState::fromData(std::move(maximumData));
    const auto maximumPath = saveDirectory / "maximum.json";
    check(repository.save(maximumPath, maximum).hasValue());
    const auto maximumLoaded = repository.load(maximumPath);
    check(maximumLoaded.hasValue());
    check(maximumLoaded.value().completedLevels.size() == world.content.levels.size());
    check(soma::game::CampaignProgression{world.content, maximumLoaded.value().completedLevels}.completed("bang"));
    check(!soma::game::CampaignProgression{world.content, maximumLoaded.value().completedLevels}.nextLevel("bang"));

    const auto malformedPath = saveDirectory / "malformed.json";
    {
        std::ofstream malformed{malformedPath};
        malformed << R"({"schema_version":1,"player_name":"Test","campaign":{}})";
    }
    check(!repository.load(malformedPath).hasValue());

    auto invalid = original;
    auto invalidData = invalid.campaign.data();
    invalidData.consumables.emplace(soma::gameplay::ItemId{"missing.item"}, 1);
    invalid.campaign = soma::game::CampaignState::fromData(std::move(invalidData));
    check(!repository.save(saveDirectory / "invalid.json", invalid).hasValue());

    const auto slotsDirectory = saveDirectory / "slots";
    soma::persistence::SaveSlotService slots{slotsDirectory, repository};
    const auto initiallyEmpty = slots.discover();
    check(std::ranges::all_of(initiallyEmpty, [](const auto& slot) {
        return slot.state == soma::persistence::SaveSlotState::empty;
    }));
    check(!slots.create(-1, "Invalid").hasValue());
    check(!slots.create(3, "Invalid").hasValue());
    check(!slots.create(0, "").hasValue());
    check(slots.create(0, "Erster Spielstand").hasValue());
    check(!slots.create(0, "Nicht überschreiben").hasValue());
    check(slots.create(0, "Überschrieben", true).hasValue());
    check(slots.load(0).hasValue());
    check(slots.load(0).value().playerName == "Überschrieben");
    const auto occupied = slots.discover();
    check(occupied.at(0).state == soma::persistence::SaveSlotState::occupied);
    check(occupied.at(0).playerName == "Überschrieben");
    check(occupied.at(1).state == soma::persistence::SaveSlotState::empty);

    std::filesystem::create_directories(slotsDirectory);
    {
        std::ofstream corrupt{slots.pathFor(1)};
        corrupt << "not json";
    }
    const auto withCorruptSlot = slots.discover();
    check(withCorruptSlot.at(1).state == soma::persistence::SaveSlotState::invalid);
    check(!withCorruptSlot.at(1).error.empty());
    check(slots.remove(1).hasValue());
    check(!slots.remove(1).hasValue());
    check(slots.remove(0).hasValue());
    check(slots.discover().at(0).state == soma::persistence::SaveSlotState::empty);
}
