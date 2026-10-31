#pragma once

#include <algorithm>

namespace soma::presentation
{
    /// Stores the player-selected multiplier used by staged battle presentation.
    class BattlePlaybackSettings
    {
      public:
        [[nodiscard]] static int speedPercent() noexcept
        {
            return speedPercent_;
        }

        static void setSpeedPercent(int value) noexcept
        {
            speedPercent_ = std::clamp(value, 50, 200);
        }

        [[nodiscard]] static float timeScale() noexcept
        {
            return static_cast<float>(speedPercent_) / 100.0F;
        }

      private:
        inline static int speedPercent_{100};
    };
} // namespace soma::presentation
