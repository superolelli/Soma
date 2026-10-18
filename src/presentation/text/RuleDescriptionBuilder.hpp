#pragma once

#include "content/RuleContent.hpp"
#include "game/RuleState.hpp"

#include <optional>
#include <string>

namespace soma::presentation
{
    [[nodiscard]] std::string buildGermanRuleDescription(const game::ComposedRule& rule,
                                                         std::optional<content::RuleMode> mode,
                                                         const content::RuleContent& content);
}
