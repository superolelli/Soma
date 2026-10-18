#include "simulation/rules/RuleEventQueue.hpp"

#include <utility>

namespace soma::simulation
{
    RuleEventQueue::RuleEventQueue(const std::size_t maximumDepth,
                                   const std::size_t maximumActivations)
        : maximumDepth_(maximumDepth)
        , maximumActivations_(maximumActivations)
    {
    }

    bool RuleEventQueue::enqueue(gameplay::GameplayEvent event,
                                 const std::size_t depth,
                                 std::optional<gameplay::EntityId> source,
                                 std::vector<gameplay::RuleId> activatedRules)
    {
        if (depth > maximumDepth_ || activationCount_ + events_.size() >= maximumActivations_)
        {
            limitReached_ = true;
            return false;
        }
        events_.push_back({std::move(event), depth, std::move(source), std::move(activatedRules)});
        return true;
    }

    bool RuleEventQueue::drain(const Handler& handler)
    {
        while (!events_.empty())
        {
            auto event = std::move(events_.front());
            events_.pop_front();
            ++activationCount_;
            handler(event, *this);
        }
        return !limitReached_;
    }

    bool RuleEventQueue::limitReached() const noexcept
    {
        return limitReached_;
    }
} // namespace soma::simulation
