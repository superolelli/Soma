#pragma once

#include "content/battle/BattleContent.hpp"

#include <json.hpp>

namespace soma::content::detail
{
    void loadBattleDefinitions(const nlohmann::json& document, BattleContent& content);
    void loadBattlePresentation(const nlohmann::json& document, BattleContent& content);
} // namespace soma::content::detail
