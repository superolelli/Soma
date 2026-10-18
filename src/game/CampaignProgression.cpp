#include "game/CampaignProgression.hpp"

#include <algorithm>
#include <tuple>

namespace soma::game
{
    CampaignProgression::CampaignProgression(const content::WorldContent& world,
                                             const std::set<gameplay::LevelId>& completedLevels)
        : world_(world), completedLevels_(completedLevels)
    {
    }

    std::vector<gameplay::LevelId> CampaignProgression::levels(std::string_view campaign) const
    {
        std::vector<const content::LevelDefinition*> definitions;
        for (const auto& [id, level] : world_.levels)
        {
            static_cast<void>(id);
            if (level.campaign == campaign)
            {
                definitions.push_back(&level);
            }
        }
        std::ranges::sort(definitions, [](const auto* left, const auto* right) {
            return std::tie(left->difficulty, left->id) < std::tie(right->difficulty, right->id);
        });

        std::vector<gameplay::LevelId> result;
        result.reserve(definitions.size());
        std::ranges::transform(definitions, std::back_inserter(result), &content::LevelDefinition::id);
        return result;
    }

    std::optional<gameplay::LevelId> CampaignProgression::nextLevel(std::string_view campaign) const
    {
        const auto campaignLevels = levels(campaign);
        const auto next = std::ranges::find_if(campaignLevels, [&](const auto& level) {
            return !completedLevels_.contains(level);
        });
        return next == campaignLevels.end() ? std::nullopt : std::optional{*next};
    }

    bool CampaignProgression::completed(std::string_view campaign) const
    {
        const auto campaignLevels = levels(campaign);
        return !campaignLevels.empty() && std::ranges::all_of(campaignLevels, [&](const auto& level) {
            return completedLevels_.contains(level);
        });
    }
} // namespace soma::game
