#pragma once

#include "content/GameFlowContent.hpp"
#include "gameplay/common/Diagnostic.hpp"

#include <vector>

namespace soma::content
{
    struct GameFlowContentLoadResult
    {
        GameFlowContent content;
        std::vector<gameplay::Diagnostic> diagnostics;

        [[nodiscard]] bool valid() const noexcept;
    };

    /// Loads startup, menu, main-room, pause-menu, and application-flow presentation data.
    class GameFlowContentLoader
    {
      public:
        [[nodiscard]] GameFlowContentLoadResult load(const std::filesystem::path& contentRoot) const;
    };
} // namespace soma::content
