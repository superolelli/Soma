#include "content/battle/BattleContentLoader.hpp"
#include "content/economy/EconomyContentLoader.hpp"
#include "content/rules/RuleContentLoader.hpp"
#include "content/world/WorldContentLoader.hpp"
#include "game/CampaignLifecycleService.hpp"
#include "game/EconomyService.hpp"
#include "game/ExplorationOutcomeService.hpp"
#include "gameplay/random/SeededRandom.hpp"
#include "persistence/SaveGame.hpp"
#include "persistence/SaveRepository.hpp"
#include "simulation/exploration/LevelGenerator.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>

namespace
{
    void check(bool condition, int line)
    {
        if (!condition)
        {
            std::cerr << "Check failed at line " << line << '\n';
            std::abort();
        }
    }

#define CHECK(condition) check((condition), __LINE__)

    void checkExplorationOutcomes(const soma::content::LevelDefinition& level,
                                  const soma::content::EconomyContent& economy,
                                  const soma::content::RuleContent& rules)
    {
        const auto simon = soma::gameplay::ContentDefinitionId{"player.simon"};
        const auto ability = soma::gameplay::AbilityId{"ability.simon.1.primary"};
        soma::game::ExplorationProgress progress;
        auto followUp = soma::game::ExplorationOutcomeService::applyEncounter(
            {true, {{simon, 18}}, {{simon, 9}}, {ability}, {}}, 3, false, progress);
        CHECK(followUp == soma::game::EncounterFollowUp::battleRewards);
        CHECK(!progress.defeated);
        CHECK(progress.enemiesDefeated == 3);
        CHECK(progress.playerHealth.at(simon) == 18);
        CHECK(progress.playerFatigue.at(simon) == 9);
        CHECK(progress.usedPlayerAbilities.contains(ability));

        followUp = soma::game::ExplorationOutcomeService::applyEncounter(
            {true, {{simon, 12}}, {{simon, 11}}, {}, {}}, 1, true, progress);
        CHECK(followUp == soma::game::EncounterFollowUp::bossRewards);
        CHECK(progress.enemiesDefeated == 4);

        followUp = soma::game::ExplorationOutcomeService::applyEncounter(
            {false, {{simon, 0}}, {{simon, 11}}, {}, {}}, 2, false, progress);
        CHECK(followUp == soma::game::EncounterFollowUp::levelDefeat);
        CHECK(progress.defeated);
        CHECK(progress.enemiesDefeated == 4);

        soma::game::CampaignState campaign;
        soma::game::EconomyService economyService{campaign};
        const auto initialItems = campaign.unlockedItems();
        const auto initialRuleTypes = soma::game::ExplorationOutcomeService::discoveredRuleTypes(campaign.data(), rules);
        const auto cardsBefore = campaign.currency(soma::gameplay::Currency::cards);
        const auto diceBefore = campaign.currency(soma::gameplay::Currency::dice);
        auto result = soma::game::ExplorationOutcomeService::completeLevel(
            level, false, progress, initialItems, initialRuleTypes, campaign, economyService, economy, rules);
        CHECK(result.hasValue());
        CHECK(campaign.currency(soma::gameplay::Currency::cards) == cardsBefore + result.value().rewards.totalCards());
        CHECK(campaign.currency(soma::gameplay::Currency::dice) == diceBefore + result.value().rewards.totalDice());
    }
} // namespace

