#pragma once

#include <string_view>

namespace soma::gameplay
{
    inline constexpr int minimumItemRarityLevel = 1;
    inline constexpr int maximumItemRarityLevel = 3;

    [[nodiscard]] constexpr int itemRarityWeight(int level) noexcept
    {
        switch (level)
        {
        case 1: return 70;
        case 2: return 25;
        case 3: return 5;
        default: return 0;
        }
    }

    [[nodiscard]] constexpr std::string_view itemRarityName(int level) noexcept
    {
        switch (level)
        {
        case 1: return "gewöhnlich";
        case 2: return "selten";
        case 3: return "episch";
        default: return "unbekannt";
        }
    }
} // namespace soma::gameplay
