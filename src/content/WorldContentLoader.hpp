#pragma once

#include "content/BattleContent.hpp"
#include "content/WorldContent.hpp"
#include "gameplay/common/Diagnostic.hpp"

namespace soma::content
{
    struct WorldContentLoadResult
    {
        WorldContent content;
        std::vector<gameplay::Diagnostic> diagnostics;

        [[nodiscard]] bool valid() const noexcept;
    };

    /// Loads campaigns, levels, encounters, lootables, rewards, and exploration presentation data.
    class WorldContentLoader
    {
      public:
        [[nodiscard]] WorldContentLoadResult load(const std::filesystem::path& contentRoot,
                                                  const BattleContent& battle,
                                                  const EconomyContent& economy) const;
    };
} // namespace soma::content
