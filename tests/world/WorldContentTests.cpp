#include "content/BattleContentLoader.hpp"
#include "content/EconomyContentLoader.hpp"
#include "content/WorldContentLoader.hpp"
#include "content/RuleContentLoader.hpp"
#include "gameplay/random/SeededRandom.hpp"
#include "simulation/exploration/LevelGenerator.hpp"
#include "simulation/rewards/LootGenerator.hpp"
#include "simulation/rewards/LevelRewardCalculator.hpp"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <source_location>
#include <string>

namespace
{
    void check(bool condition, const std::source_location location = std::source_location::current())
    {
        if (!condition)
        {
            std::cerr << "Check failed at line " << location.line() << '\n';
            std::exit(1);
        }
    }
}

int main(int argc, char** argv)
{
    check(argc == 2);
    const auto root = std::filesystem::absolute(argv[1]);
    const auto battle = soma::content::BattleContentLoader{}.load(root);
    const auto economy = soma::content::EconomyContentLoader{}.load(root);
    check(battle.valid());
    check(economy.valid());
    const auto world = soma::content::WorldContentLoader{}.load(root, battle.content, economy.content);
    for (const auto& diagnostic : world.diagnostics)
    {
        std::cerr << diagnostic.code << ": " << diagnostic.message << '\n';
    }
    check(world.valid());
    check(world.content.levels.size() == 8);
    check(world.content.encounters.size() == 95);
    check(world.content.lootables.size() == 17);
    check(world.content.lootTables.size() == 112);
    struct ExpectedLevel
    {
        const char* id;
        int difficulty;
        int rooms;
        std::size_t encounters;
        const char* boss;
    };
    constexpr std::array expectedLevels{
        ExpectedLevel{"level.bang.1", 1, 5, 6, "encounter.bang.1.boss"},
        ExpectedLevel{"level.bang.2", 2, 6, 12, "encounter.bang.2.boss"},
        ExpectedLevel{"level.bang.3", 3, 7, 10, "encounter.bang.3.boss"},
        ExpectedLevel{"level.bang.4", 4, 8, 12, "encounter.bang.4.boss"},
        ExpectedLevel{"level.bang.5", 5, 9, 14, "encounter.bang.5.boss"},
        ExpectedLevel{"level.bang.6", 6, 10, 14, "encounter.bang.6.boss"},
        ExpectedLevel{"level.kutschfahrt.1", 1, 5, 8, "encounter.kutschfahrt.1.boss"},
        ExpectedLevel{"level.kutschfahrt.2", 2, 6, 11, "encounter.kutschfahrt.2.boss"},
    };
    for (const auto& expected : expectedLevels)
    {
        const auto& level = world.content.levels.at(soma::gameplay::LevelId{expected.id});
        check(level.difficulty == expected.difficulty);
        check(level.roomCount == expected.rooms);
        check(level.encounters.size() == expected.encounters);
        check(level.bossEncounter == soma::gameplay::EncounterId{expected.boss});
        check(world.content.encounters.at(level.bossEncounter).boss);
    }
    check(std::ranges::count_if(world.content.encounters, [](const auto& entry) {
              return entry.second.introCue.has_value();
          }) == 6);
    constexpr std::array bossVideos{
        std::pair{"encounter.bang.2.boss", "video.intro.greg"},
        std::pair{"encounter.bang.3.boss", "video.intro.apache_kid"},
        std::pair{"encounter.bang.4.boss", "video.intro.tequila_joe"},
        std::pair{"encounter.bang.5.boss", "video.intro.big_spencer"},
        std::pair{"encounter.bang.6.boss", "video.intro.bill_noface"},
        std::pair{"encounter.kutschfahrt.2.boss", "video.intro.basilius_kartov"},
    };
    for (const auto& [encounterId, videoId] : bossVideos)
    {
        const auto& cue = world.content.encounters.at(soma::gameplay::EncounterId{encounterId}).introCue;
        check(cue == soma::gameplay::AssetId{videoId});
    }
    check(world.content.presentation.speechBubbles.lines.size() == 4);
    check(world.content.presentation.speechBubbles.lines.at(soma::gameplay::ContentDefinitionId{"player.simon"}).size() == 4);
    check(world.content.presentation.speechBubbles.lines.at(soma::gameplay::ContentDefinitionId{"player.ole"}).size() == 6);
    check(world.content.presentation.partyScale == 0.6);
    check(world.content.presentation.walkAnimationSpeed == 0.85);
    check(world.content.presentation.partyPositions.at(0).x == 100);
    check(world.content.presentation.partyPositions.at(3).x == 580);
    check(world.content.presentation.partyStatus.fatigueBar == soma::gameplay::AssetId{"exploration.fatigue_bar"});
    check(world.content.presentation.partyStatus.fatigueBarFrame == soma::gameplay::AssetId{"exploration.fatigue_bar_frame"});
    const auto& bangParticles = world.content.campaignPresentation.at("bang").particles;
    const auto& kutschfahrtParticles = world.content.campaignPresentation.at("kutschfahrt").particles;
    check(bangParticles.size() == 1);
    check(bangParticles.front().texture == soma::gameplay::AssetId{"particle.dust"});
    check(bangParticles.front().atlasColumns == 2);
    check(bangParticles.front().atlasRows == 2);
    check(bangParticles.front().spawnArea.y < 100);
    check(bangParticles.front().velocityY.minimum < 0.0);
    check(bangParticles.front().velocityY.maximum > 0.0);
    check(bangParticles.front().accelerationY == 0.0);
    check(kutschfahrtParticles.size() == 1);
    check(kutschfahrtParticles.front().texture == soma::gameplay::AssetId{"particle.leaves"});
    check(kutschfahrtParticles.front().velocityY.minimum > 0.0);
    const auto& first = world.content.levels.at(soma::gameplay::LevelId{"level.bang.1"});
    check(first.roomCount == 5);
    check(first.encounters.size() == 6);
    check(world.content.encounters.at(first.bossEncounter).boss);
    const auto& firstEncounter = world.content.encounters.at(first.encounters.front());
    check(world.content.lootTables.contains(firstEncounter.rewardTable));

    soma::gameplay::SeededRandom firstRandom{42};
    soma::gameplay::SeededRandom secondRandom{42};
    auto firstRun = soma::simulation::LevelGenerator{world.content, firstRandom}.generate(first.id);
    auto secondRun = soma::simulation::LevelGenerator{world.content, secondRandom}.generate(first.id);
    check(firstRun.rooms() == secondRun.rooms());
    check(firstRun.parallax() == secondRun.parallax());
    check(!firstRun.parallax().secondLayerTiles.empty());
    check(!firstRun.parallax().thirdLayerTiles.empty());
    check(firstRun.rooms().size() == 5);
    check(firstRun.rooms().back().boss);
    auto generatedBudget = 0;
    for (const auto& room : firstRun.rooms())
    {
        if (room.encounter.has_value() && !room.boss)
        {
            generatedBudget += world.content.encounters.at(*room.encounter).budgetCost;
        }
    }
    check(generatedBudget >= first.minimumEncounterBudget);
    check(generatedBudget <= first.maximumEncounterBudget);
    check(firstRun.enterRoom(1));
    check(!firstRun.enterRoom(3));

    const auto lootableId = soma::gameplay::LootableId{"lootable.cup"};
    soma::simulation::LevelRun visibleLootRun{
        soma::gameplay::LevelId{"test.visible-loot"},
        {{soma::gameplay::AssetId{"background.current"}, std::nullopt, std::nullopt, false, false, false},
         {soma::gameplay::AssetId{"background.next"}, std::nullopt, lootableId, true, false, false}}};
    check(visibleLootRun.openLootable(1));
    check(visibleLootRun.rooms().at(1).lootableOpened);
    check(!visibleLootRun.openLootable(1));

    const auto rules = soma::content::RuleContentLoader{}.load(root);
    check(rules.valid());
    soma::gameplay::SeededRandom firstLootRandom{7};
    soma::gameplay::SeededRandom secondLootRandom{7};
    const soma::simulation::LootGenerator firstLoot{world.content, economy.content, rules.content, firstLootRandom};
    const soma::simulation::LootGenerator secondLoot{world.content, economy.content, rules.content, secondLootRandom};
    const soma::simulation::RuleLootContext bangLootable{
        soma::simulation::RuleRewardSource::lootable, "bang", first.id, {}};
    const auto firstGrants = firstLoot.generate(soma::gameplay::LootTableId{"loot.cup"}, "bang", 3, bangLootable);
    const auto secondGrants = secondLoot.generate(soma::gameplay::LootTableId{"loot.cup"}, "bang", 3, bangLootable);
    check(firstGrants.items == secondGrants.items);
    check(firstGrants.ruleFragment == secondGrants.ruleFragment);
    check(!firstGrants.items.empty());
    check(firstGrants.items.size() <= 9);

    soma::content::EconomyContent rarityEconomy;
    const auto rarityTemplate = economy.content.items.at(soma::gameplay::ItemId{"item.bang.beer"});
    for (auto level = 1; level <= 3; ++level)
    {
        auto definition = rarityTemplate;
        definition.id = soma::gameplay::ItemId{"item.test.rarity." + std::to_string(level)};
        definition.level = level;
        definition.campaigns = {"rarity-test"};
        rarityEconomy.items.emplace(definition.id, std::move(definition));
    }
    soma::content::WorldContent rarityWorld;
    const auto rarityTableId = soma::gameplay::LootTableId{"loot.test.rarity"};
    rarityWorld.lootTables.emplace(
        rarityTableId,
        soma::content::LootTableDefinition{rarityTableId, {{soma::gameplay::ItemId{"item.test.rarity.1"}, 1}}, 1, 1, 0.0});
    soma::content::RuleContent noRules;
    soma::gameplay::SeededRandom rarityLootRandom{4242};
    const soma::simulation::LootGenerator rarityLoot{rarityWorld, rarityEconomy, noRules, rarityLootRandom};
    const soma::simulation::RuleLootContext rarityContext{
        soma::simulation::RuleRewardSource::lootable, "rarity-test", soma::gameplay::LevelId{"level.test"}, {}};
    std::array<int, 4> lootRarityCounts{};
    for (auto iteration = 0; iteration < 5'000; ++iteration)
    {
        const auto bundle = rarityLoot.generate(rarityTableId, "rarity-test", 1, rarityContext);
        for (const auto& grant : bundle.items)
        {
            const auto& item = rarityEconomy.items.at(grant.item.definition);
            if (item.category != soma::gameplay::ItemCategory::currency)
            {
                ++lootRarityCounts.at(static_cast<std::size_t>(item.level));
            }
        }
    }
    const auto itemLootCount = lootRarityCounts.at(1) + lootRarityCounts.at(2) + lootRarityCounts.at(3);
    check(itemLootCount > 1'000);
    check(lootRarityCounts.at(1) * 100 / itemLootCount >= 65);
    check(lootRarityCounts.at(1) * 100 / itemLootCount <= 75);
    check(lootRarityCounts.at(2) * 100 / itemLootCount >= 20);
    check(lootRarityCounts.at(2) * 100 / itemLootCount <= 30);
    check(lootRarityCounts.at(3) > 0);

    soma::gameplay::SeededRandom rewardRandom{11};
    const soma::simulation::LootGenerator rewardGenerator{world.content, economy.content, rules.content, rewardRandom};
    const soma::simulation::RuleLootContext firstBattle{
        soma::simulation::RuleRewardSource::battle, first.campaign, first.id, firstEncounter.combatants};
    const auto battleRewards = rewardGenerator.generate(firstEncounter.rewardTable, first.campaign, first.difficulty, firstBattle);
    check(!battleRewards.items.empty());
    check(battleRewards.items.size() <= 9);

    auto guaranteedRules = rules.content;
    guaranteedRules.lootChance = 1.0;
    soma::gameplay::SeededRandom guaranteedRuleRandom{22};
    const soma::simulation::LootGenerator guaranteedRuleLoot{world.content, economy.content, guaranteedRules, guaranteedRuleRandom};
    check(guaranteedRuleLoot.generate(firstEncounter.rewardTable, first.campaign, first.difficulty, firstBattle).ruleFragment.has_value());

    auto bountyOnly = guaranteedRules;
    const auto bountyId = soma::gameplay::RuleFragmentId{"rule.effect.bounty_three"};
    const auto bounty = bountyOnly.fragments.at(bountyId);
    bountyOnly.fragments.clear();
    bountyOnly.fragments.emplace(bountyId, bounty);
    const auto rollBounty = [&](const soma::simulation::RuleLootContext& context, std::uint64_t seed) {
        soma::gameplay::SeededRandom random{seed};
        return soma::simulation::LootGenerator{world.content, economy.content, bountyOnly, random}.rollRuleFragment(context);
    };
    check(rollBounty(bangLootable, 30) == bountyId);
    const soma::simulation::RuleLootContext kutschfahrtLootable{
        soma::simulation::RuleRewardSource::lootable, "kutschfahrt", soma::gameplay::LevelId{"level.kutschfahrt.1"}, {}};
    check(!rollBounty(kutschfahrtLootable, 31));
    const soma::simulation::RuleLootContext sheriffBattle{soma::simulation::RuleRewardSource::battle,
                                                          "bang",
                                                          first.id,
                                                          {soma::gameplay::ContentDefinitionId{"enemy.hilfssheriff"}}};
    check(rollBounty(sheriffBattle, 32) == bountyId);
    const soma::simulation::RuleLootContext outlawBattle{soma::simulation::RuleRewardSource::battle,
                                                         "bang",
                                                         first.id,
                                                         {soma::gameplay::ContentDefinitionId{"enemy.gesetzloser"}}};
    check(!rollBounty(outlawBattle, 33));

    const auto rewards = soma::simulation::LevelRewardCalculator{}.calculate({3, true, 0, 7, true, 67});
    check(rewards.totalCards() == 119);
    check(rewards.totalDice() == 3);
}
