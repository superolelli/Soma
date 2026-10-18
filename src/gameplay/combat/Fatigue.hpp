#pragma once

#include <algorithm>

namespace soma::gameplay
{
    inline constexpr int maximumFatigue = 27;
    inline constexpr int fatigueStageSize = 9;
    inline constexpr int maximumFatigueStage = maximumFatigue / fatigueStageSize;

    [[nodiscard]] constexpr int fatigueStage(int fatigue) noexcept
    {
        return std::clamp(fatigue, 0, maximumFatigue) / fatigueStageSize;
    }
} // namespace soma::gameplay