int main(int argc, char** argv)
{
    CHECK(argc == 3);
    const auto contentRoot = std::filesystem::absolute(argv[1]);
    const auto battle = soma::content::BattleContentLoader{}.load(contentRoot);
    const auto economy = soma::content::EconomyContentLoader{}.load(contentRoot);
    const auto rules = soma::content::RuleContentLoader{}.load(contentRoot);
    const auto world = soma::content::WorldContentLoader{}.load(contentRoot, battle.content, economy.content);
    CHECK(battle.valid());
    CHECK(economy.valid());
    CHECK(rules.valid());

    const auto level = soma::gameplay::LevelId{"level.bang.1"};
    checkExplorationOutcomes(world.content.levels.at(level), economy.content, rules.content);
    soma::persistence::SaveGame save;
    save.playerName = "Lifecycle Test";
    soma::game::EconomyService service{save.campaign};
    const auto baseline = soma::game::CampaignLifecycleService::baselineForLevel(save.campaign, save.activeLevel);
    CHECK(service.grant(economy.content.items.at(soma::gameplay::ItemId{"currency.dice"}), 7).hasValue());

    soma::gameplay::SeededRandom random{1234};
    auto run = soma::simulation::LevelGenerator{world.content, random}.generate(level);
    CHECK(run.restoreProgress(1));
    soma::game::ActiveLevelState resumable{std::move(run),
                                           {.enemiesDefeated = 2,
                                            .usedPlayerAbilities = {soma::gameplay::AbilityId{"ability.simon.1.primary"}},
                                            .playerHealth = {{soma::gameplay::ContentDefinitionId{"player.simon"}, 17}},
                                            .playerFatigue = {{soma::gameplay::ContentDefinitionId{"player.simon"}, 8}},
                                            .playerBuffs = {{soma::gameplay::ContentDefinitionId{"player.simon"},
                                                             {{{.armour = 3}, 2, true}}}}},
                                           640.0F,
                                           baseline};
    auto saveTransition = soma::game::CampaignLifecycleService::apply(
        {soma::game::LevelSessionResult::saveAndMainMenu, std::move(resumable)},
        level,
        baseline,
        save.campaign,
        service,
        save.completedLevels,
        save.activeLevel);
    CHECK(saveTransition.hasValue());
    CHECK(saveTransition.value().returnToMainMenu);
    CHECK(save.activeLevel.has_value());

    const auto saveDirectory = std::filesystem::absolute(argv[2]);
    std::error_code cleanupError;
    std::filesystem::remove_all(saveDirectory, cleanupError);
    soma::persistence::SaveRepository repository{economy.content, world.content, rules.content};
    const auto savePath = saveDirectory / "active-level.json";
    CHECK(repository.save(savePath, save).hasValue());
    auto reloaded = repository.load(savePath);
    CHECK(reloaded.hasValue());
    CHECK(reloaded.value().activeLevel.has_value());
    CHECK(reloaded.value().activeLevel->run.currentRoom() == 1);
    CHECK(reloaded.value().activeLevel->progress.enemiesDefeated == 2);
    CHECK(reloaded.value().activeLevel->progress.usedPlayerAbilities.contains(soma::gameplay::AbilityId{"ability.simon.1.primary"}));
    CHECK(reloaded.value().activeLevel->progress.playerBuffs.at(soma::gameplay::ContentDefinitionId{"player.simon"}).front().attributes.armour ==
          3);
    CHECK(reloaded.value().campaign.currency(soma::gameplay::Currency::dice) == 7);

    auto resumedSave = std::move(reloaded.value());
    soma::game::EconomyService resumedService{resumedSave.campaign};
    const auto resumedBaseline = soma::game::CampaignLifecycleService::baselineForLevel(resumedSave.campaign, resumedSave.activeLevel);
    CHECK(resumedBaseline.currencies.at(soma::gameplay::Currency::dice) == 0);
    CHECK(resumedService.grant(economy.content.items.at(soma::gameplay::ItemId{"currency.dice"}), 2).hasValue());
    auto abandonTransition = soma::game::CampaignLifecycleService::apply({soma::game::LevelSessionResult::abandonAndMainMenu, std::nullopt},
                                                                         level,
                                                                         resumedBaseline,
                                                                         resumedSave.campaign,
                                                                         resumedService,
                                                                         resumedSave.completedLevels,
                                                                         resumedSave.activeLevel);
    CHECK(abandonTransition.hasValue());
    CHECK(!abandonTransition.value().returnToMainMenu);
    CHECK(!resumedSave.activeLevel.has_value());
    CHECK(resumedSave.campaign.currency(soma::gameplay::Currency::dice) == 0);

    soma::persistence::SaveGame completedSave;
    soma::game::EconomyService completedService{completedSave.campaign};
    const auto completionBaseline = completedSave.campaign.data();
    CHECK(completedService.grant(economy.content.items.at(soma::gameplay::ItemId{"currency.dice"}), 3).hasValue());
    const auto stockedItem = economy.content.items.begin()->first;
    completedService.replaceShopStock({{soma::gameplay::ItemInstance{stockedItem, {}}, std::nullopt}});
    auto completionTransition = soma::game::CampaignLifecycleService::apply({soma::game::LevelSessionResult::completed, std::nullopt},
                                                                            level,
                                                                            completionBaseline,
                                                                            completedSave.campaign,
                                                                            completedService,
                                                                            completedSave.completedLevels,
                                                                            completedSave.activeLevel);
    CHECK(completionTransition.hasValue());
    CHECK(completionTransition.value().shopStockInvalidated);
    CHECK(completedSave.completedLevels.contains(level));
    CHECK(completedSave.campaign.currency(soma::gameplay::Currency::dice) == 3);
    CHECK(completedSave.campaign.shopStock().empty());
}
