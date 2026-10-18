#pragma once

#include "gameplay/common/StrongId.hpp"

#include <array>
#include <optional>

namespace soma::gameplay
{
    struct ComposedRule
    {
        std::optional<RuleFragmentId> trigger;
        std::optional<RuleFragmentId> subject;
        std::optional<RuleFragmentId> effect;
        std::optional<RuleFragmentId> modifier;

        auto operator<=>(const ComposedRule&) const = default;
    };

    inline constexpr std::size_t maximumRuleSlots = 4;
    using ComposedRules = std::array<ComposedRule, maximumRuleSlots>;
} // namespace soma::gameplay
