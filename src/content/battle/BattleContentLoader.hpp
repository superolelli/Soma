#pragma once

#include "content/battle/BattleContent.hpp"
#include "gameplay/common/Diagnostic.hpp"

#include <filesystem>
#include <vector>

namespace soma::content
{
    struct BattleContentLoadResult
    {
        BattleContent content;
        std::vector<gameplay::Diagnostic> diagnostics;

        [[nodiscard]] bool valid() const;
    };

    /// Builds the complete battle simulation and presentation catalog from JSON.
    class BattleContentLoader
    {
      public:
        [[nodiscard]] BattleContentLoadResult load(
            const std::filesystem::path& contentRoot) const;
    };
} // namespace soma::content

