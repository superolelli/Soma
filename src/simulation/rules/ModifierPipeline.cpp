#include "simulation/rules/ModifierPipeline.hpp"

#include <algorithm>

namespace soma::simulation
{
    void ModifierPipeline::add(ModifierContribution contribution)
    {
        contributions_.push_back(std::move(contribution));
    }

    ModifiedValue ModifierPipeline::evaluate(const double baseValue) const
    {
        auto ordered = contributions_;
        std::ranges::stable_sort(ordered, [](const auto& left, const auto& right) {
            if (left.operation != right.operation)
            {
                return left.operation < right.operation;
            }
            return left.priority < right.priority;
        });
        ModifiedValue result{baseValue, {}};
        result.trace.reserve(ordered.size());
        for (const auto& contribution : ordered)
        {
            const auto before = result.value;
            switch (contribution.operation)
            {
            case ModifierOperation::additive:
                result.value += contribution.operand;
                break;
            case ModifierOperation::multiplicative:
                result.value *= contribution.operand;
                break;
            case ModifierOperation::clampMinimum:
                result.value = std::max(result.value, contribution.operand);
                break;
            case ModifierOperation::clampMaximum:
                result.value = std::min(result.value, contribution.operand);
                break;
            }
            result.trace.push_back({contribution.source,
                                    contribution.operation,
                                    contribution.operand,
                                    before,
                                    result.value});
        }
        return result;
    }
} // namespace soma::simulation
