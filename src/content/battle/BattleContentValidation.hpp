#pragma once

#include "content/battle/BattleContent.hpp"

#include <filesystem>

namespace soma::content::detail
{
    void validateBattleContent(const BattleContent& content, const std::filesystem::path& contentRoot);
} // namespace soma::content::detail
