#pragma once

#include "gameplay/rules/ComposedRule.hpp"

#include <cstddef>
#include <string>

namespace soma::simulation::detail
{
    [[nodiscard]] inline gameplay::RuleId activeRuleId(std::size_t slot)
    {
        return gameplay::RuleId{"active-rule." + std::to_string(slot + 1)};
    }

    [[nodiscard]] inline bool isCompleteTriggeredRule(const gameplay::ComposedRule& rule)
    {
        return rule.trigger.has_value() && rule.subject.has_value() && rule.effect.has_value() && !rule.modifier.has_value();
    }

    [[nodiscard]] inline bool isCompleteStandingRule(const gameplay::ComposedRule& rule)
    {
        return !rule.trigger.has_value() && rule.subject.has_value() && !rule.effect.has_value() && rule.modifier.has_value();
    }
} // namespace soma::simulation::detail
