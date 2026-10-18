#pragma once

#include "content/WorldContent.hpp"

#include <optional>
#include <set>
#include <string_view>
#include <vector>

namespace soma::game
{
    /// Determines the available and next levels from world content and completed-level state.
    class CampaignProgression
    {
      public:
        CampaignProgression(const content::WorldContent& world, const std::set<gameplay::LevelId>& completedLevels);

        [[nodiscard]] std::vector<gameplay::LevelId> levels(std::string_view campaign) const;
        [[nodiscard]] std::optional<gameplay::LevelId> nextLevel(std::string_view campaign) const;
        [[nodiscard]] bool completed(std::string_view campaign) const;

      private:
        const content::WorldContent& world_;
        const std::set<gameplay::LevelId>& completedLevels_;
    };
} // namespace soma::game
