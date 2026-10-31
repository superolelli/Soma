#pragma once

#include "content/rules/RuleContent.hpp"
#include "gameplay/rules/ComposedRule.hpp"

#include <optional>
#include <string>
#include <vector>

namespace soma::presentation
{
    [[nodiscard]] std::string buildGermanRuleDescription(const gameplay::ComposedRule& rule,
                                                         std::optional<content::RuleMode> mode,
                                                         const content::RuleContent& content);

    [[nodiscard]] std::optional<content::RuleMode> completedRuleMode(const gameplay::ComposedRule& rule);
    [[nodiscard]] std::vector<std::string> buildGermanActiveRuleDescriptions(const gameplay::ComposedRules& rules,
                                                                              const content::RuleContent& content);
}
