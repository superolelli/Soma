#pragma once

#include "content/economy/EconomyContent.hpp"
#include "gameplay/common/Diagnostic.hpp"

#include <filesystem>
#include <vector>

namespace soma::content
{
    struct EconomyContentLoadResult
    {
        EconomyContent content;
        std::vector<gameplay::Diagnostic> diagnostics;

        [[nodiscard]] bool valid() const;
    };

    /// Loads items, skills, ability sets, shop data, and economy presentation settings.
    class EconomyContentLoader
    {
      public:
        [[nodiscard]] EconomyContentLoadResult load(const std::filesystem::path& contentRoot) const;
    };
} // namespace soma::content
