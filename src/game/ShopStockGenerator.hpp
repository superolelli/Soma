#pragma once

#include "content/EconomyContent.hpp"
#include "content/RuleContent.hpp"
#include "game/CampaignState.hpp"

#include <cstddef>
#include <random>
#include <string_view>
#include <vector>

namespace soma::game
{
    /// Produces the shop offers retained for the duration of one completed level cycle.
    class ShopStockGenerator
    {
      public:
        explicit ShopStockGenerator(std::mt19937& random);

        [[nodiscard]] std::vector<ShopStockEntry> generate(const content::EconomyContent& economy,
                                                           const content::RuleContent& rules,
                                                           const CampaignState& state,
                                                           std::string_view campaign,
                                                           std::size_t slotCount) const;

      private:
        std::mt19937& random_;
    };
} // namespace soma::game
