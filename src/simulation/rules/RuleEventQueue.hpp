#pragma once
#include "gameplay/events/GameplayEvent.hpp"

#include <cstddef>
#include <deque>
#include <functional>
#include <optional>
#include <vector>

namespace soma::simulation
{
    struct QueuedRuleEvent
    {
        gameplay::GameplayEvent event;
        std::size_t depth{};
        std::optional<gameplay::EntityId> source;
        std::vector<gameplay::RuleId> activatedRules;
    };

    /// Resolves chained triggered-rule events with explicit recursion and activation limits.
    class RuleEventQueue
    {
      public:
        using Handler = std::function<void(const QueuedRuleEvent&, RuleEventQueue&)>;
        explicit RuleEventQueue(std::size_t maximumDepth = 8, std::size_t maximumActivations = 128);
        [[nodiscard]] bool enqueue(gameplay::GameplayEvent event,
                                   std::size_t depth = 0,
                                   std::optional<gameplay::EntityId> source = std::nullopt,
                                   std::vector<gameplay::RuleId> activatedRules = {});
        [[nodiscard]] bool drain(const Handler& handler);
        [[nodiscard]] bool limitReached() const noexcept;

      private:
        std::size_t maximumDepth_;
        std::size_t maximumActivations_;
        std::size_t activationCount_{};
        bool limitReached_{};
        std::deque<QueuedRuleEvent> events_;
    };
} // namespace soma::simulation
