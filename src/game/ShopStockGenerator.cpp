#include "game/ShopStockGenerator.hpp"

#include "gameplay/items/ItemRarity.hpp"

#include <algorithm>
#include <array>

namespace soma::game
{
    namespace
    {
        gameplay::ItemColor randomItemColor(std::mt19937& random)
        {
            switch (std::uniform_int_distribution{0, 7}(random))
            {
                case 0: return {128, 0, 0};
                case 1: return {0, 128, 0};
                case 2: return {0, 0, 128};
                case 3: return {128, 128, 0};
                default: return {};
            }
        }
    } // namespace

    ShopStockGenerator::ShopStockGenerator(std::mt19937& random)
        : random_(random)
    {
    }

    std::vector<ShopStockEntry> ShopStockGenerator::generate(const content::EconomyContent& economy,
                                                             const content::RuleContent& rules,
                                                             const CampaignState& state,
                                                             std::string_view campaign,
                                                             std::size_t slotCount) const
    {
        std::vector<gameplay::RuleFragmentId> ruleCandidates;
        for (const auto type : {content::RuleFragmentType::trigger,
                                content::RuleFragmentType::subject,
                                content::RuleFragmentType::effect,
                                content::RuleFragmentType::modifier})
        {
            const auto typeDiscovered = std::ranges::any_of(rules.fragments, [&state, type](const auto& entry) {
                return entry.second.type == type && state.ruleFragmentCount(entry.first) > 0;
            });
            if (!typeDiscovered)
            {
                continue;
            }
            std::vector<gameplay::RuleFragmentId> candidates;
            for (const auto& [id, definition] : rules.fragments)
            {
                if (definition.type == type)
                {
                    candidates.push_back(id);
                }
            }
            std::ranges::shuffle(candidates, random_);
            if (!candidates.empty())
            {
                ruleCandidates.push_back(candidates.front());
            }
        }
        std::ranges::shuffle(ruleCandidates, random_);
        const auto ruleCount = std::min(ruleCandidates.size(), slotCount / 2U);

        std::array<std::vector<gameplay::ItemId>, gameplay::maximumItemRarityLevel> itemCandidatesByLevel;
        for (const auto& [id, item] : economy.items)
        {
            const auto supportsCampaign = std::ranges::find(item.campaigns, campaign) != item.campaigns.end();
            if (item.category == gameplay::ItemCategory::equipment && supportsCampaign)
            {
                itemCandidatesByLevel.at(static_cast<std::size_t>(item.level - gameplay::minimumItemRarityLevel)).push_back(id);
            }
        }
        for (auto& candidates : itemCandidatesByLevel)
        {
            std::ranges::sort(candidates, {}, &gameplay::ItemId::value);
        }

        auto availableItemCount = std::size_t{};
        for (const auto& candidates : itemCandidatesByLevel)
        {
            availableItemCount += candidates.size();
        }
        const auto itemCount = std::min(availableItemCount, slotCount - ruleCount);

        std::vector<ShopStockEntry> result;
        result.reserve(itemCount + ruleCount);
        for (std::size_t index = 0; index < itemCount; ++index)
        {
            auto totalWeight = 0;
            for (auto level = gameplay::minimumItemRarityLevel; level <= gameplay::maximumItemRarityLevel; ++level)
            {
                if (!itemCandidatesByLevel.at(static_cast<std::size_t>(level - 1)).empty())
                {
                    totalWeight += gameplay::itemRarityWeight(level);
                }
            }
            auto selectedWeight = std::uniform_int_distribution{1, totalWeight}(random_);
            for (auto level = gameplay::minimumItemRarityLevel; level <= gameplay::maximumItemRarityLevel; ++level)
            {
                auto& candidates = itemCandidatesByLevel.at(static_cast<std::size_t>(level - 1));
                if (candidates.empty())
                {
                    continue;
                }
                selectedWeight -= gameplay::itemRarityWeight(level);
                if (selectedWeight > 0)
                {
                    continue;
                }
                const auto candidateIndex = std::uniform_int_distribution<std::size_t>{0, candidates.size() - 1}(random_);
                result.push_back({gameplay::ItemInstance{candidates.at(candidateIndex), randomItemColor(random_)}, std::nullopt});
                candidates.erase(candidates.begin() + static_cast<std::ptrdiff_t>(candidateIndex));
                break;
            }
        }
        for (std::size_t index = 0; index < ruleCount; ++index)
        {
            result.push_back({std::nullopt, ruleCandidates.at(index)});
        }
        std::ranges::shuffle(result, random_);
        return result;
    }
} // namespace soma::game
