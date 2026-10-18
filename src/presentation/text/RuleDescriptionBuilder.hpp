#pragma once

#include "content/RuleContent.hpp"
#include "game/RuleState.hpp"

#include <optional>
#include <string>
#include <vector>

namespace soma::presentation
{
    [[nodiscard]] std::string buildGermanRuleDescription(const game::ComposedRule& rule,
                                                         std::optional<content::RuleMode> mode,
                                                         const content::RuleContent& content);

    [[nodiscard]] std::optional<content::RuleMode> completedRuleMode(const game::ComposedRule& rule);
    [[nodiscard]] std::vector<std::string> buildGermanActiveRuleDescriptions(const game::ComposedRules& rules,
                                                                              const content::RuleContent& content);
}
