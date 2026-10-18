#pragma once

#include "gameplay/rules/ComposedRule.hpp"

namespace soma::game
{
    using ComposedRule = gameplay::ComposedRule;
    inline constexpr auto maximumRuleSlots = gameplay::maximumRuleSlots;
    using ComposedRules = gameplay::ComposedRules;
} // namespace soma::game
