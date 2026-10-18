#pragma once

#include <algorithm>

namespace soma::presentation
{
    /// Stores application-wide music and sound-effect volume percentages.
    class AudioSettings
    {
      public:
        [[nodiscard]] static int musicVolume() noexcept { return musicVolume_; }
        [[nodiscard]] static int soundVolume() noexcept { return soundVolume_; }

        static void setMusicVolume(int value) noexcept { musicVolume_ = std::clamp(value, 0, 100); }
        static void setSoundVolume(int value) noexcept { soundVolume_ = std::clamp(value, 0, 100); }

        [[nodiscard]] static float music(float baseVolume) noexcept
        {
            return baseVolume * static_cast<float>(musicVolume_) / 100.0F;
        }

        [[nodiscard]] static float sound(float baseVolume = 100.0F) noexcept
        {
            return baseVolume * static_cast<float>(soundVolume_) / 100.0F;
        }

      private:
        inline static int musicVolume_{100};
        inline static int soundVolume_{100};
    };
} // namespace soma::presentation
