#pragma once
#include <chrono>

namespace soma::gameplay
{
    /// Abstracts elapsed time for systems that must be testable without wall-clock delays.
    class IClock
    {
      public:
        using TimePoint = std::chrono::steady_clock::time_point;
        virtual ~IClock() = default;
        [[nodiscard]] virtual TimePoint now() const = 0;
    };

    /// Supplies production elapsed time from the standard monotonic clock.
    class SteadyClock final : public IClock
    {
      public:
        [[nodiscard]] TimePoint now() const override
        {
            return std::chrono::steady_clock::now();
        }
    };
} // namespace soma::gameplay
