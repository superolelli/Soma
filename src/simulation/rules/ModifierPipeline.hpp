#pragma once
#include "gameplay/common/StrongId.hpp"

#include <vector>

namespace soma::simulation
{
    enum class ModifierOperation
    {
        additive,
        multiplicative,
        clampMinimum,
        clampMaximum
    };

    struct ModifierContribution
    {
        gameplay::RuleId source;
        ModifierOperation operation{};
        double operand{};
        int priority{};
    };

    struct ModifierTraceEntry
    {
        gameplay::RuleId source;
        ModifierOperation operation{};
        double operand{};
        double before{};
        double after{};
    };

    struct ModifiedValue
    {
        double value{};
        std::vector<ModifierTraceEntry> trace;
    };

    /// Applies ordered standing-rule contributions while retaining an explainable value trace.
    class ModifierPipeline
    {
      public:
        void add(ModifierContribution contribution);
        [[nodiscard]] ModifiedValue evaluate(double baseValue) const;

      private:
        std::vector<ModifierContribution> contributions_;
    };
} // namespace soma::simulation
