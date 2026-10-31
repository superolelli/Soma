#include "game/ExplorationOutcomeService.hpp"

#include "game/EconomyService.hpp"

#include <algorithm>
#include <ranges>

namespace soma::game
{
    namespace
    {
        constexpr std::size_t allPlayerAbilityCount = 16;

        std::vector<gameplay::ItemId> newlyUnlockedItems(const CampaignState& campaign,
                                                        const content::EconomyContent& economy,
                                                        const std::set<gameplay::ItemId>& initiallyUnlocked)
        {
            std::vector<gameplay::ItemId> result;
            for (const auto& item : campaign.unlockedItems())
            {
                if (!initiallyUnlocked.contains(item) && economy.items.at(item).category != gameplay::ItemCategory::currency)
                {
                    result.push_back(item);
                }
            }
            return result;
        }

        std::vector<content::RuleFragmentType> newlyDiscoveredRuleTypes(
            const CampaignState& campaign,
            const content::RuleContent& rules,
            const std::set<content::RuleFragmentType>& initiallyDiscovered)
        {
            const auto discovered = ExplorationOutcomeService::discoveredRuleTypes(campaign.data(), rules);
            std::vector<content::RuleFragmentType> result;
            for (const auto type : discovered)
            {
                if (!initiallyDiscovered.contains(type))
                {
                    result.push_back(type);
                }
            }
            return result;
        }

        gameplay::Result<bool> grantCurrency(EconomyService& service,
                                             const content::EconomyContent& economy,
                                             const gameplay::ItemId& currency,
                                             int amount)
        {
            if (amount <= 0)
            {
                return gameplay::Result<bool>::success(false);
            }
            const auto granted = service.grant(economy.items.at(currency), amount);
            if (!granted.hasValue())
            {
                return gameplay::Result<bool>::failure(granted.error());
            }
            return gameplay::Result<bool>::success(true);
        }
    } // namespace

    EncounterFollowUp ExplorationOutcomeService::applyEncounter(ResolvedEncounterState outcome,
                                                                 int enemyCount,
                                                                 bool boss,
                                                                 ExplorationProgress& progress)
    {
        progress.playerHealth = std::move(outcome.playerHealth);
        progress.playerFatigue = std::move(outcome.playerFatigue);
        progress.playerBuffs = std::move(outcome.playerBuffs);
        progress.usedPlayerAbilities.insert(outcome.usedPlayerAbilities.begin(), outcome.usedPlayerAbilities.end());
        progress.defeated = !outcome.playersWon;
        if (progress.defeated)
        {
            return EncounterFollowUp::levelDefeat;
        }

        progress.enemiesDefeated += std::max(0, enemyCount);
        return boss ? EncounterFollowUp::bossRewards : EncounterFollowUp::battleRewards;
    }

    gameplay::Result<LevelResultSummary> ExplorationOutcomeService::completeLevel(
        const content::LevelDefinition& level,
        bool completed,
        const ExplorationProgress& progress,
        const std::set<gameplay::ItemId>& initiallyUnlockedItems,
        const std::set<content::RuleFragmentType>& initiallyDiscoveredRuleTypes,
        CampaignState& campaign,
        EconomyService& economyService,
        const content::EconomyContent& economy,
        const content::RuleContent& rules)
    {
        const auto playerDeaths = static_cast<int>(std::ranges::count_if(progress.playerHealth, [](const auto& entry) {
            return entry.second <= 0;
        }));
        const auto highestFatigue = progress.playerFatigue.empty() ? 0 : std::ranges::max(progress.playerFatigue | std::views::values);
        LevelResultSummary result;
        result.rewards = simulation::LevelRewardCalculator{}.calculate(
            {level.difficulty,
             completed,
             playerDeaths,
             progress.enemiesDefeated,
             progress.usedPlayerAbilities.size() == allPlayerAbilityCount,
             highestFatigue});
        result.newlyUnlockedItems = newlyUnlockedItems(campaign, economy, initiallyUnlockedItems);
        result.newlyDiscoveredRuleTypes = newlyDiscoveredRuleTypes(campaign, rules, initiallyDiscoveredRuleTypes);

        const auto cards = grantCurrency(economyService, economy, gameplay::ItemId{"currency.cards"}, result.rewards.totalCards());
        if (!cards.hasValue())
        {
            return gameplay::Result<LevelResultSummary>::failure(cards.error());
        }
        const auto dice = grantCurrency(economyService, economy, gameplay::ItemId{"currency.dice"}, result.rewards.totalDice());
        if (!dice.hasValue())
        {
            return gameplay::Result<LevelResultSummary>::failure(dice.error());
        }
        return gameplay::Result<LevelResultSummary>::success(std::move(result));
    }

    std::set<content::RuleFragmentType> ExplorationOutcomeService::discoveredRuleTypes(const CampaignStateData& campaign,
                                                                                        const content::RuleContent& rules)
    {
        std::set<content::RuleFragmentType> result;
        for (const auto& [id, count] : campaign.ruleFragments)
        {
            const auto definition = rules.fragments.find(id);
            if (count > 0 && definition != rules.fragments.end())
            {
                result.insert(definition->second.type);
            }
        }
        return result;
    }
} // namespace soma::game
